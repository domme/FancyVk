#ifndef INCLUDE_GPUPROGRAMGL4_H
#define INCLUDE_GPUPROGRAMGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramResource.h"
#include "VertexInputLayout.h"
#include "StaticManagedObject.h"

namespace Fancy{namespace IO{
  class Serializer;
}}

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  struct ShaderStageInterfaceElement
  {
    ShaderStageInterfaceElement() : 
      uLocation(0u), uArraySize(0u), uOffset(0u), 
      uBlockIndex(0u), eTypeGL(0u), uAtomicCountBufIndex(0u) {}

    ObjectName name;
    GLuint uLocation;
    GLuint uArraySize;
    GLuint uArrayStride;
    GLuint uOffset;
    GLuint uBlockIndex;
    GLenum eTypeGL;
    GLuint uAtomicCountBufIndex;
  };
//---------------------------------------------------------------------------//
  struct ShaderStageFragmentOutput
  {
    ShaderStageFragmentOutput() :
      uRtIndex(0u), uLocation(0u), eFormat(DataFormat::NONE), uFormatComponentCount(1u) {}

    /// Name in glsl
    ObjectName name;
    /// Rendertarget index
    GLuint uRtIndex;
    /// Register index
    GLuint uLocation;
    /// expected format of the output
    DataFormat eFormat;
     // Multiplier for eFormat. Used for multi-component elements (e.g. Matrices)
    uint8 uFormatComponentCount; 
  };
//---------------------------------------------------------------------------//
  const uint32 kMaxNumShaderStageInterfaceElements = 16;
  typedef FixedArray<ShaderStageInterfaceElement, kMaxNumShaderStageInterfaceElements> ShaderStageInterfaceList;
  typedef FixedArray<ShaderStageFragmentOutput, kMaxNumShaderStageInterfaceElements> ShaderStageFragmentOutputList;
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  struct GpuProgramDescriptionGL4
  {
    GpuProgramDescriptionGL4();
    ObjectName name;
    GLuint uProgramHandleGL;
    ShaderStage eShaderStage;
    VertexInputLayout clVertexInputLayout;
    ShaderStageInterfaceList vInputInterfaces;
    ShaderStageInterfaceList vOutputInterfaces;
    ShaderStageFragmentOutputList vFragmentOutputs;
    GpuResourceInfoList vReadTextureInfos;
    GpuResourceInfoList vReadBufferInfos;
    GpuResourceInfoList vWriteTextureInfos;
    GpuResourceInfoList vWriteBufferInfos;
  };
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  class GpuProgramGL4
  {
  //---------------------------------------------------------------------------//
    friend class GpuProgramCompilerGL4;
    friend class RendererGL4;
  //---------------------------------------------------------------------------//
    public:
      GpuProgramGL4();
      ~GpuProgramGL4();
    //---------------------------------------------------------------------------//
      void init(const GpuProgramDescriptionGL4& _desc);
    //---------------------------------------------------------------------------//
      const ObjectName& getName() const {return m_Name;}
      const ObjectName getTypeName() const { return _N(GpuProgram); }
      void serialize(IO::Serializer* aSerializer);

      GLuint getProgramHandle() const {return m_uProgramHandleGL;}
      ShaderStage getShaderStage() const {return m_eShaderStage;}
      const GpuResourceInfoList& getReadTextureInfoList() const {return m_vReadTextureInfos;}
      const GpuResourceInfoList& getReadBufferInfoList() const {return m_vReadBufferInfos;}
      const GpuResourceInfoList& getWriteTextureInfoList() const {return m_vWriteTextureInfos;}
      const GpuResourceInfoList& getWriteBufferInfoList() const {return m_vWriteBufferInfos;}
      const VertexInputLayout* getVertexInputLayout() const {return &m_clVertexInputLayout;}
    //---------------------------------------------------------------------------//
    protected:
      void destroy();
      
      /// Name of the shader as loaded from disk
      ObjectName m_Name;
      /// GL-internal handle to the program object
      GLuint m_uProgramHandleGL;
      /// ShaderStage this program defines
      ShaderStage m_eShaderStage;
      /// Expected layout of incoming vertices. Only valid for vertex-programs
      VertexInputLayout m_clVertexInputLayout;
      /// List of incoming inter-stage varyings. Not valid for vertex-programs
      ShaderStageInterfaceList m_vInputInterfaces;
      /// List of outgoing inter-stage varyings.
      ShaderStageInterfaceList m_vOutputInterfaces;
      /// List of output-elements in the fragment shader stage
      ShaderStageFragmentOutputList m_vFragmentOutputs;

      /// Lists of resources defined in the shader (buffers, textures, ...)
      GpuResourceInfoList m_vReadTextureInfos;
      GpuResourceInfoList m_vReadBufferInfos;
      GpuResourceInfoList m_vWriteTextureInfos;
      GpuResourceInfoList m_vWriteBufferInfos;
      // TODO: Add lists for other opaque types (e.g. atomics)
    //---------------------------------------------------------------------------//
  };
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::GL4


#endif  // INCLUDE_GPUPROGRAMGL4_H