#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy 
{
  class CommandList;

  class CommandQueue
  {
  public:
    static CommandListType GetCommandListType(uint64 aFenceVal);

    explicit CommandQueue(CommandListType aType);
    virtual ~CommandQueue();

    virtual bool IsFenceDone(uint64 aFenceVal) = 0;
    virtual uint64 SignalAndIncrementFence() = 0;
    // Waits for a fence-completion on CPU timeline
    virtual void WaitForFence(uint64 aFenceVal) = 0;
    virtual void WaitForIdle() = 0;
    // Waits for a fence-completion on GPU timeline
    virtual void StallForQueue(const CommandQueue* aCommandQueue) = 0;
    virtual void StallForFence(uint64 aFenceVal) = 0;
    
    CommandList* BeginCommandList(uint someCommandListFlags);
    uint64 ExecuteAndFreeCommandList(CommandList* aContext, SyncMode aSyncMode = SyncMode::ASYNC);
    uint64 ExecuteAndResetCommandList(CommandList* aContext, SyncMode aSyncMode = SyncMode::ASYNC);

  protected:
    virtual uint64 ExecuteCommandListInternal(CommandList* aContext, SyncMode aSyncMode = SyncMode::ASYNC) = 0;
    virtual uint64 ExecuteAndResetCommandListInternal(CommandList* aContext, SyncMode aSyncMode = SyncMode::ASYNC) = 0;

    void ResolveResourceBarriers(CommandList* aCommandList);
    void FreeCommandList(CommandList* aCommandList);

    DynamicArray<UniquePtr<CommandList>> myCommandListPool;
    std::list<CommandList*> myAvailableCommandLists;

    CommandListType myType;
    uint64 myLastCompletedFenceVal;
    uint64 myNextFenceVal;
  };
}