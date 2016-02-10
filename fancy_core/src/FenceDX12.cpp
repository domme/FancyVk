#include "StdAfx.h"

#if defined (RENDERER_DX12)

#include "FenceDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {

  FenceDX12::FenceDX12() :
      myFence(nullptr)
    , myIsDoneEvent(nullptr)
    , myCurrWaitingOnVal(0u)
  {
    
  }

  void FenceDX12::init(ID3D12Device* aDevice, const String& aName)
  {
    HRESULT success = aDevice->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence));
    ASSERT(success == S_OK);

    myIsDoneEvent = CreateEventEx(nullptr, nullptr, 0u, EVENT_ALL_ACCESS);
    ASSERT(myIsDoneEvent != nullptr);
  }

uint64 FenceDX12::signal(ID3D12CommandQueue* aCommandQueue)
{
  ++myCurrWaitingOnVal;
  aCommandQueue->Signal(myFence.Get(), myCurrWaitingOnVal);
  return myCurrWaitingOnVal;
}

void FenceDX12::wait()
{
  if (!IsDone(myCurrWaitingOnVal))
  {
    myFence->SetEventOnCompletion(myCurrWaitingOnVal, myIsDoneEvent);
    WaitForSingleObject(myIsDoneEvent, INFINITE);
    myLastCompletedVal = myCurrWaitingOnVal;
  }
}

bool FenceDX12::IsDone(uint64 anOtherFenceVal)
{
  // The fast path: the other fence-value is passed if the last completed val is greater/equal
  if (anOtherFenceVal <= myLastCompletedVal)
    return true;

  // Otherwise, we can't be sure and need to fetch the fence's value (potentially expensive..)
  myLastCompletedVal = glm::max(myLastCompletedVal, myFence->GetCompletedValue());

  return anOtherFenceVal <= myLastCompletedVal;
}

FenceDX12::~FenceDX12()
{
  
}

} } }

#endif
