#pragma once
#include "FancyCorePrerequisites.h"
#include "GpuProgramFeatures.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc
  {
    GpuProgramDesc() : myShaderFileName(""), myShaderStage(0u), myMainFunction("main") {}
    bool operator==(const GpuProgramDesc& anOther) const;
    uint64 GetHash() const;

    String myShaderFileName;
    String myMainFunction;
    uint myShaderStage;
    GpuProgramPermutation myPermutation;
  };
//---------------------------------------------------------------------------//
  inline bool GpuProgramDesc::operator==(const GpuProgramDesc& anOther) const
  {
    return myShaderFileName == anOther.myShaderFileName
      && myShaderStage == anOther.myShaderStage
      && myPermutation == anOther.myPermutation
      && myMainFunction == anOther.myMainFunction;
  }
//---------------------------------------------------------------------------//
  inline uint64 GpuProgramDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, MathUtil::hashFromString(myShaderFileName));
    MathUtil::hash_combine(hash, myShaderStage);
    MathUtil::hash_combine(hash, myPermutation.GetHash());
    MathUtil::hash_combine(hash, MathUtil::hashFromString(myMainFunction));
    return hash;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//  
  struct GpuProgramPipelineDesc
  {
    bool operator==(const GpuProgramPipelineDesc& anOther) const;
    uint64 GetHash() const;
    
    GpuProgramDesc myGpuPrograms[(uint32)ShaderStage::NUM];
  };
//---------------------------------------------------------------------------//
  inline bool GpuProgramPipelineDesc::operator==(const GpuProgramPipelineDesc& anOther) const
  {
    bool equal = true;
    for (uint32 i = 0u; equal && i < (uint32)ShaderStage::NUM; ++i)
      equal &= myGpuPrograms[i] == anOther.myGpuPrograms[i];
    return equal;
  }
//---------------------------------------------------------------------------//
  inline uint64 GpuProgramPipelineDesc::GetHash() const
  {
    uint64 hash = 0u;

    for (uint32 i = 0u; i < (uint32)ShaderStage::NUM; ++i)
      MathUtil::hash_combine(hash, myGpuPrograms[i].GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
} }

