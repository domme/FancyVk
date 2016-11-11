#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgram.h"
#include "FileWatcher.h"

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering {
  class GpuProgramPermutation;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerDX12 : public FileWatcher
  {
  public:
      GpuProgramCompilerDX12();
      ~GpuProgramCompilerDX12();

      bool Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutputDX12* aProgram) const;
      
      // TODO: Find a nicer place for platform-dependent infos
      static String GetPlatformShaderFileExtension() { return ".hlsl"; }
      static String GetPlatformShaderFileDirectory() { return "shader/DX12/"; }

      String ResolvePlatformShaderPath(const String& aPath) const;
  };
//---------------------------------------------------------------------------//
} } }

#endif 

