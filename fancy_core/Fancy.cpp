#include "fancy_core_precompile.h"

#include "FancyCoreDefines.h"
#include "Fancy.h"

#include "RenderCore.h"

#include "PathService.h"
#include "TimeManager.h"
#include "Log.h"
#include "CommandQueue.h"
#include "RenderOutput.h"
#include "Profiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::ourInstance = nullptr;
//---------------------------------------------------------------------------//
  FancyRuntime::FancyRuntime(HINSTANCE anAppInstanceHandle)
    : myAppInstanceHandle(anAppInstanceHandle)
  {
    
  }
//---------------------------------------------------------------------------//
  FancyRuntime::~FancyRuntime()
  {
    RenderCore::Shutdown();
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::Init(HINSTANCE anAppInstanceHandle, const RenderingStartupParameters& someParams, const WindowParameters& someWindowParams)
  {
    ASSERT(ourInstance == nullptr);
    if (ourInstance != nullptr)
      return ourInstance;

    // Init IO-subsystem
    Resources::InitResourceFolders();

    ourInstance = new FancyRuntime(anAppInstanceHandle);

    // Init rendering subsystem
    if (!RenderCore::IsInitialized())
      RenderCore::Init(someParams.myRenderingApi);

    ASSERT(RenderCore::IsInitialized());

    // Create the output
    ourInstance->myRenderOutput = RenderCore::CreateRenderOutput(anAppInstanceHandle, someWindowParams);

    Profiler::Init();
    
    return ourInstance;
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Shutdown()
  {
    RenderCore::GetCommandQueue(CommandListType::Graphics)->WaitForIdle();
    ourInstance->myRenderOutput.reset();

    ASSERT(ourInstance != nullptr);
    SAFE_DELETE(ourInstance);
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::GetInstance()
  {
    return ourInstance;
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::BeginFrame()
  {
    RenderCore::BeginFrame();
    Profiler::BeginFrame();
    myRenderOutput->BeginFrame();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Update(double _dt)
  {
    myRealTimeClock.Update(static_cast<float>(_dt));
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::EndFrame()
  {
    Profiler::EndFrame();
    myRenderOutput->EndFrame();
    RenderCore::EndFrame();
    ++Time::ourFrameIdx;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy