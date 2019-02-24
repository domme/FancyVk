#pragma once

#include "FancyCoreDefines.h"
#include "Ptr.h"
#include "GpuQuery.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuQueryHeap;
  class GpuBuffer;
//---------------------------------------------------------------------------//
  struct GpuQueryStorage
  {
    void Create(GpuQueryType aQueryType, uint aNumQueries);
    
    void FreeAllQueries() { myNextFree = 0u; }
    uint GetNumFreeQueries() const;
    bool AllocateQueries(uint aNumQueries, uint& aStartQueryIdxOut);

    SharedPtr<GpuBuffer> myReadbackBuffer;
    SharedPtr<GpuQueryHeap> myQueryHeap;
    uint myNextFree = 0u;
  };
//---------------------------------------------------------------------------//
}
