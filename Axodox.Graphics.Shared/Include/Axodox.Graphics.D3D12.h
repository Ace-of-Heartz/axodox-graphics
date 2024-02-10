#pragma once
#include "../includes.h"

#include "Graphics/D3D12/Enumerations.h"
#include "Graphics/D3D12/GraphicsTypes.h"
#include "Graphics/D3D12/Devices/GraphicsDevice.h"
#include "Graphics/D3D12/Commands/CommandQueue.h"
#include "Graphics/D3D12/Commands/CommandFence.h"
#include "Graphics/D3D12/Commands/CommandList.h"
#include "Graphics/D3D12/Commands/CommandAllocator.h"
#include "Graphics/D3D12/Swap Chains/SwapChain.h"
#include "Graphics/D3D12/Swap Chains/CoreSwapChain.h"
#include "Graphics/D3D12/States/RootParameters.h"
#include "Graphics/D3D12/States/StaticSampler.h"
#include "Graphics/D3D12/States/RootSignature.h"
#include "Graphics/D3D12/States/PipelineState.h"
#include "Graphics/D3D12/Resources/DynamicBufferManager.h"
#include "Graphics/D3D12/Resources/CommittedResourceAllocator.h"
#include "Graphics/D3D12/Resources/GroupedResourceAllocator.h"
#include "Graphics/D3D12/Resources/ResourceUploader.h"
#include "Graphics/D3D12/Resources/ImmutableTexture.h"
#include "Graphics/D3D12/Resources/MutableTexture.h"
#include "Graphics/D3D12/Meshes/VertexDefinitions.h"
#include "Graphics/D3D12/Meshes/MeshDescriptions.h"
#include "Graphics/D3D12/Meshes/Primitives.h"
#include "Graphics/D3D12/Meshes/ImmutableMesh.h"
#include "Graphics/D3D12/Descriptors/Descriptor.h"
#include "Graphics/D3D12/Descriptors/DescriptorHeap.h"
#include "Graphics/D3D12/Descriptors/RenderTargetView.h"
#include "Graphics/D3D12/Descriptors/DepthStencilView.h"
#include "Graphics/D3D12/Descriptors/ConstantBufferView.h"
#include "Graphics/D3D12/Descriptors/ShaderResourceView.h"
#include "Graphics/D3D12/Descriptors/CommonDescriptorHeap.h"
#include "Graphics/D3D12/Descriptors/UnorderedAccessView.h"