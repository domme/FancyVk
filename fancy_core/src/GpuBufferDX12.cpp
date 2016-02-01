#include "GpuBufferDX12.h"
#include "Renderer.h"

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  GpuBufferDX12::GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  bool GpuBufferDX12::operator==(const GpuBufferDesc& aDesc) const
  {
    return GetDescription() == aDesc;
  }
//---------------------------------------------------------------------------//
  GpuBufferDesc GpuBufferDX12::GetDescription() const
  {
    GpuBufferDesc desc;
    desc.myInternalRefIndex = myParameters.myInternalRefIndex;
    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::setBufferData(void* pData, uint uOffsetElements, uint uNumElements)
  {
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::create(const GpuBufferCreationParams& someParameters, void* pInitialData)
  {
    RendererDX12& renderer = Renderer::getInstance();

    ASSERT_M(someParameters.uElementSizeBytes > 0 && someParameters.uNumElements > 0,
      "Invalid buffer size specified");

    GpuBufferCreationParams* pBaseParams = &myParameters;
    *pBaseParams = someParameters;
    
    const bool wantsCpuWrite = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE) > 0u;
    const bool wantsCpuRead = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::READ) > 0u;

    // Can we do something with these flags on DX12?
    //const bool wantsDynamic = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::DYNAMIC) > 0u;
    //const bool wantsCoherent = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::COHERENT) > 0u;
    //const bool wantsCpuStorage = (someParameters.uAccessFlags & (uint)GpuResourceAccessFlags::PREFER_CPU_STORAGE) > 0u;
    
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_CPU_PAGE_PROPERTY cpuPageProp = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATE_COMMON;

    if (wantsCpuWrite && wantsCpuRead)
    {
      heapType = D3D12_HEAP_TYPE_UPLOAD;  // could also select _READBACK here...
      cpuPageProp = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;  // For CPU-reads, we need the data as soon as possible
      
    }
    else if(wantsCpuWrite && !wantsCpuRead)
    {
      heapType = D3D12_HEAP_TYPE_UPLOAD;
      cpuPageProp = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
    }
    else if(!wantsCpuWrite && wantsCpuRead)
    {
      heapType = D3D12_HEAP_TYPE_READBACK;
      cpuPageProp = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    }



    
    D3D12_HEAP_PROPERTIES heapProps;
    // heapProps.



    //renderer.myDevice->CreateCommittedResource()

  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::destroy()
  {
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::lock(GpuResoruceLockOption eLockOption, uint uOffsetElements, uint uNumElements)
  {
    return nullptr;
  }
//---------------------------------------------------------------------------//
} } }

#endif
