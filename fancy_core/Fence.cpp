#include "StdAfx.h"

#include "Fence.h"
#include "RenderCore.h"

#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Fence::Fence(CommandListType aCommandListType)
    : myLastCompletedVal(0)
    , myNextVal(1)
    , myCommandListType(aCommandListType)
    , myEventHandle(nullptr)
  {
    myEventHandle = CreateEventEx(nullptr, nullptr, 0u, EVENT_ALL_ACCESS);
    ASSERT(myEventHandle != nullptr);
  }
//---------------------------------------------------------------------------//
  void Fence::Wait()
  {
    if (IsDone(myCurrWaitingOnVal))
      return;

    myGpuFence->SetEventOnCompletion(myCurrWaitingOnVal, myIsDoneEvent);
    WaitForSingleObject(myIsDoneEvent, INFINITE);
    myLastCompletedVal = myCurrWaitingOnVal;
  }
  //---------------------------------------------------------------------------//
  bool Fence::IsDone(uint64 anOtherFenceVal)
  {
    // The fast path: the other fence-value is passed if the last completed val is greater/equal
    if (anOtherFenceVal <= myLastCompletedVal)
      return true;

    // Otherwise, we can't be sure and need to fetch the fence's value (potentially expensive..)
    myLastCompletedVal = glm::max(myLastCompletedVal, myGpuFence->GetCompletedValue());

    return anOtherFenceVal <= myLastCompletedVal;
  }
  //---------------------------------------------------------------------------//
  Fence::~Fence()
  {

  }

  bool Fence::IsDone()
  {
    return 
  }

  void Fence::SignalOnCpu(uint64 aFenceVal)
  {

  }

  void Fence::SignalOnGpu(uint64 aFenceVal)
  {
    myQueue->Signal(myGpuFence.Get(), )
  }

  // Wait on CPU-time until the GPU has passed the fence
  void Fence::WaitWithCpu(uint64 aFenceVal)
  {
    if (IsDone(myCurrWaitingOnVal))
      return;

    myGpuFence->SetEventOnCompletion(myCurrWaitingOnVal, myIsDoneEvent);
    WaitForSingleObject(myIsDoneEvent, INFINITE);
    myLastCompletedVal = myCurrWaitingOnVal;
  }

  // Wait on GPU-time until the cpu has passed the fence
  void Fence::WaitWithGpu(uint64 aFenceVal)
  {
    myQueue->Wait(myGpuFence.Get(), myCurrWaitingOnVal);
  }

  //---------------------------------------------------------------------------//
}
