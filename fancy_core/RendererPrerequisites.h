#pragma once

/// Enables various sanity-checks and validations
#define FANCY_RENDERER_USE_VALIDATION 1
#define FANCY_RENDERER_DEBUG 1
#define FANCY_RENDERER_DEBUG_MEMORY_ALLOCS FANCY_RENDERER_DEBUG

#define FANCY_RENDERER_LOG_RESOURCE_BARRIERS 1

#define FANCY_RENDERER_SUPPORT_MESH_SHADERS 0

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace RenderConstants 
  {
    enum 
    {
      kMaxNumRenderTargets = 8u,
      kMaxNumTextureSamplers = 32u,
      kMaxNumReadBuffers = 32u,
      kMaxNumWriteBuffers = 8u,
      kMaxNumReadTextures = 32u,
      kMaxNumWriteTextures = 8u,
      kMaxNumBoundConstantBuffers = 12u,
      kMaxNumShaderResources = 32u,
      kMaxNumConstantBufferElements = 128u,
      kMaxNumGeometriesPerSubModel = 128u,
      kMaxNumRenderContexts = 256u,
      kNumRenderThreads = 1u,
    };
  }
//---------------------------------------------------------------------------//
  enum class RenderPlatformType
  {
    DX12 = 0,
    VULKAN,
  };
//---------------------------------------------------------------------------//
  struct RenderPlatformCaps
  {
    unsigned int myMaxNumVertexAttributes = 32u;
    unsigned int myCbufferPlacementAlignment = 1u;
    unsigned int myTextureRowAlignment = 1u;
    unsigned int myTextureSubresourceBufferAlignment = 10;
    unsigned int myMaxTextureAnisotropy = 16u;
    bool myHasAsyncCompute = false;
    bool myHasAsyncCopy = false;
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy