#include "pch.h"
#include "ace_pch.h"
#include <d3d12.idl>


using namespace Axodox::Graphics::D3D12;

#include "DXRUtility.h"

namespace AceOfHearts::Graphics::DXR {

	struct DXRWrapper {
		GraphicsDevice* _device;
		FrameResources* _frameResources;
	};

	struct DXRAccelerationStructureWrapper {
		AccelerationStructureBuffers* _topLevelAB;
		AccelerationStructureBuffers* _bottomLevelAB;
	};

	struct FrameResources
	{
		CommandAllocator Allocator;
		CommandFence Fence;
		CommandFenceMarker Marker;
		DynamicBufferManager DynamicBuffer;

		MutableTexture DepthBuffer;
		MutableTexture PostProcessingBuffer;
		descriptor_ptr<ShaderResourceView> ScreenResourceView;

		FrameResources(const ResourceAllocationContext& context) :
			Allocator(*context.Device),
			Fence(*context.Device),
			Marker(),
			DynamicBuffer(*context.Device),
			DepthBuffer(context),
			PostProcessingBuffer(context)
		{ }
	};

	struct SimpleRootDescription : public RootSignatureMask
	{
		RootDescriptor<RootDescriptorType::ConstantBuffer> ConstantBuffer;
		RootDescriptorTable<1> Texture;
		StaticSampler Sampler;

		SimpleRootDescription(const RootSignatureContext& context) :
			RootSignatureMask(context),
			ConstantBuffer(this, { 0 }, ShaderVisibility::Vertex),
			Texture(this, { DescriptorRangeType::ShaderResource }, ShaderVisibility::Pixel),
			Sampler(this, { 0 }, Filter::Linear, TextureAddressMode::Clamp, ShaderVisibility::Pixel)
		{
			Flags = RootSignatureFlags::AllowInputAssemblerInputLayout;
		}
	};

	struct PostProcessingRootDescription : public RootSignatureMask
	{
		RootDescriptor<RootDescriptorType::ConstantBuffer> ConstantBuffer;
		RootDescriptorTable<1> InputTexture;
		RootDescriptorTable<1> OutputTexture;
		StaticSampler Sampler;

		PostProcessingRootDescription(const RootSignatureContext& context) :
			RootSignatureMask(context),
			ConstantBuffer(this, { 0 }),
			InputTexture(this, { DescriptorRangeType::ShaderResource }),
			OutputTexture(this, { DescriptorRangeType::UnorderedAccess }),
			Sampler(this, { 0 }, Filter::Linear, TextureAddressMode::Clamp)
		{
			Flags = RootSignatureFlags::AllowInputAssemblerInputLayout;
		}
	};
}          