#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "RenderContext.h"
#include "DataFormat.h"
#include "CommandListType.h"
#include "ScopedPtr.h"

#if defined (RENDERER_DX12)

#include "Texture.h"
#include "FenceDX12.h"
#include "RenderContextDX12.h"
#include "CommandAllocatorPoolDX12.h"
#include "DescriptorHeapPoolDX12.h"

namespace Fancy {
class RenderWindow;
}

namespace Fancy { namespace Rendering {
  class ShaderResourceInterface;
  class GeometryVertexLayout;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorHeapPoolDX12;
//---------------------------------------------------------------------------//
	class RenderOutputDX12
	{
	public:
    RenderOutputDX12();
		virtual ~RenderOutputDX12();
		void postInit(); /// Sets the render-system to a valid state. Should be called just before the first frame
		void beginFrame();
		void endFrame();

    Texture* GetBackbuffer() { return &myBackbuffers[myCurrBackbufferIndex]; }
    Texture* GetDefaultDepthStencilBuffer() { return &myDefaultDepthStencil; }
    RenderWindow* GetWindow();

	protected:
    void CreateSwapChain();
    void CreateBackbufferResources();

    void OnWindowResized(uint aWidth, uint aHeight);

    SharedPtr<RenderWindow> myWindow;
    uint myCurrBackbufferIndex;
    static const uint kBackbufferCount = 2u;
    ComPtr<IDXGISwapChain3> mySwapChain;
    Texture myBackbuffers[kBackbufferCount];
    Texture myDefaultDepthStencil;
	};
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//

  // TODO: Make the RenderCore the main API entry point for render-related stuff (avoid having to call Fancy::GetRenderer() everywhere)

  class RenderCoreDX12
  {
  public:
    /// Initializes platform-dependent rendering stuff
    static void InitPlatform();
    /// Shutdown of platform-dependent rendering stuff
    static void ShutdownPlatform();

    static DXGI_FORMAT GetFormat(DataFormat aFormat);
    static DataFormat ResolveFormat(DataFormat aFormat);
    static DataFormatInfo GetFormatInfo(DXGI_FORMAT aFormat);
    static D3D12_COMMAND_LIST_TYPE GetCommandListType(CommandListType aType);

    static ID3D12Device* GetDevice() { return ourDevice.Get(); }

    static Rendering::ShaderResourceInterface* 
      GetShaderResourceInterface(const D3D12_ROOT_SIGNATURE_DESC& anRSdesc, ComPtr<ID3D12RootSignature> anRS = nullptr);

    static void WaitForFence(CommandListType aType, uint64 aFenceVal);
    static bool IsFenceDone(CommandListType aType, uint64 aFenceVal);

    static uint64 ExecuteCommandList(ID3D12CommandList* aCommandList);

    static ID3D12CommandAllocator* GetCommandAllocator(CommandListType aCmdListType);
    static void ReleaseCommandAllocator(ID3D12CommandAllocator* anAllocator, CommandListType aCmdListType, uint64 aFenceVal);
    
    static Descriptor AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    static DescriptorHeapPoolDX12* GetDescriptorHeapPool() { return ourDynamicDescriptorHeapPool; }
    static DescriptorHeapDX12* AllocateDynamicDescriptorHeap(uint32 aDescriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    static void ReleaseDynamicDescriptorHeap(DescriptorHeapDX12* aHeap, CommandListType aCmdListType, uint64 aFenceVal);

    static ComPtr<ID3D12Device> ourDevice;

    static CommandAllocatorPoolDX12* ourCommandAllocatorPools [(uint) CommandListType::NUM];
    static ComPtr<ID3D12CommandQueue> ourCommandQueues[(uint) CommandListType::NUM];
    static FenceDX12 ourCmdListDoneFences[(uint)CommandListType::NUM];

  protected:
    static DescriptorHeapPoolDX12* ourDynamicDescriptorHeapPool;
    static DescriptorHeapDX12 ourStaticDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    RenderCoreDX12() {}
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Renderer::DX12

#endif