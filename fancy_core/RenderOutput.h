#pragma once

namespace Fancy {
  class Window;
}

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc;
  struct GpuProgramPipelineDesc;
  struct DepthStencilStateDesc;
  struct BlendStateDesc;
  class BlendState;
  class DepthStencilState;
//---------------------------------------------------------------------------//
  class RenderOutput
  {
  public:
    explicit RenderOutput(void* aNativeInstanceHandle);
    virtual ~RenderOutput();

    virtual void BeginFrame() = 0;
    virtual void EndFrame()= 0;

    Texture* GetBackbuffer() const { return myBackbuffers[myCurrBackbufferIndex].get(); }
    Texture* GetDefaultDepthStencilBuffer() const { return myDefaultDepthStencil.get(); }
    Window* GetWindow() const { return myWindow.get(); }

  protected:
    virtual void PrepareForFirstFrame() = 0;
    virtual void OnWindowResized(uint aWidth, uint aHeight) = 0;
    virtual void CreateBackbufferResources();

    static const uint kBackbufferCount = 2u;
    uint myCurrBackbufferIndex;
    SharedPtr<Window> myWindow;
    SharedPtr<Texture> myBackbuffers[kBackbufferCount];
    SharedPtr<Texture> myDefaultDepthStencil;
  };
//---------------------------------------------------------------------------//
} }