#include "RenderOutput.h"
#include "Window.h"
#include "RenderCore.h"
#include "TextureViewProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  RenderOutput::RenderOutput(void* aNativeInstanceHandle)
    : myCurrBackbufferIndex(0u)
  {
    Fancy::WindowParameters params;
    params.myTitle = "Fancy";
    params.myWidth = 1280u;
    params.myHeight = 720u;

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
  void RenderOutput::OnWindowResized(uint aWidth, uint aHeight)
  {
    CreateBackbufferResources();
  }
//---------------------------------------------------------------------------//
  void RenderOutput::CreateBackbufferResources()
  {
    TextureProperties dsTexProps;
    dsTexProps.myDimension = GpuResourceDimension::TEXTURE_2D;
    dsTexProps.bIsDepthStencil = true;
    dsTexProps.eFormat = DataFormat::DS_24_8;
    dsTexProps.myIsRenderTarget = false;
    dsTexProps.myIsShaderWritable = false;
    dsTexProps.myWidth = myWindow->GetWidth();
    dsTexProps.myHeight = myWindow->GetHeight();
    dsTexProps.myNumMipLevels = 1u;

    SharedPtr<Texture> dsTexture = RenderCore::CreateTexture(dsTexProps);
    ASSERT(dsTexture != nullptr);

    // DepthStencil DSVs
    {
      TextureViewProperties props;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myIsRenderTarget = true;
      props.myFormat = DataFormat::DS_24_8;
      myDepthStencilDsv = RenderCore::CreateTextureView(dsTexture, props);
      ASSERT(myDepthStencilDsv != nullptr);

      props.myIsDepthReadOnly = true;
      props.myIsStencilReadOnly = true;
      myDepthStencilDsv_ReadOnly = RenderCore::CreateTextureView(dsTexture, props);
      ASSERT(myDepthStencilDsv_ReadOnly != nullptr);
    }

    // Depth SRV
    {
      TextureViewProperties props;
      props.myDimension = GpuResourceDimension::TEXTURE_2D;
      props.myFormat = DataFormat::R_24UNORM_8X;
      myDepthStencilDsv = RenderCore::CreateTextureView(dsTexture, props);
      ASSERT(myDepthStencilDsv != nullptr);
    }

    for (uint i = 0u; i < kBackbufferCount; i++)
    {
      TextureProperties backbufferProps;
      backbufferProps.myDimension = GpuResourceDimension::TEXTURE_2D;
      backbufferProps.myIsRenderTarget = true;
      backbufferProps.eFormat = DataFormat::RGBA_8;
      backbufferProps.myWidth = myWindow->GetWidth();
      backbufferProps.myHeight = myWindow->GetHeight();

      SharedPtr<Texture> backbuffer = RenderCore::CreateTexture(backbufferProps);

      // Backbuffer RTV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = DataFormat::RGBA_8;
        props.myIsRenderTarget = true;
        myBackbufferRtv[i] = RenderCore::CreateTextureView(backbuffer, props);
        ASSERT(myBackbufferRtv[i] != nullptr);
      }

      // Backbuffer SRV
      {
        TextureViewProperties props;
        props.myDimension = GpuResourceDimension::TEXTURE_2D;
        props.myFormat = DataFormat::RGBA_8;
        myBackbufferSrv[i] = RenderCore::CreateTextureView(backbuffer, props);
        ASSERT(myBackbufferSrv[i] != nullptr);
      }
    }
  }
//---------------------------------------------------------------------------//  
}
