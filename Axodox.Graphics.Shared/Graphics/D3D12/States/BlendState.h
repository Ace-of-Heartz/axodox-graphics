#pragma once
#include "pch.h"

namespace Axodox::Graphics::D3D12
{

	/// <summary>
	///	Blend types for blending operations.
	/// </summary>
	enum BlendType : uint8_t
	{
		Opaque,
		Additive,
		Subtractive,
		AlphaBlend,
		Minimum,
		Maximum
	};

	/// <summary>
	/// Stores up to 8 blend types for the render target.
	/// </summary>
	struct AXODOX_GRAPHICS_API BlendState
	{
		std::array<BlendType, 8> BlendTypes;

		BlendState(BlendType blendType = BlendType::Opaque);
		BlendState(std::initializer_list<BlendType> blendTypes);

		/// <summary>
		/// 
		/// </summary>
		explicit operator D3D12_BLEND_DESC() const;
	};
}