#pragma once

#include <unordered_map>

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "DX12Prerequisites.h"
#include "RenderContext.h"
#include "CommandContextBaseDX12.h"

namespace Fancy{ namespace Rendering{
  class Descriptor;
  class GpuResource;
  class RenderOutput;
  class GpuProgramPipeline;
  struct BlendState;
  struct DepthStencilState;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorHeapDX12;
  class GpuResourceDX12;
  class CommandAllocatorPoolDX12;
//---------------------------------------------------------------------------//
  class RenderContextDX12 : public RenderContext, public CommandContextBaseDX12
  {
  public:
    RenderContextDX12();
    ~RenderContextDX12() override;

    void Reset() override;
    uint64 ExecuteAndReset(bool aWaitForCompletion) override;

    static D3D12_GRAPHICS_PIPELINE_STATE_DESC GetNativePSOdesc(const GraphicsPipelineState& aState);

    void ClearRenderTarget(Texture* aTexture, const float* aColor) override;
    void ClearDepthStencilTarget(Texture* aTexture, float aDepthClear, uint8 aStencilClear, uint32 someClearFlags = (uint32)DepthStencilClearFlags::CLEAR_ALL) const override;

    // Root arguments:
    void SetReadTexture(const Texture* aTexture, uint32 aRegisterIndex) const override;
    void SetWriteTexture(const Texture* aTexture, uint32 aRegisterIndex) const override;
    void SetReadBuffer(const GpuBuffer* aBuffer, uint32 aRegisterIndex) const override;
    void SetConstantBuffer(const GpuBuffer* aConstantBuffer, uint32 aRegisterIndex) const override;

    // Descriptor tables:
    void SetMultipleResources(const Descriptor* someResources, uint32 aResourceCount, uint32 aRegisterIndex) override;

    void SetGpuProgramPipeline(const SharedPtr<GpuProgramPipeline>& aGpuProgramPipeline) override;

    void RenderGeometry(const Geometry::GeometryData* pGeometry) override;
    
  protected:
    static std::unordered_map<uint, ID3D12PipelineState*> ourPSOcache;

    void Reset_Internal() override;

    void ApplyViewport();
    void ApplyPipelineState();
    void ApplyRenderTargets();
  };
//---------------------------------------------------------------------------//
} } }
