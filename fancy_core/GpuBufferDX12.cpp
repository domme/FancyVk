#include "GpuBufferDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "GpuResourceStorageDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuBufferDX12::GpuBufferDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuBufferDX12::~GpuBufferDX12()
  {
    Destroy();
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Create(const GpuBufferProperties& someProperties, const void* pInitialData)
  {
    ASSERT(someProperties.myElementSizeBytes > 0 && someProperties.myNumElements > 0, "Invalid buffer size specified");

    Destroy();

    GpuResourceStorageDX12* storageDx12 = new GpuResourceStorageDX12();
    myStorage.reset(storageDx12);

    myProperties = someProperties;

    myAlignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    if (someProperties.myUsage == GpuBufferUsage::CONSTANT_BUFFER)
      myAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

    const uint64 pitch = MathUtil::Align(someProperties.myNumElements * someProperties.myElementSizeBytes, myAlignment);

    D3D12_RESOURCE_DESC resourceDesc;
    memset(&resourceDesc, 0, sizeof(resourceDesc));
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = pitch;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.Flags = someProperties.myIsShaderWritable ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES readState = D3D12_RESOURCE_STATE_GENERIC_READ;

    switch (someProperties.myUsage) 
    { 
      case GpuBufferUsage::STAGING_UPLOAD: 
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ; // Required for upload-heaps according to the D3D12-docs
      break;
      case GpuBufferUsage::STAGING_READBACK: 
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ; // Not sure about this one...
      break;
      case GpuBufferUsage::VERTEX_BUFFER:
      case GpuBufferUsage::CONSTANT_BUFFER: 
        initialState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        readState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
      break;
      case GpuBufferUsage::INDEX_BUFFER: 
        initialState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        readState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
      break;
      case GpuBufferUsage::SHADER_BUFFER:
        initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        readState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (someProperties.myIsShaderWritable)
          initialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
      break;
    default: ;
    }
    storageDx12->myState = initialState;
    storageDx12->myReadState = readState;

    RenderCore_PlatformDX12* dx12Platform = RenderCore::GetPlatformDX12();
    ID3D12Device* device = dx12Platform->GetDevice();

    const GpuMemoryAccessType gpuMemAccess = (GpuMemoryAccessType)someProperties.myCpuAccess;
    GpuMemoryAllocationDX12 gpuMemory = dx12Platform->AllocateGpuMemory(GpuMemoryType::BUFFER, gpuMemAccess, pitch, myAlignment);
    ASSERT(gpuMemory.myHeap != nullptr);
    
    uint64 alignedHeapOffset = MathUtil::Align(gpuMemory.myOffsetInHeap, myAlignment);
    CheckD3Dcall(device->CreatePlacedResource(gpuMemory.myHeap, alignedHeapOffset, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&storageDx12->myResource)));

    storageDx12->myGpuMemory = gpuMemory;

    if (pInitialData != nullptr)
    {
      if (gpuMemAccess == GpuMemoryAccessType::CPU_WRITE)
      {
        void* dest = Lock(GpuResoruceLockOption::WRITE);
        ASSERT(dest != nullptr);
        memcpy(dest, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
        Unlock();
      }
      else
      {
        RenderCore::UpdateBufferData(this, 0u, pInitialData, someProperties.myNumElements * someProperties.myElementSizeBytes);
      }
    }
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Destroy()
  {
    GpuResourceStorageDX12* storageDx12 = (GpuResourceStorageDX12*)myStorage.get();
    if (storageDx12 != nullptr && storageDx12->myGpuMemory.myHeap != nullptr)
    {
      storageDx12->myResource.Reset();
      RenderCore::GetPlatformDX12()->FreeGpuMemory(storageDx12->myGpuMemory);
    }

    myStorage = nullptr;
  }
//---------------------------------------------------------------------------//
  void* GpuBufferDX12::Lock(GpuResoruceLockOption eLockOption, uint64 uOffsetElements /* = 0u */, uint64 uNumElements /* = 0u */) const
  {
    ASSERT(uOffsetElements + uNumElements <= myProperties.myNumElements);

    if (uNumElements == 0u)
      uNumElements = myProperties.myNumElements - uOffsetElements;

    D3D12_RANGE range;
    range.Begin = uOffsetElements * myProperties.myElementSizeBytes;
    range.End = range.Begin + uNumElements * myProperties.myElementSizeBytes;

    const bool isCpuWritable = myProperties.myCpuAccess == GpuMemoryAccessType::CPU_WRITE;
    const bool isCpuReadable = myProperties.myCpuAccess == GpuMemoryAccessType::CPU_READ;

    const bool wantsWrite = eLockOption == GpuResoruceLockOption::READ_WRITE || eLockOption == GpuResoruceLockOption::WRITE;
    const bool wantsRead = eLockOption == GpuResoruceLockOption::READ_WRITE || eLockOption == GpuResoruceLockOption::READ;

    if ((wantsWrite && !isCpuWritable) || (wantsRead && !isCpuReadable))
      return nullptr;

    // TODO: Do something with the current usage type? Transition it into something correct? Early-out?

    const GpuResourceStorageDX12* storageDx12 = static_cast<GpuResourceStorageDX12*>(myStorage.get());

    void* mappedData = nullptr;
    CheckD3Dcall(storageDx12->myResource->Map(0, &range, &mappedData));

    return mappedData;
  }
//---------------------------------------------------------------------------//
  void GpuBufferDX12::Unlock(uint64 anOffsetElements /* = 0u */, uint64 aNumElements /* = 0u */) const
  {
    if (anOffsetElements == 0u && aNumElements == 0u)
      aNumElements = myProperties.myNumElements;

    ASSERT(anOffsetElements + aNumElements <= myProperties.myNumElements);

    D3D12_RANGE range;
    range.Begin = anOffsetElements * myProperties.myElementSizeBytes;
    range.End = range.Begin + aNumElements * myProperties.myElementSizeBytes;

    const GpuResourceStorageDX12* storageDx12 = static_cast<GpuResourceStorageDX12*>(myStorage.get());
    storageDx12->myResource->Unmap(0u, &range);
  }
//---------------------------------------------------------------------------//
}
