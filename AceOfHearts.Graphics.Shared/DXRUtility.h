#pragma once

#include "ace_pch.h"
#include "../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/winrt/wrl/client.h"


namespace AceOfHearts::Graphics::DXR {
	using Microsoft::WRL::ComPtr;



	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}
	inline void CheckRayTracingSupport(ID3D12Device* device)
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
		if (options5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		{
			throw std::exception("Raytracing not supported on device");
		}
		
	}
}