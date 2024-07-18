#pragma once
#include <wrl/client.h>

#include "Graphics/DXR/TopLevelASGenerator.h"

using namespace Microsoft::WRL;

namespace AceOfHearts::Graphics::DXR {
	typedef ComPtr<ID3D12Resource> ResourcePtr;

	struct AccelerationStructureBuffers
	{
		ResourcePtr pScratch;
		ResourcePtr pResult;
		ResourcePtr pInstanceDesc;
	};

	struct ResourcesDXR
	{
		ComPtr<ID3D12Device5> device;
		ComPtr<ID3D12GraphicsCommandList4> commandList;
		ComPtr<ID3D12CommandQueue> commandQueue;
		ComPtr<ID3D12Fence> fence;
		HANDLE fenceEvent;
		UINT64 fenceValue;
		ComPtr<ID3D12PipelineState> pipelineState;
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		ComPtr<Axodox::Graphics::D3D12::ResourceAllocator> resourceAllocator;
	};

	class AccelerationStructBuilder{
		typedef std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> VertexBufferVec;
		typedef std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> InstanceVec;
		

		public:
			void CreateAccelerationStructures();

		private:

			ResourcesDXR m_resources;
			InstanceVec m_instances;
			ResourcePtr m_bottomLevelAS;
			AccelerationStructureBuffers m_topLevelASBuffers;
			nv_helpers_dx12::TopLevelASGenerator m_topLevelASGenerator;

			AccelerationStructureBuffers CreateBottomLevelAS(VertexBufferVec vVertexBuffers);
			void CreateTopLevelAS(const InstanceVec& instances);
	};

	
}
