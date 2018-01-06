#pragma once

#include "DX12Prerequisites.h"
#include "GpuResource.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RenderOutputDX12;
  class RenderContextDX12;
  class DescriptorDX12;
//---------------------------------------------------------------------------//
  class GpuResourceDX12
  {
    friend class RenderOutputDX12;
    friend class RenderContextDX12;
    friend class CommandContextDX12;

  public:
    GpuResourceDX12();
    virtual ~GpuResourceDX12();

    ID3D12Resource* GetResource() const  { return myResource.Get(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return myResource->GetGPUVirtualAddress(); }

    void Reset() { myResource.Reset(); }

  protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
  };
//---------------------------------------------------------------------------//
} } }
