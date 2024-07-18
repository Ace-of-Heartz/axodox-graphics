#include "pch.h"

#include "DXRUtility.h"

using namespace Axodox::Graphics::D3D12;

void AceOfHearts::Graphics::DXR::CheckRayTracingSupport(GraphicsDevice* device)
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
    device->get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
        &options5, sizeof(options5));
    if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
        throw std::runtime_error("Raytracing not supported on device");
}