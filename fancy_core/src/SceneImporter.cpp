#include "SceneImporter.h"

#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Scene.h"
#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialPass.h"
#include "PathService.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace Internal
  {
  //---------------------------------------------------------------------------//
    class FancyLog : public Assimp::LogStream
    {
      void write(const char* message) override;
    };
    //---------------------------------------------------------------------------//
    void FancyLog::write(const char* message)
    {
      log_Info(message);
    }
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    static FancyLog* m_pLogger = nullptr;
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  void SceneImporter::initLogger()
  {
    const unsigned int severity = 
        Assimp::Logger::Debugging
        |Assimp::Logger::Info
        |Assimp::Logger::Err
        |Assimp::Logger::Warn;

    Internal::m_pLogger = new Internal::FancyLog();

    Assimp::DefaultLogger::get()->attachStream(Internal::m_pLogger, severity);
  }
//---------------------------------------------------------------------------//
  void SceneImporter::destroyLogger()
  {
    Assimp::DefaultLogger::get()->detatchStream(Internal::m_pLogger);
  }
//---------------------------------------------------------------------------//
  bool SceneImporter::importToSceneGraph( const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode )
  {
    bool success = false;
    std::string szImportPathAbs = PathService::convertToAbsPath(_szImportPathRel);

    Assimp::Importer aImporter;
    
    const aiScene* aScene = aImporter.ReadFile(szImportPathAbs,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (!aScene)
    {
      return false;
    }

    return success;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO