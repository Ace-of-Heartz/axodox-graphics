#include "pch.h"
#include "AccelerationStructures.h"

#include "Graphics/DXR/BottomLevelASGenerator.h"

using namespace Axodox::Graphics::D3D12;
using namespace AceOfHearts::Graphics::DXR;

void AccelerationStructBuilder::CreateAccelerationStructures()
{

    AccelerationStructureBuffers bottomLevelBuffers =
        CreateBottomLevelAS({{m_resources->vertexBuffer->operator->(),3}});

    m_instances = { {bottomLevelBuffers.pResult->getResource(), DirectX::XMMatrixIdentity()} };
    CreateTopLevelAS(m_instances);

    auto rayTracingCommandList = m_resources->commandAllocator->EndList();
    m_resources->commandQueue->Execute(rayTracingCommandList);
    auto marker = m_resources->fence->EnqueueSignal(*m_resources->commandQueue.get());
    m_resources->fence->Await(marker);

    m_resources->commandAllocator->Reset();

    m_bottomLevelAS.swap(bottomLevelBuffers.pResult);
}


AceOfHearts::Graphics::DXR::AccelerationStructureBuffers
AceOfHearts::Graphics::DXR::AccelerationStructBuilder::CreateBottomLevelAS(VertexBufferVec vVertexBuffers)
{
    nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS;

    for (const auto& buffer : vVertexBuffers) {
        bottomLevelAS.AddVertexBuffer(buffer.first,
            0,
            buffer.second,
            sizeof(VertexPositionColor),
            0,
            0);
    }

    UINT64 scratchSizeInBytes = 0;

    UINT64 resultSizeInBytes = 0;

    bottomLevelAS.ComputeASBufferSizes(m_resources->device->get(), false, &scratchSizeInBytes,
        &resultSizeInBytes);

    AccelerationStructureBuffers buffers;
    buffers.pScratch = m_resources->resourceAllocator->CreateBuffer(
		BufferDefinition{scratchSizeInBytes,BufferFlags::UnorderedAccess}
    );

    buffers.pResult = m_resources->resourceAllocator->CreateBuffer(
        BufferDefinition{resultSizeInBytes, BufferFlags::UnorderedAccess});
    m_resources->commandAllocator->TransitionResource(buffers.pResult->get(),
		ResourceStates::Common, ResourceStates::RaytracingAccelerationStructure);

    bottomLevelAS.Generate(m_resources->commandList->get(), buffers.pScratch->getResource(),
        buffers.pResult->getResource(), false, nullptr);

    return buffers;
}

void AceOfHearts::Graphics::DXR::AccelerationStructBuilder::CreateTopLevelAS(const InstanceVec& instances)
{
    for (size_t i = 0; i < instances.size(); i++) {
        m_topLevelASGenerator.AddInstance(instances[i].first,
            instances[i].second, static_cast<uint32_t>(i),
            static_cast<uint32_t>(0));
    }

    UINT64 scratchSize, resultSize, instanceDescsSize;

    m_topLevelASGenerator.ComputeASBufferSizes(m_resources->device->get(), true, &scratchSize,
        &resultSize, &instanceDescsSize);

    m_topLevelASBuffers.pScratch = m_resources->resourceAllocator->CreateBuffer(
        BufferDefinition{scratchSize, BufferFlags::UnorderedAccess});
    m_resources->commandAllocator->TransitionResource(m_topLevelASBuffers.pScratch->get(),
		ResourceStates::Common, ResourceStates::UnorderedAccess);


    m_topLevelASBuffers.pResult = m_resources->resourceAllocator->CreateBuffer(
        BufferDefinition{ resultSize, BufferFlags::UnorderedAccess });
    m_resources->commandAllocator->TransitionResource(m_topLevelASBuffers.pResult->get(),
        ResourceStates::Common, ResourceStates::RaytracingAccelerationStructure);


    m_topLevelASBuffers.pInstanceDesc = m_resources->resourceAllocator->CreateBuffer(
        BufferDefinition{ instanceDescsSize, BufferFlags::None});
    m_resources->commandAllocator->TransitionResource(m_topLevelASBuffers.pInstanceDesc->get(),
		ResourceStates::Common, ResourceStates::GenericRead);


    m_topLevelASGenerator.Generate(m_resources->commandList->get(),
        m_topLevelASBuffers.pScratch->getResource(),
        m_topLevelASBuffers.pResult->getResource(),
        m_topLevelASBuffers.pInstanceDesc->getResource());
}
