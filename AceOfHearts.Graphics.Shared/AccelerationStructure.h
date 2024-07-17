#pragma once

#include "ace_pch.h"
#include "pch.h"
#include <nv_helpers_dx12/BottomLevelASGenerator.h>
#include <DXRWrapper.h>
#include <nv_helpers_dx12/TopLevelASGenerator.h>

using namespace AceOfHearts::Graphics::DXR;


namespace AceOfHearts::Graphics::DXR{
	struct AccelerationStructureBuffers {
		winrt::com_ptr<Resource> pScratch;
		winrt::com_ptr<Resource> pResult;
		winrt::com_ptr<Resource> pInstanceDesc;
	};

	AccelerationStructureBuffers CreateBottomLevelAS(DXRWrapper context,
		std::vector<std::pair<winrt::com_ptr<Resource>, uint32_t >> vVertexBuffers) 
	{

		BottomLevelASGenerator bottomLevelAS;

		// Adding all vertex buffers and not transforming their position.
		for (const auto& buffer : vVertexBuffers) {
			bottomLevelAS.AddVertexBuffer(buffer.first.get()->getResource(), 0, buffer.second,
				sizeof(VertexPositionColor), 0, 0);
		}

		UINT64 scratchSizeInBytes = 0;

		UINT64 resultSizeInBytes = 0;

		bottomLevelAS.ComputeASBufferSizes(context._device->get(), false, &scratchSizeInBytes,
			&resultSizeInBytes);

		AccelerationStructureBuffers buffers;
		// TODO: Write this into the wrappers style
		buffers.pScratch = AceOfHearts::Graphics::DXR::CreateBuffer(
			context._device->get(), scratchSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
			nullptr);
		buffers.pResult = AceOfHearts::Graphics::DXR::CreateBuffer(
			context._device->get(), resultSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr);

		bottomLevelAS.Generate(context._frameResources->Allocator.getCommand(), buffers.pScratch.get()->getResource(),
			buffers.pResult.get()->getResource(), false, nullptr);

		return buffers;
	}

	void CreateTopLevelAS(DXRWrapper context, 
		TopLevelASGenerator* topLevelASGenerator, 
		DXRAccelerationStructureWrapper* acWrappers,
		const std::vector<std::pair<com_ptr<Resource>,DirectX::XMMATRIX>> instances) 
	{
		for (size_t i = 0; i < instances.size(); i++) {
			topLevelASGenerator -> AddInstance(instances[i].first.get()->getResource(),
				instances[i].second, static_cast<unsigned int>(i),
				static_cast<unsigned int>(0));
		}

		UINT64 scratchSize, resultSize, instanceDescsSize;

		topLevelASGenerator -> ComputeASBufferSizes(context._device->get(), true, &scratchSize,
			&resultSize, &instanceDescsSize);

		acWrappers->_topLevelAB -> pScratch = AceOfHearts::Graphics::DXR::CreateBuffer(
			context._device->get(), scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			AceOfHearts::Graphics::DXR::kDefaultHeapProps);
		acWrappers->_topLevelAB -> pResult = AceOfHearts::Graphics::DXR::CreateBuffer(
			context._device->get(), resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			AceOfHearts::Graphics::DXR::kDefaultHeapProps);

		acWrappers->_topLevelAB -> pInstanceDesc = AceOfHearts::Graphics::DXR::CreateBuffer(
			context._device->get(), instanceDescsSize, D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ, AceOfHearts::Graphics::DXR::kUploadHeapProps);

		topLevelASGenerator -> Generate(context._frameResources->Allocator.getCommand(),
			acWrappers->_topLevelAB -> pScratch.get(),
			acWrappers->_topLevelAB -> pResult.get(),
			acWrappers->_topLevelAB -> pInstanceDesc.get());
	}
	
	void CreateAccelerationStructures(DXRWrapper context,Resource* vertexBuffer) {
		// No idea
		AccelerationStructureBuffers bottomLevelBuffers = CreateBottomLevelAS({ {vertexBuffer->get(), 3} });

		auto instances = { {bottomLevelBuffers.pResult, XMMatrixIdentity()} };
		CreateTopLevelAS(instances);

		context._frameResources->Allocator.EndList();
		ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
		m_commandQueue->ExecuteCommandLists(1, ppCommandLists);
		context._frameResources->Fence.CreateMarker();//???
		m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);

		// Once the command list is finished executing, reset it to be reused for
		// rendering
		ThrowIfFailed(
			m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

		// Store the AS buffers. The rest of the buffers will be released once we exit
		// the function
		bottomLevelAS = bottomLevelBuffers.pResult;
	}

	
}
