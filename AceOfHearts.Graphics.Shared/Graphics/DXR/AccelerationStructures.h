#pragma once
#include <wrl/client.h>

#include "Graphics/DXR/TopLevelASGenerator.h"

using namespace Microsoft::WRL;
using namespace Axodox::Graphics::D3D12;

namespace AceOfHearts::Graphics::DXR {
	struct AccelerationStructureBuffers
	{
		ComPtr<ID3D12Resource> scratch;
		ComPtr<ID3D12Resource> result;
		ComPtr<ID3D12Resource> instanceDesc;
	};
	struct FrameResourcesDXR
	{
		CommandAllocator Allocator;
		CommandFence Fence;
		CommandFenceMarker Marker;
		DynamicBufferManager DynamicBuffer;
	};

	struct 

	class AccelerationStructBuilder{
		typedef std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> InstanceVec;
		typedef std::vector<std::pair<ComPtr<ID3D12Resource>, uint321_t >> VertexBufferVec;
		
		public:
			void CreateAccelerationStructures();

		private:
		ComPtr<ID3D12Resource> m_bottomLevelAS;
		nv_helpers_dx12::TopLevelASGenerator m_topLevelASGenerator;
		AccelerationStructureBuffers m_topLevelASBuffers;
		InstanceVec m_instances;

		GraphicsDevice* m_device;
		CommandList* m_commandList;
		CommandQueue* m_commandQueue;


		AccelerationStructBuilder(GraphicsDevice* device, CommandList*)
			: m_device(device), m_commandList(commandList)
		{}

		AccelerationStructureBuffers CreateBottomLevelAS(VertexBufferVec vVertexBuffers);
		void CreateTopLevelAS(const InstanceVec& instances);

	};

	
}
