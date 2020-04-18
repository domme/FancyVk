#pragma once

#include "Test.h"
#include "fancy_core/Ptr.h"
#include "fancy_assets/ModelLoader.h"
#include "fancy_core/Camera.h"
#include "fancy_core/CameraController.h"

namespace Fancy
{
  class CommandList;
  class AssetManager;
  class ShaderPipeline;
  class TextureSampler;
}

class Test_ModelViewer : public Test
{
public:
  Test_ModelViewer(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_ModelViewer() override;
  void OnWindowResized(uint aWidth, uint aHeight) override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  void RenderGrid(Fancy::CommandList* ctx);
  void RenderScene(Fancy::CommandList* ctx);

  Fancy::SharedPtr<Fancy::AssetManager> myAssetManager;
  Fancy::ModelLoader::Scene myScene;

  Fancy::Camera myCamera;
  Fancy::CameraController myCameraController;

  Fancy::SharedPtr<Fancy::ShaderPipeline> myUnlitTexturedShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myUnlitVertexColorShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myDebugGeoShader;
  Fancy::SharedPtr<Fancy::TextureSampler> mySampler;
};

