#ifndef INCLUDE_SCENEIMPORTER_H
#define INCLUDE_SCENEIMPORTER_H

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Scene {
  class Scene;
  class SceneNode;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class DLLEXPORT SceneImporter 
  {
    public: 
      static void initLogger();
      static void destroyLogger();
      static bool importToSceneGraph(const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode);
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

#endif  // INCLUDE_SCENEIMPORTER_H