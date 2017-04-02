#include "Fancy.h"

#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "RenderCore.h"
#include "SceneNodeComponentFactory.h"

#include "ModelComponent.h"
#include "CameraComponent.h"
#include "CameraControllerComponent.h"
#include "PathService.h"
#include "SceneImporter.h"
#include "TimeManager.h"
#include "RenderingProcess.h"
#include "LightComponent.h"
#include "ScopedPtr.h"
#include "RenderingProcessForward.h"
#include "RenderView.h"
#include "GraphicsWorld.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  DLLEXPORT FancyRuntime* FancyRuntime::ourInstance = nullptr;
//---------------------------------------------------------------------------//
  FancyRuntime::FancyRuntime(HINSTANCE anAppInstanceHandle)
    : myAppInstanceHandle(anAppInstanceHandle)
    , myFrameIndex(0u)
  {
    
  }
//---------------------------------------------------------------------------//
  FancyRuntime::~FancyRuntime()
  {
    IO::SceneImporter::destroyLogger();

    Rendering::RenderCore::Shutdown();
    Rendering::RenderCore::ShutdownPlatform();
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Internal_Init(const EngineParameters& someParams)
  {
    myMainWorld = std::make_shared<GraphicsWorld>();
    myMainView = new RenderView(myAppInstanceHandle, static_cast<uint32>(someParams.myRenderingTechnique), myMainWorld);
    myViews.push_back(myMainView.Get());
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::DoFirstFrameTasks()
  {
    for (RenderView* view : myViews)
      view->Startup();
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::Init(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams)
  {
    ASSERT(ourInstance == nullptr);
    if (ourInstance != nullptr)
      return ourInstance;

    // Init IO-subsystem
    IO::PathService::SetResourceLocation(someParams.myResourceFolder);
    IO::SceneImporter::initLogger();

    // Init Component subsystem
    Scene::SceneNodeComponentFactory::registerFactory(_N(ModelComponent), Scene::ModelComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(CameraComponent), Scene::CameraComponent::create);
    Scene::SceneNodeComponentFactory::registerFactory(_N(LightComponent), Scene::LightComponent::create);
    // Scene::SceneNodeComponentFactory::registerFactory(_N(CameraControllerComponent), Scene::CameraControllerComponent::create);

    ourInstance = new FancyRuntime(anAppInstanceHandle);

    // Init rendering subsystem
    if (!Rendering::RenderCore::IsInitialized())
    {
      Rendering::RenderCore::InitPlatform();
      Rendering::RenderCore::Init();
      Rendering::RenderCore::PostInit();
    }

    ourInstance->Internal_Init(someParams);

    return ourInstance;
  }
//---------------------------------------------------------------------------//
  FancyRuntime* FancyRuntime::GetInstance()
  {
    return ourInstance;
  }
//---------------------------------------------------------------------------//
  void FancyRuntime::Update(double _dt)
  {
    if (myRealTimeClock.GetElapsed() == 0.0f)
      DoFirstFrameTasks();

    myRealTimeClock.Update(_dt);
   
    for (RenderView* view : myViews)
      view->Tick(myRealTimeClock);

    ++myFrameIndex;
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy