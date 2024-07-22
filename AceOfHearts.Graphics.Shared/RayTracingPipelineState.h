#pragma once

using namespace Axodox::Graphics::D3D12;

namespace AceOfHearts::Graphics::DXR{
	class RayTracingPipelineProvider
	{

	};

	struct RayTracingPipelineStateDefinition
	{
		RootSignatureBase* RootSignature = nullptr;
		ComputeShader* ComputeShader = nullptr;

		BlendState BlendState = {}

		void AddCachedPso(std::span<const uint8_t> buffer);
		explicit operator D3D12_COMPUTE_PIPELINE_STATE_DESC() const;
	};

}