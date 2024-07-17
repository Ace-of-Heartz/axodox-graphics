#pragma once
#include "VertexDefinitions.h"
#include "MeshDescriptions.h"

namespace Axodox::Graphics::D3D12
{
  AXODOX_GRAPHICS_API MeshDescription CreateQuad(float size = 1.f);
  AXODOX_GRAPHICS_API MeshDescription CreateCube(float size = 1.f, bool useSameTextureOnAllFaces = true);
  /// <summary>
  /// Creates a plane mesh with the specified size and subdivision level
  /// <param name="size"></param>
  /// <param name="subdivisions">.x and .y must be larger or equal to 2. Their product can't be larger than the numeric limit of uint32_t</param>
  /// <returns></returns>
  /// </summary>
  AXODOX_GRAPHICS_API MeshDescription CreatePlane(float size, DirectX::XMUINT2 subdivisions);
}