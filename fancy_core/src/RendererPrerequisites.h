#ifndef INCLUDE_RENDERERPREREQUISITES_H
#define INCLUDE_RENDERERPREREQUISITES_H

// DEPRECATED: This will be selected by the VS-platform config
// This define selects the render-system
// #define RENDERER_OPENGL4
// #define RENDERER_DX12

#if defined (RENDERER_OPENGL4)
	#include "OpenGLprerequisites.h"
#elif defined (RENDERER_DX12)
	#include "DX12Prerequisites.h"
#endif // RENDERER_OPENGL4

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"

//---------------------------------------------------------------------------//
namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Geometry {
    class GeometryData;
  }
//---------------------------------------------------------------------------//
  namespace Rendering {
//---------------------------------------------------------------------------//
    namespace MultiBuffering {
      enum { kGpuMultiBufferingCount = 2u };
      
      uint32 getCurrentBufferIndex();
    }
//---------------------------------------------------------------------------//
    namespace Constants {
      enum {
        kMaxNumRenderTargets = 7u, // (-1 for depth-stencil target)
        kMaxNumTextureSamplers = 32u,
        kMaxNumReadBuffers = 32u,
        kMaxNumWriteBuffers = 8u,
        kMaxNumReadTextures = 32u,
        kMaxNumWriteTextures = 8u,
        kMaxNumBoundConstantBuffers = 12u,
        kMaxNumGpuProgramResources = 32u,
        kMaxNumConstantBufferElements = 128u,
        kMaxNumGeometriesPerSubModel = 128u,
#if defined (RENDERER_OPENGL4)
        kNumRenderThreads = 1u,
#elif defined (RENDERER_DX12)
        kNumRenderThreads = 1u
#endif // RENDERER_OPENGL4
      };
    }
//---------------------------------------------------------------------------//
  // Forward-declarations of common rendering classes
    class VolumeMesh;
    class Material;
    class RenderOperation;
    class Texture;
    class TextureSampler;
    class GpuBuffer;
    class GpuProgram;
    class GpuProgramPipeline;
    class GpuProgramCompiler;
    class GpuProgramResource;
  //---------------------------------------------------------------------------//
  // Forward-declarations of platform-dependent rendering classes
    #if defined (RENDERER_OPENGL4)
      namespace GL4 
      {
        class RendererGL4;
        class TextureGL4;
        class GpuProgramGL4;
        class GpuBufferGL4;
        class GpuProgramPipelineGL4;
        class GpuProgramCompilerGL4;
        class TextureSamplerGL4;
        class GpuProgramResourceGL4;
        class GpuDataInterfaceGL4;
        class RenderingSubsystemGL4;
      }

      #define PLATFORM_DEPENDENT_NAME(name) Fancy::Rendering::GL4::name##GL4
      #define PLATFORM_DEPENDENT_INCLUDE_RENDERER   "RendererGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_TEXTURE    "TextureGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_TEXTURESAMPLER "TextureSamplerGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAM "GpuProgramGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_GPUBUFFER "GpuBufferGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMPIPELINE "GpuProgramPipelineGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMCOMPILER "GpuProgramCompilerGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMRESOURCE "GpuProgramResourceGL4.h"
      #define PLATFORM_DEPENDENT_INCLUDE_GPUDATAINTERFACE "GpuDataInterfaceGL4.h"

    #elif defined (RENDERER_DX11)
      namespace DX11 {}
      #define PLATFORM_DEPENDENT_NAME(name) Fancy::Rendering::DX11::name##DX11
	#elif defined (RENDERER_DX12)
		namespace DX12
		{
			class RendererDX12;
			class TextureDX12;
			class GpuProgramDX12;
			class GpuBufferDX12;
			class GpuProgramPipelineDX12;
			class GpuProgramCompilerDX12;
			class TextureSamplerDX12;
			class GpuProgramResourceDX12;
      class GpuDataInterfaceDX12;
      class RenderingSubsystemDX12;
		}

		#define PLATFORM_DEPENDENT_NAME(name) Fancy::Rendering::DX12::name##DX12
		#define PLATFORM_DEPENDENT_INCLUDE_RENDERER   "RendererDX12.h"
		#define PLATFORM_DEPENDENT_INCLUDE_TEXTURE    "TextureDX12.h"
		#define PLATFORM_DEPENDENT_INCLUDE_TEXTURESAMPLER "TextureSamplerDX12.h"
		#define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAM "GpuProgramDX12.h"
		#define PLATFORM_DEPENDENT_INCLUDE_GPUBUFFER "GpuBufferDX12.h"
		#define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMPIPELINE "GpuProgramPipelineDX12.h"
		#define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMCOMPILER "GpuProgramCompilerDX12.h"
		#define PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMRESOURCE "GpuProgramResourceDX12.h"
    #define PLATFORM_DEPENDENT_INCLUDE_GPUDATAINTERFACE "GpuDataInterfaceDX12.h"
    #endif // RENDERER
  //---------------------------------------------------------------------------//

  //-----------------------------------------------------------------------//
    enum class CompFunc {
      NEVER = 0,
      LESS,
      EQUAL,
      LEQUAL,
      GREATER,
      NOTEQUAL,
      GEQUAL,
      ALWAYS,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class StencilOp {
      KEEP = 0,
      ZERO,
      REPLACE,
      INCREMENT_CLAMP,
      DECREMENT_CLAMP,
      INVERT,
      INCEMENT_WRAP,
      DECREMENT_WRAP,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class FillMode {
      WIREFRAME = 0,
      SOLID,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class CullMode { 
      NONE = 0,
      FRONT,
      BACK,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class WindingOrder {
      CCW = 0,
      CW,

      NUM
    };
  //-----------------------------------------------------------------------//    
    enum class BlendInput {
      ZERO = 0,
      ONE,
      SRC_COLOR,
      INV_SRC_COLOR,
      SRC_ALPHA,
      INV_SRC_ALPHA,
      DEST_ALPHA,
      INV_DEST_ALPHA,
      DEST_COLOR,
      INV_DEST_COLOR,
      SRC_ALPHA_CLAMPED,
      BLEND_FACTOR,  // DX11-only
      INV_BLEND_FACTOR,
      SRC1_COLOR,
      INV_SRC1_COLOR,
      SRC1_ALPHA,
      INV_SRC1_ALPHA,

      // OpenGL-only:
      CONSTANT_COLOR,
      INV_CONSTANT_COLOR,
      CONSTANT_ALPHA,
      INV_CONSTANT_ALPHA,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class BlendOp {
      ADD = 0,
      SUBTRACT,
      REV_SUBTRACT,
      MIN,
      MAX,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class ShaderStage {
      VERTEX        = 0,
      FRAGMENT,      
      GEOMETRY,      
      TESS_HULL,     
      TESS_DOMAIN,
      COMPUTE,                  
      
      NUM,
      NUM_NO_COMPUTE = NUM - 1,
      NONE
    };
  //---------------------------------------------------------------------------//
    enum class ShaderStageFlag {
      VERTEX        = 0x01,
      FRAGMENT      = 0x02,      
      GEOMETRY      = 0x04,      
      TESS_HULL     = 0x08,     
      TESS_DOMAIN   = 0x10,   
      COMPUTE       = 0x20,                  

      ALL           = 0xFF
    };
  //-----------------------------------------------------------------------//
    enum class FaceType {
      FRONT = 0,
      BACK,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class DataFormat {
      NONE = 0,
      SRGB_8_A_8,
      RGBA_8,
      SRGB_8,
      RGB_8,
      RGB_11_11_10F,
      RGBA_16F,
      RGB_16F,
      RG_16F,
      R_16F,
      RGBA_32F,
      RGB_32F,
      RG_32F,
      R_32F,
      RGBA_32UI,
      RGB_32UI,
      RG_32UI,
      R_32UI,
      RGBA_16UI,
      RGB_16UI,
      RG_16UI,
      R_16UI,
      RGBA_8UI,
      RGB_8UI,
      RG_8UI,
      R_8UI,
      RGBA_32I,
      RGB_32I,
      RG_32I,
      R_32I,
      RGBA_16I,
      RGB_16I,
      RG_16I,
      R_16I,
      RGBA_8I,
      RGB_8I,
      RG_8I,
      R_8I,
      DS_24_8,

      // Compressed formats go here...
      // TODO: Find a way to declare compressed formats
  
      NUM
    };
  //---------------------------------------------------------------------------//
    enum class GpuResourceAccessFlags {
      /// No special access flags
      NONE                = 0x00000000,
      /// Allow CPU read-access only
      READ                = 0x00000001,
      /// Allow CPU write-access only
      WRITE               = 0x00000002,
      /// CPU will change data frequently
      DYNAMIC              = 0x00000008,
      /// CPU-access may remain valid even during GPU-access
      PERSISTENT_LOCKABLE = 0x00000010, //GL4: GL_MAP_PERSISTENT_BIT
      /// Changes from CPU/GPU are immediately visible to GPU/CPU
      COHERENT            = 0x00000020,
      /// Try to use CPU-RAM as backing storage
      PREFER_CPU_STORAGE  = 0x00000040
    };
  //---------------------------------------------------------------------------//
    // Note: Could make bitfield-flags instead, but this would suggest that each field
    // can be combined - even in DX11. Instead TODO: Reduce/expand this set of options during experiments
    enum class GpuResoruceLockOption {
      READ = 0,
      WRITE,
      READ_WRITE,
      WRITE_DISCARD,
      READ_WRITE_DISCARD,
      READ_UNSYNCHRONIZED,  // GL4-only?
      WRITE_UNSYNCHRONIZED,
      READ_WRITE_UNSYNCHRONIZED,
      READ_PERSISTENT,      // GL4-only?
      WRITE_PERSISTENT,
      READ_WRITE_PERSISTENT,
      READ_PERSISTENT_COHERENT,
      WRITE_PERSISTENT_COHERENT,
      READ_WRITE_PERSISTENT_COHERENT,
      
      NUM
    };
  //---------------------------------------------------------------------------//
    struct TextureCreationParams {
      TextureCreationParams() : path(""), u16Width(0u), u16Height(0u), u16Depth(0u),
        eFormat(DataFormat::NONE), uAccessFlags(0u), pPixelData(nullptr),
        uPixelDataSizeBytes(0), bIsDepthStencil(false), myIsExternalTexture(true), myInternalRefIndex(~0u), u8NumMipLevels(0u) {}

      uint16 u16Width;
      uint16 u16Height;
      uint16 u16Depth;
      DataFormat eFormat;
      uint32 uAccessFlags;
      /// true if the texture should be a depth- or depthStencil Texture
      bool bIsDepthStencil;
      /// (optional) pointer to pixelData
      void* pPixelData;
      /// (optional) size of the pixelData
      uint uPixelDataSizeBytes;
      String path;
      bool myIsExternalTexture;
      uint32 myInternalRefIndex;

      uint8 u8NumMipLevels;
    };
 //---------------------------------------------------------------------------//
    enum class GpuBufferUsage {
      CONSTANT_BUFFER = 0u,
      VERTEX_BUFFER,
      INDEX_BUFFER,
      DRAW_INDIRECT_BUFFER,
      DISPATCH_INDIRECT_BUFFER,
      RESOURCE_BUFFER,
      RESOURCE_BUFFER_RW,
      RESOURCE_BUFFER_LARGE,
      RESOURCE_BUFFER_LARGE_RW
    };
 //---------------------------------------------------------------------------//
    struct GpuBufferCreationParams {
      GpuBufferCreationParams() : uNumElements(0u), myInternalRefIndex(~0u), bIsMultiBuffered(false), 
        uElementSizeBytes(0u), ePrimaryUsageType(GpuBufferUsage::CONSTANT_BUFFER), uAccessFlags(0u) {}

      uint uNumElements;
      uint32 uElementSizeBytes;
      uint32 uAccessFlags;
      uint32 myInternalRefIndex;
      GpuBufferUsage ePrimaryUsageType;
      bool bIsMultiBuffered;
    };
 //---------------------------------------------------------------------------//
    enum class VertexSemantics {
      NONE = 0,

      POSITION,
      NORMAL,
      TANGENT,
      BITANGENT,
      COLOR,
      TEXCOORD,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class SamplerFilterMode {
      NEAREST = 0,
      BILINEAR,
      TRILINEAR,
      ANISOTROPIC,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class SamplerAddressMode {
      WRAP = 0,
      CLAMP_EDGE,
      MIRROR_CLAMP_EDGE,
      CLAMP_BORDER,
      REPEAT,
      MIRROR_REPEAT,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class EMaterialPass {
      SOLID_GBUFFER = 0,
      SOLID_FORWARD,
      TRANSPARENT_FORWARD,
      // More to come...
      NUM
    };
//---------------------------------------------------------------------------//
  } // end of namespace Rendering 
  }  // end of namespace Fancy
//---------------------------------------------------------------------------//

#endif  // INCLUDE_RENDERERPREREQUISITES