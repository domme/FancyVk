#ifndef INCLUDE_GPUPROGRAMCOMPILERGL4_H
#define INCLUDE_GPUPROGRAMCOMPILERGL4_H

#include <list>

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "FixedArray.h"
#include "ObjectName.h"
#include "GpuProgramResource.h"
#include "VertexInputLayout.h"
#include "GPUProgramGL4.h"
#include "AdapterGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerGL4
  {
  public:
    static bool compile(const String& _shaderPath, ShaderStage _eShaderStage, GpuProgramGL4& _rGpuProgram);

  protected:
    static bool reflectProgram(GpuProgramGL4& rGpuProgram);
    static void reflectConstants(GLuint uProgram);
    static void reflectResources( GLuint uProgram, 
      GpuResourceInfoList& rReadTextureInfos, GpuResourceInfoList& rReadBufferInfos, 
      GpuResourceInfoList& rWriteTextureInfos, GpuResourceInfoList& rWriteBufferInfos);
    static void reflectVertexInputs(GLuint uProgram, VertexInputLayout& rVertexLayout);
    static void reflectFragmentOutputs(GLuint uProgram, ShaderStageFragmentOutputList& vFragmentOutputs);
    static void reflectStageInputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList);
    static void reflectStageOutputs(GLuint uProgram, ShaderStageInterfaceList& rInterfaceList);
    static bool compileFromSource(const String& szSource, const ShaderStage& eShaderStage, GpuProgramGL4& rGpuProgram);
    static void preprocessShaderSource(std::list<String>& sourceLines, const ShaderStage& eShaderStage);

  private:
    GpuProgramCompilerGL4();
    ~GpuProgramCompilerGL4();
    
    static const uint32 kMaxNumLogChars = 10000u;
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering:GL4

#endif  // INCLUDE_GPUPROGRAMPIPELINEGL4_H