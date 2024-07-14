#include "pch.h"
#include "MutableTexture.h"
#include "Infrastructure/BitwiseOperations.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Axodox::Graphics::D3D12
{

  /// <summary>
  /// Initializes a new instance with the specified context.
  /// </summary>
  /// <param name="context"></param>
  MutableTexture::MutableTexture(const ResourceAllocationContext& context) :
    _context(context)
  { }

  /// <summary>
  /// Initializes a new instance with the specified context and allocates a new texture with the specified definition.
  /// </summary>
  /// <param name="context"></param>
  /// <param name="definition"></param>
  MutableTexture::MutableTexture(const ResourceAllocationContext & context, const TextureDefinition & definition) :
    MutableTexture(context)
  {
    Allocate(definition);
  }


  /// <summary>
  /// Gets the definition of the texture.
  /// </summary>
  /// <returns></returns>
  const TextureDefinition* MutableTexture::Definition() const
  {
    return _definition.get();
  }

  /// <summary>
  /// Gets the shader resource view for the texture.
  /// </summary>
  /// <returns></returns>
  ShaderResourceView* MutableTexture::ShaderResource() const
  {
    return _shaderResourceView.get();
  }

  /// <summary>
  /// Gets the render target view for the texture.
  /// </summary>
  /// <returns></returns>
  RenderTargetView* MutableTexture::RenderTarget() const
  {
    return _renderTargetView.get();
  }

  /// <summary>
  /// Gets the depth stencil view for the texture.
  /// </summary>
  /// <returns></returns>
  DepthStencilView* MutableTexture::DepthStencil() const
  {
    return _depthStencilView.get();
  }

  /// <summary>
  /// Gets the unordered access view for the texture.
  /// </summary>
  /// <returns></returns>
  UnorderedAccessView* MutableTexture::UnorderedAccess() const
  {
    return _unorderedAccessView.get();
  }

  /// <summary>
  /// Allocates a new texture with the specified definition.
  /// Resets the previous texture and all associated views.
  /// Sends out an event when the texture is allocated.
  /// </summary>
  /// <param name="definition"></param>
  void MutableTexture::Allocate(const TextureDefinition& definition)
  {
    Reset();

    _texture = _context.ResourceAllocator->CreateTexture(definition);
    _allocatedSubscription = _texture->Allocated(event_handler{ this, &MutableTexture::OnAllocated });
  }

  /// <summary>
  /// Resizes the texture to the specified dimensions if it is different from the current size.
  /// </summary>
  /// <param name="width"></param>
  /// <param name="height"></param>
  void MutableTexture::Resize(uint32_t width, uint32_t height)
  {
    if (_definition && _definition->Width == width && _definition->Height == height) return;

    auto definition = *_definition;
    definition.Width = width;
    definition.Height = height;
    Allocate(definition);
  }

  /// <summary>
  /// Resets the texture and all associated views.
  /// </summary>
  void MutableTexture::Reset()
  {
    _texture.reset();
    _shaderResourceView.reset();
    _renderTargetView.reset();
    _depthStencilView.reset();
    _unorderedAccessView.reset();
    _definition.reset();
  }

  /// <summary>
  /// Returns the texture as a resource argument.
  /// </summary>
  MutableTexture::operator ResourceArgument() const
  {
    return _texture->get();
  }

  /// <summary>
  /// Returns true if the texture is allocated.
  /// </summary>
  MutableTexture::operator bool() const
  {
    return _texture != nullptr;
  }

  void MutableTexture::OnAllocated(Resource* resource)
  {
    _definition = make_unique<TextureDefinition>(resource->Description());
    auto flags = D3D12_RESOURCE_FLAGS(_definition->Flags);
    auto texture = static_cast<Texture*>(resource);

    if (!has_flag(flags, D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
    {
      _shaderResourceView = _context.CommonDescriptorHeap->CreateShaderResourceView(resource);
    }

    if (has_flag(flags, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET))
    {
      _renderTargetView = _context.RenderTargetDescriptorHeap->CreateRenderTargetView(texture);
    }

    if (has_flag(flags, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
    {
      _depthStencilView = _context.DepthStencilDescriptorHeap->CreateDepthStencilView(texture);
    }

    if (has_flag(flags, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
    {
      _unorderedAccessView = _context.CommonDescriptorHeap->CreateUnorderedAccessView(texture);
    }
  }
}