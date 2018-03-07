#include "RenderOutput.h"
#include "Window.h"
#include "RenderCore.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle)
    : myCurrBackbufferIndex(0u)
    , myDefaultDepthStencil(nullptr)
  {
    memset(myBackbuffers, 0u, sizeof(myBackbuffers));

    Fancy::WindowParameters params;
    params.myTitle = "Fancy";
    params.myWidth = 800u;
    params.myHeight = 600u;

    HINSTANCE instanceHandle = static_cast<HINSTANCE>(aNativeInstanceHandle);

    myWindow = Window::Create(instanceHandle, params);
    myWindow->myOnResize.Connect(this, &RenderOutput::OnWindowResized);
  }
//---------------------------------------------------------------------------//
  RenderOutput::~RenderOutput()
  {
    myWindow->myOnResize.DetachObserver(this);
  }
//---------------------------------------------------------------------------//
  void RenderOutput::CreateBackbufferResources()
  {
    TextureParams dsTexParams;
    dsTexParams.bIsDepthStencil = true;
    dsTexParams.eFormat = DataFormat::DS_24_8;
    dsTexParams.myIsExternalTexture = false;
    dsTexParams.myIsRenderTarget = false;
    dsTexParams.myIsShaderWritable = false;
    dsTexParams.u16Width = myWindow->GetWidth();
    dsTexParams.u16Height = myWindow->GetHeight();
    dsTexParams.u8NumMipLevels = 1u;

    myDefaultDepthStencil = RenderCore::CreateTexture(dsTexParams);
    ASSERT(myDefaultDepthStencil != nullptr);

    for (uint i = 0u; i < kBackbufferCount; i++)
    {
      TextureParams backbufferParams;
      backbufferParams.myIsRenderTarget = true;
      backbufferParams.eFormat = DataFormat::RGBA_8;
      backbufferParams.u16Width = myWindow->GetWidth();
      backbufferParams.u16Height = myWindow->GetHeight();
      backbufferParams.u16Depth = 1u;

      myBackbuffers[i] = RenderCore::CreateTexture(backbufferParams);
    }
  }
//---------------------------------------------------------------------------//  
}

Internal ref-indices for multiple RenderOutputs??