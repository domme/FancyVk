#pragma once

namespace Fancy {
  class Window;
  struct InputState;
}

class Camera;

//---------------------------------------------------------------------------//
  class CameraController
  {
  public:
    CameraController(Fancy::Window* aWindow, Camera* aCamera);
    ~CameraController();

    void Update(float aDeltaTime, const Fancy::InputState& anInputState);

    float myMoveSpeed;

  private:
    Fancy::Window* myWindow;
    Camera* myCamera;
  };
//---------------------------------------------------------------------------//

