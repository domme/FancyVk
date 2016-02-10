#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include <unordered_map>
#include "DepthStencilState.h"
#include "BlendState.h"
#include "GpuDynamicAllocatorDX12.h"

namespace Fancy{ namespace Rendering{
class Renderer;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuResourceDX12;
//---------------------------------------------------------------------------//
  struct InputLayout
  {
    std::vector<D3D12_INPUT_ELEMENT_DESC> myElements;

    // Combined hash of ShaderVertexLayout, GeometryVertexLayout as well as any instancing data
    // should be used to quickly determine if an inputlayout can be re-used for changing shader- geometry- or instance data 
    // TODO: Important: Implement InputLayout in DX12
    uint myHash;
  };
//---------------------------------------------------------------------------//
  struct PipelineState
  {
    PipelineState();
    uint getHash();
    D3D12_GRAPHICS_PIPELINE_STATE_DESC toNativePSOdesc();

    FillMode myFillMode;
    CullMode myCullMode;
    WindingOrder myWindingOrder;
    DepthStencilState myDepthStencilState;
    BlendState myBlendState;
    const GpuProgram* myShaderStages[static_cast<uint>(ShaderStage::NUM)];
    uint8 myNumRenderTargets;
    DataFormat myRTVformats[Constants::kMaxNumRenderTargets];
    DataFormat myDSVformat;
    InputLayout myInputLayout;

    bool myIsDirty : 1;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class RenderContextDX12
  {
  public:
    enum Constants
    {
      kMaxNumCachedResourceBarriers = 16
    };

    explicit RenderContextDX12(Renderer& aRenderer);
    ~RenderContextDX12();

    void SetGraphicsRootSignature(ID3D12RootSignature* aRootSignature);
    void setReadTexture(const Texture* pTexture, const uint8 u8RegisterIndex);
    void setWriteTexture(const Texture* pTexture, const uint8 u8RegisterIndex);
    void setReadBuffer(const GpuBuffer* pBuffer, const uint8 u8RegisterIndex);
    void setConstantBuffer(const GpuBuffer* pConstantBuffer, const uint8 u8RegisterIndex);
    void setTextureSampler(const TextureSampler* pSampler, const uint8 u8RegisterIndex);
    void setGpuProgram(const GpuProgram* pProgram, const ShaderStage eShaderStage);
    void setViewport(const glm::uvec4& uViewportParams); /// x, y, width, height
    const glm::uvec4 getViewport() const { return myViewportParams; } /// x, y, width, height
    void setBlendState(const BlendState& clBlendState);
    void setDepthStencilState(const DepthStencilState& clDepthStencilState);
    void setFillMode(const FillMode eFillMode);
    void setCullMode(const CullMode eCullMode);
    void setWindingOrder(const WindingOrder eWindingOrder);

    void setDepthStencilRenderTarget(Texture* pDStexture);
    void setRenderTarget(Texture* pRTTexture, const uint8 u8RenderTargetIndex);
    void removeAllRenderTargets();

    void renderGeometry(const Geometry::GeometryData* pGeometry);

    void CopySubresources(ID3D12Resource* aDestResource, ID3D12Resource* aSrcResource, uint aFirstSubresource, uint aSubResourceCount);
    void InitBufferData(GpuResourceDX12* aBuffer, void* aDataPtr);

    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, ID3D12DescriptorHeap* aDescriptorHeap);
    void SetCommandAllocator(ID3D12CommandAllocator* aCommandAllocator);

    uint64 ExecuteAndFinish(bool aWaitForCompletion = false);

    void Release();

  protected:
    void applyViewport();
    void applyPipelineState();
    void ApplyDescriptorHeaps();
    void KickoffResourceBarriers();

    static std::unordered_map<uint, ID3D12PipelineState*> ourPSOcache;
    Renderer& myRenderer;

    PipelineState myState;
    uint myPSOhash;
    ID3D12PipelineState* myPSO;

    glm::uvec4 myViewportParams;
    bool myViewportDirty;

    ID3D12RootSignature* myRootSignature;
    ID3D12GraphicsCommandList* myCommandList;
    ID3D12CommandAllocator* myCommandAllocator;
    FixedArray<D3D12_RESOURCE_BARRIER, kMaxNumCachedResourceBarriers> myWaitingResourceBarriers;
    
    ID3D12DescriptorHeap* myDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    bool myDescriptorHeapsDirty;

    GpuDynamicAllocatorDX12 myCpuVisibleAllocator;
    GpuDynamicAllocatorDX12 myGpuOnlyAllocator;
  };
//---------------------------------------------------------------------------//
} } }
