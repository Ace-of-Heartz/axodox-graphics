#include "pch.h"
#include "CommandAllocator.h"
#include "../States/PipelineState.h"

using namespace std;
using namespace winrt;

namespace Axodox::Graphics::D3D12
{
  CommandAllocator::CommandAllocator(const GraphicsDevice& device, CommandKind type) :
    _device(device),
    _type(type)
  { 
    check_hresult(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE(type), guid_of<ID3D12CommandAllocator>(), _allocator.put_void()));
  }

  ID3D12GraphicsCommandListT* CommandAllocator::operator->()
  {
    if (!_recorder) BeginList();
    return _recorder->_list.get();
  }

  void CommandAllocator::BeginList(PipelineState* pipelineState)
  {
    if (_recorder) throw logic_error("Cannot start a command list before finishing the last one!");

    _recorder = _lists.Borrow();
    if (!_recorder->_list)
    {
      check_hresult(_device->CreateCommandList(
        0u,
        D3D12_COMMAND_LIST_TYPE(_type),
        _allocator.get(),
        pipelineState ? pipelineState->get() : nullptr,
        IID_PPV_ARGS(_recorder->_list.put())
      ));
    }
    else
    {
      _recorder->_list->Reset(_allocator.get(), nullptr);
    }
  }

  Collections::ObjectPoolHandle<CommandList> CommandAllocator::EndList()
  {
    if (!_recorder) throw logic_error("Cannot end a command list before starting one!");

    auto result{ move(_recorder) };
    result->_list->Close();
    return result;
  }

  CommandKind CommandAllocator::Type() const
  {
    return _type;
  }

  void CommandAllocator::AddAwaiter(CommandFenceMarker marker)
  {
    if (!_recorder) throw logic_error("There is no active command list!");
    _recorder->_awaiters.push_back(marker);
  }

  void CommandAllocator::AddSignaler(CommandFenceMarker marker)
  {
    if (!_recorder) throw logic_error("There is no active command list!");
    _recorder->_signalers.push_back(marker);
  }

  void CommandAllocator::ResourceTransition(ResourceReference resource, ResourceStates from, ResourceStates to)
  {
    D3D12_RESOURCE_BARRIER barrier{
      .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
      .Transition = { 
        .pResource = resource.Pointer,
        .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        .StateBefore = D3D12_RESOURCE_STATES(from),
        .StateAfter = D3D12_RESOURCE_STATES(to)
      }
    };

    (*this)->ResourceBarrier(1, &barrier);
  }

  void CommandAllocator::Reset()
  {
    _allocator->Reset();
  }
}