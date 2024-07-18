#include "pch.h"
#include "AccelerationStructures.h"

#include "Graphics/DXR/BottomLevelASGenerator.h"

using namespace Axodox::Graphics::D3D12;
using namespace AceOfHearts::Graphics::DXR;

void AccelerationStructBuilder::CreateAccelerationStructures()
{

    AccelerationStructureBuffers bottomLevelBuffers =
        CreateBottomLevelAS({ {m_resources.vertexBuffer.Get(), 3} });

    m_instances = { {bottomLevelBuffers.pResult, DirectX::XMMatrixIdentity()} };
    CreateTopLevelAS(m_instances);

    m_resources.commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_resources.commandList.Get()};
    m_resources.commandQueue->ExecuteCommandLists(1, ppCommandLists);
    m_resources.fenceValue++;
    m_resources.commandQueue->Signal(m_resources.fence.Get(), m_resources.fenceValue);

    m_resources.fence->SetEventOnCompletion(m_resources.fenceValue, m_resources.fenceEvent);
    WaitForSingleObject(m_resources.fenceEvent, INFINITE);


    
	m_resources.commandList->Reset(m_resources.commandAllocator.Get(), m_resources.pipelineState.Get());

    m_bottomLevelAS = bottomLevelBuffers.pResult;
}


AceOfHearts::Graphics::DXR::AccelerationStructureBuffers
AceOfHearts::Graphics::DXR::AccelerationStructBuilder::CreateBottomLevelAS(VertexBufferVec vVertexBuffers)
{
    nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS;

    for (const auto& buffer : vVertexBuffers) {
        bottomLevelAS.AddVertexBuffer(buffer.first.Get(), 0, buffer.second,
            sizeof(VertexPositionColor), 0, 0);
    }

    UINT64 scratchSizeInBytes = 0;

    UINT64 resultSizeInBytes = 0;

    bottomLevelAS.ComputeASBufferSizes(m_resources.device.Get(), false, &scratchSizeInBytes,
        &resultSizeInBytes);

    AccelerationStructureBuffers buffers;
    buffers.pScratch = m_resources.resourceAllocator->CreateBuffer(
    BufferDefinition{scratchSizeInBytes,BufferFlags::UnorderedAccess}
    )->getResource();
	//Transition to State Common	

    buffers.pResult = m_resources.resourceAllocator.CreateBuffer(
        m_resources.device.Get(), resultSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nullptr).getResource();

    bottomLevelAS.Generate(m_resources.commandList.Get(), buffers.pScratch.Get(),
        buffers.pResult.Get(), false, nullptr);

    return buffers;
}

void AceOfHearts::Graphics::DXR::AccelerationStructBuilder::CreateTopLevelAS(const InstanceVec& instances)
{
    for (size_t i = 0; i < instances.size(); i++) {
        m_topLevelASGenerator.AddInstance(instances[i].first.Get(),
            instances[i].second, static_cast<uint32_t>(i),
            static_cast<uint32_t>(0));
    }

    UINT64 scratchSize, resultSize, instanceDescsSize;

    m_topLevelASGenerator.ComputeASBufferSizes(m_resources.device.Get(), true, &scratchSize,
        &resultSize, &instanceDescsSize);

    m_topLevelASBuffers.pScratch = m_resources.resourceAllocator.CreateBuffer(
        m_resources.device.Get(), scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr);
    m_topLevelASBuffers.pResult = m_resources.resourceAllocator.CreateBuffer(
        m_resources.device.Get(), resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        nullptr);

    m_topLevelASBuffers.pInstanceDesc = m_resources.resourceAllocator.CreateBuffer(
        m_resources.device.Get(), instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

    m_topLevelASGenerator.Generate(m_resources.commandList.Get(),
        m_topLevelASBuffers.pScratch.Get(),
        m_topLevelASBuffers.pResult.Get(),
        m_topLevelASBuffers.pInstanceDesc.Get());
}
