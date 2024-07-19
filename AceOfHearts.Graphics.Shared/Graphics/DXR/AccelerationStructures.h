#pragma once
#include <wrl/client.h>

#include "Graphics/DXR/TopLevelASGenerator.h"

using namespace Microsoft::WRL;
using namespace Axodox::Graphics::D3D12;


namespace AceOfHearts::Graphics::DXR {
	

	struct AccelerationStructureBuffers
	{
		ResourceRef pScratch;
		ResourceRef pResult;
		ResourceRef pInstanceDesc;

		AccelerationStructureBuffers() : pScratch(nullptr), pResult(nullptr), pInstanceDesc(nullptr) {}
		AccelerationStructureBuffers(ResourceRef scratch, ResourceRef result, ResourceRef instanceDesc)
		{
			pScratch.swap(scratch);
			pResult.swap(result);
			pInstanceDesc.swap(instanceDesc);
		}
	};

	struct ResourcesDXR
	{
		winrt::com_ptr<GraphicsDevice> device;
		winrt::com_ptr<CommandList> commandList;
		winrt::com_ptr<CommandQueue> commandQueue;
		winrt::com_ptr<CommandFence> fence;
		winrt::com_ptr<PipelineState> pipelineState;
		winrt::com_ptr<DescriptorHeap> descriptorHeap;
		ResourceRef vertexBuffer;
		winrt::com_ptr<CommandAllocator> commandAllocator;
		winrt::com_ptr<ResourceAllocator> resourceAllocator;

		ResourcesDXR() = default;

		ResourcesDXR(
			winrt::com_ptr<GraphicsDevice> device,
			winrt::com_ptr<CommandList> commandList,
			winrt::com_ptr<CommandQueue> commandQueue,
			winrt::com_ptr<CommandFence> fence,
			winrt::com_ptr<PipelineState> pipelineState,
			winrt::com_ptr<DescriptorHeap> descriptorHeap,
			ResourceRef vertexBuffer,
			winrt::com_ptr<CommandAllocator> commandAllocator,
			winrt::com_ptr<ResourceAllocator> resourceAllocator
		) :
		device(device),
		commandList(commandList),
		commandQueue(commandQueue), fence(fence),
		pipelineState(pipelineState),
		descriptorHeap(descriptorHeap),
		commandAllocator(commandAllocator),
		resourceAllocator(resourceAllocator)
		{
			vertexBuffer.swap(vertexBuffer);
		}
	};

	class AccelerationStructBuilder{
		typedef std::vector<std::pair<ID3D12Resource*, uint32_t>> VertexBufferVec;
		typedef std::vector<std::pair<ID3D12Resource*, DirectX::XMMATRIX>> InstanceVec;
		

		public:
			void CreateAccelerationStructures();
			AccelerationStructBuilder() {}
			AccelerationStructBuilder(ResourcesDXR* resources) {
				m_resources = resources;
			}
		private:

			ResourcesDXR* m_resources;
			InstanceVec m_instances;
			ResourceRef m_bottomLevelAS;
			AccelerationStructureBuffers m_topLevelASBuffers;
			nv_helpers_dx12::TopLevelASGenerator m_topLevelASGenerator;

			AccelerationStructureBuffers CreateBottomLevelAS(VertexBufferVec vVertexBuffers);
			void CreateTopLevelAS(const InstanceVec& instances);
	};

	
}
