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
		GraphicsDevice* device;
		CommandList* commandList;
		CommandQueue* commandQueue;
		CommandFence* fence;
		PipelineState* pipelineState;
		DescriptorHeap* descriptorHeap;
		ResourceRef* vertexBuffer;
		CommandAllocator* commandAllocator;
		ResourceAllocator* resourceAllocator;

		ResourcesDXR() = default;

		ResourcesDXR(
			GraphicsDevice* device,
			CommandList* commandList,
			CommandQueue* commandQueue,
			CommandFence* fence,
			PipelineState* pipelineState,
			DescriptorHeap* descriptorHeap,
			ResourceRef* vertexBuffer,
			CommandAllocator* commandAllocator,
			ResourceAllocator* resourceAllocator
		) :
		device(device),
		commandList(commandList),
		commandQueue(commandQueue),
		fence(fence),
		pipelineState(pipelineState),
		descriptorHeap(descriptorHeap),
		commandAllocator(commandAllocator),
		resourceAllocator(resourceAllocator)
		{
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
