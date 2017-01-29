#include "SceneImporter.h"

#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <xxHash/xxhash.h>

#include "SceneNode.h"
#include "SceneNodeComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialPass.h"
#include "PathService.h"
#include "ModelComponent.h"
#include "Model.h"
#include "SubModel.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "GeometryVertexLayout.h"
#include "StringUtil.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "FileReader.h"
#include "GpuProgram.h"
#include "GpuProgramCompiler.h"

#define FANCY_IMPORTER_USE_VALIDATION
#include "MathUtil.h"
#include "BinaryCache.h"
#include "JSONwriter.h"
#include "JSONreader.h"
#include "GpuProgramFeatures.h"
#include "VertexInputLayout.h"
#include "GpuProgramPipeline.h"
#include "MaterialPassInstance.h"
#include "Log.h"
#include "Renderer.h"
#include "GraphicsWorld.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  using namespace Fancy::Geometry;
  using namespace Fancy::Rendering;
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
      C_LOG_INFO("SceneImporter: %s", message);
    }
//---------------------------------------------------------------------------//
    glm::mat4 matFromAiMat( const aiMatrix4x4& mat )
    {
      return glm::mat4(	mat.a1, mat.a2, mat.a3, mat.a4,
        mat.b1, mat.b2, mat.b3, mat.b4,
        mat.c1, mat.c2, mat.c3, mat.c4,
        mat.d1, mat.d2, mat.d3, mat.d4 );
    }
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    static FancyLog* m_pLogger = nullptr;
  //---------------------------------------------------------------------------//
  }
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
  SceneImporter::SceneImporter(GraphicsWorld& aGraphicsWorld)
    : myGraphicsWorld(aGraphicsWorld)
  {
  }
//---------------------------------------------------------------------------//
  SceneImporter::~SceneImporter()
  {

  }
//---------------------------------------------------------------------------//
  bool SceneImporter::importToSceneGraph( const std::string& _szImportPathRel, Scene::SceneNode* _pParentNode)
  {
    std::string szImportPathAbs = PathService::convertToAbsPath(_szImportPathRel);

    // TODO: Look for cached binary data and don't re-import if possible

    Assimp::Importer aImporter;

    const aiScene* aScene = aImporter.ReadFile(szImportPathAbs,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_FindInstances);

    if (!aScene)
      return false;

    myWorkingData = WorkingData();
    myWorkingData.szCurrScenePathInResources = _szImportPathRel;
    myWorkingData.pCurrScene = aScene;

    const aiNode* pArootNode = aScene->mRootNode;

    if (!pArootNode)
      return false;

    if (!processAiNode(pArootNode, _pParentNode))
      return false;

    // Serialization-tests....
    // Serialization needs a re-work after adjusting Resource-Handling apis
    {
      JSONwriter serializer(szImportPathAbs);
      serializer.Serialize(&_pParentNode, "rootNode");
    }

    {
      JSONreader serializer(szImportPathAbs, myGraphicsWorld);
      serializer.Serialize(&_pParentNode, "rootNode");
    }
    
    return true;
  }
//---------------------------------------------------------------------------//
  bool SceneImporter::processAiNode(const aiNode* _pAnode, Scene::SceneNode* _pParentNode)
  {
    bool success = true;

    Scene::SceneNode* pNode = _pParentNode->createChildNode();
    if (_pAnode->mName.length > 0u)
    {
      pNode->setName(ObjectName(_pAnode->mName.C_Str()));
    }

    glm::mat4 transformMat = Internal::matFromAiMat(_pAnode->mTransformation);
    glm::mat3 rotationScale(transformMat);
    glm::quat rotation = glm::toQuat(rotationScale);
    glm::vec3 scale(glm::length(rotationScale[0]), glm::length(rotationScale[1]), glm::length(rotationScale[2]));
    glm::vec3 pos(transformMat[3]);

    pNode->getTransform().setRotationLocal(rotation);
    pNode->getTransform().setScaleLocal(scale);
    pNode->getTransform().setPositionLocal(pos);

    if (_pAnode->mNumMeshes > 0u)
    {
      Scene::ModelComponent* pModelComponent = static_cast<Scene::ModelComponent*>(pNode->addOrRetrieveComponent(_N(ModelComponent)));
      success &= processMeshes(_pAnode, pModelComponent);
    }
    
    for (uint32 i = 0u; success && i < _pAnode->mNumChildren; ++i)
    {
      success &= processAiNode(_pAnode->mChildren[i], pNode);
    }

    return success;
  }
//---------------------------------------------------------------------------//
  bool SceneImporter::processMeshes(const aiNode* _pAnode, Scene::ModelComponent* _pModelComponent)
  {
    // Sort all meshes by their material. Each entry will become a submodel of the model
    
    typedef FixedArray<aiMesh*, kMaxNumAssimpMeshesPerNode> AssimpMeshList;

    typedef std::map<uint32, AssimpMeshList> MaterialMeshMap;
     MaterialMeshMap mapMaterialIndexMesh;

    const uint32 uNumMeshes = _pAnode->mNumMeshes;
    for (uint32 i = 0u; i < uNumMeshes; ++i)
    {
      const uint32 uMeshIndex = _pAnode->mMeshes[i];
      aiMesh* pAmesh = 
        myWorkingData.pCurrScene->mMeshes[uMeshIndex];

      const uint32 uMaterialIndex = pAmesh->mMaterialIndex;
      AssimpMeshList& vMeshesWithMaterial = mapMaterialIndexMesh[uMaterialIndex];
      
      if (!vMeshesWithMaterial.contains(pAmesh))
      {
        vMeshesWithMaterial.push_back(pAmesh);
      }
    }

    // Construct or retrieve Fancy Meshes and Submodels
    // Each mesh-list with the same material becomes a submodel
    std::vector<SharedPtr<SubModel>> vSubModels;
    for (MaterialMeshMap::iterator it = mapMaterialIndexMesh.begin(); it != mapMaterialIndexMesh.end(); ++it)
    {
      const uint32 uMaterialIndex = it->first;
      AssimpMeshList& vAssimpMeshes = it->second;
      
      SharedPtr<Geometry::Mesh> pMesh = 
        constructOrRetrieveMesh(_pAnode, &vAssimpMeshes[0], vAssimpMeshes.size());

      // Create or retrieve the material
      aiMaterial* pAmaterial = 
        myWorkingData.pCurrScene->mMaterials[uMaterialIndex];
      SharedPtr<Rendering::Material> pMaterial = createOrRetrieveMaterial(pAmaterial);

      // Do we already have a Submodel with this mesh and material?
      Geometry::SubModelDesc submodelDesc;
      submodelDesc.myMaterial = pMaterial->GetDescription();
      submodelDesc.myMesh = pMesh->GetDescription();

      SharedPtr<Geometry::SubModel> pSubModel = myGraphicsWorld.CreateSubModel(submodelDesc);
      if (vSubModels.end() == std::find(vSubModels.begin(), vSubModels.end(), pSubModel))
      {
        vSubModels.push_back(pSubModel);
      }
    }  // end iteration of materialMeshList-map

    // At this point, we constructed a bunch of submodels. Now construct them to 
    // a Model (or retrieve an equivalent one...)
    Geometry::ModelDesc modelDesc;
    modelDesc.mySubmodels.resize(vSubModels.size());
    for (uint i = 0u; i < vSubModels.size(); ++i)
      modelDesc.mySubmodels[i] = vSubModels[i]->GetDescription();

    SharedPtr<Geometry::Model> pModel = myGraphicsWorld.CreateModel(modelDesc);
    _pModelComponent->setModel(pModel);

    return true;
  }
//---------------------------------------------------------------------------//
  uint64 locComputeHashFromVertexData(aiMesh** someMeshes, uint32 aMeshCount)
  {
    XXH64_state_t* xxHashState = XXH64_createState();
    XXH64_reset(xxHashState, 0u);

    for (uint iMesh = 0u; iMesh < aMeshCount; ++iMesh)
    {
      aiMesh* mesh = someMeshes[iMesh];

      XXH64_update(xxHashState, mesh->mVertices, mesh->mNumVertices * sizeof(aiVector3D));

      if (mesh->HasNormals())
        XXH64_update(xxHashState, mesh->mNormals, mesh->mNumVertices * sizeof(aiVector3D));

      if (mesh->HasTangentsAndBitangents())
      {
        XXH64_update(xxHashState, mesh->mTangents, mesh->mNumVertices * sizeof(aiVector3D));
        XXH64_update(xxHashState, mesh->mBitangents, mesh->mNumVertices * sizeof(aiVector3D));
      }

      for (uint iChannel = 0u; iChannel < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++iChannel)
      {
        if (mesh->HasTextureCoords(iChannel))
          XXH64_update(xxHashState, mesh->mTextureCoords[iChannel], mesh->mNumVertices * sizeof(aiVector3D));
      }

      for (uint iChannel = 0u; iChannel < AI_MAX_NUMBER_OF_COLOR_SETS; ++iChannel)
      {
        if (mesh->HasVertexColors(iChannel))
          XXH64_update(xxHashState, mesh->mColors[iChannel], mesh->mNumVertices * sizeof(aiColor4D));
      }

      if (mesh->HasFaces())
      {
        for (uint iFace = 0u; iFace < mesh->mNumFaces; ++iFace)
        {
          aiFace& face = mesh->mFaces[iFace];
          XXH64_update(xxHashState, face.mIndices, face.mNumIndices * sizeof(unsigned int));
        }
      }
    }

    uint64 hash = XXH64_digest(xxHashState);

    XXH64_freeState(xxHashState);

    return hash;
  }
//---------------------------------------------------------------------------//
  const ShaderVertexInputElement* locGetShaderExpectedInput(VertexSemantics aSemantic)
  {
    const ShaderVertexInputLayout& modelLayout = ShaderVertexInputLayout::ourDefaultModelLayout;
    const ShaderVertexInputElementList& vertexElements = modelLayout.getVertexElementList();

    for (uint32 i = 0u; i < vertexElements.size(); ++i)
      if (vertexElements[i].mySemantics == aSemantic)
        return &vertexElements[i];

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Geometry::Mesh> SceneImporter::constructOrRetrieveMesh(const aiNode* _pANode, aiMesh** someMeshes, uint32 aMeshCount)
  {
    // TODO: Refactor this caching-mechanism:
    // We don't save any processing time if we read in the cached mesh, construct its hash and THEN check if we already have this mesh loaded in the engine
    // Instead, do the following:
    // 1) Modify the cache-system so we can "peek ahead" and only retrieve its vertexIndexHash
    // 2) Check if we already have a mesh with this hash, if so --> return this mesh
    // 3) If there isn't any matching Mesh in the engine yet --> load it from cache if the cache-timestamp is newer than the imported scene-file
    // 4) If the scene-file is newer: load the mesh from assimp and update cache file

    ObjectName meshName = GetCachePathForMesh();
    // if (BinaryCache::read(&outputMesh, meshName, 0u))
    // return outputMesh;

    // Did we construct a similar mesh before from the same ai-Meshes?
    auto findMeshInCache = [&]() -> SharedPtr<Geometry::Mesh> {
      for (uint32 i = 0u; i < myWorkingData.localMeshCache.size(); ++i)
      {
        const std::pair<AiMeshList, SharedPtr<Geometry::Mesh>>& entry = myWorkingData.localMeshCache[i];

        bool isValid = true;
        for (uint32 iAiMesh = 0u; isValid && iAiMesh < aMeshCount; ++iAiMesh)
          isValid &= entry.first.contains(someMeshes[iAiMesh]);

        if (isValid)
          return entry.second;
      }
      return nullptr;
    };
    
    SharedPtr<Geometry::Mesh> mesh;
    mesh = findMeshInCache();
    if (mesh != nullptr)
      return mesh;

    uint64 vertexIndexHash = locComputeHashFromVertexData(someMeshes, aMeshCount);
    
    mesh = Rendering::RenderCore::GetMesh(vertexIndexHash);
    if (mesh != nullptr)
      return mesh;
    
    // We don't have the mesh in any cache and have to create it.

    MeshDesc meshDesc;
    meshDesc.myVertexAndIndexHash = vertexIndexHash;
    
    std::vector<void*> vertexDatas;
    std::vector<uint> numVertices;
    std::vector<void*> indexDatas;
    std::vector<uint> numIndices;
    for (uint32 iAiMesh = 0; iAiMesh < aMeshCount; ++iAiMesh)
    {
      const aiMesh* aiMesh = someMeshes[iAiMesh];

      struct ImportVertexStream
      {
        void* mySourceData;
        uint32 mySourceDataStride;
        VertexSemantics mySourceSemantic;
        uint32 mySourceSemanticIndex;
      };
      FixedArray<ImportVertexStream, Rendering::kMaxNumGeometryVertexAttributes> importStreams;

      ASSERT(aiMesh->HasPositions());
      {
        ImportVertexStream stream;
        stream.mySourceDataStride = sizeof(aiMesh->mVertices[0]);
        stream.mySourceData = aiMesh->mVertices;
        stream.mySourceSemantic = VertexSemantics::POSITION;
        stream.mySourceSemanticIndex = 0u;
        importStreams.push_back(stream);
      }

      if (aiMesh->HasNormals())
      {
        ImportVertexStream stream;
        stream.mySourceDataStride = sizeof(aiMesh->mNormals[0]);
        stream.mySourceData = aiMesh->mNormals;
        stream.mySourceSemantic = VertexSemantics::NORMAL;
        stream.mySourceSemanticIndex = 0u;
        importStreams.push_back(stream);
      }

      if (aiMesh->HasTangentsAndBitangents())
      {
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mTangents[0]);
          stream.mySourceData = aiMesh->mTangents;
          stream.mySourceSemantic = VertexSemantics::TANGENT;
          stream.mySourceSemanticIndex = 0u;
          importStreams.push_back(stream);
        }
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mBitangents[0]);
          stream.mySourceData = aiMesh->mBitangents;
          stream.mySourceSemantic = VertexSemantics::BITANGENT;
          stream.mySourceSemanticIndex = 0u;
          importStreams.push_back(stream);
        }
      }

      for (uint32 iUVchannel = 0u; iUVchannel < aiMesh->GetNumUVChannels(); ++iUVchannel)
      {
        if (aiMesh->HasTextureCoords(iUVchannel))
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mTextureCoords[iUVchannel][0]);
          stream.mySourceData = aiMesh->mTextureCoords[iUVchannel];
          stream.mySourceSemantic = VertexSemantics::TEXCOORD;
          stream.mySourceSemanticIndex = iUVchannel;
          importStreams.push_back(stream);
        }
      }

      for (uint32 iColorChannel = 0u; iColorChannel < aiMesh->GetNumColorChannels(); ++iColorChannel)
      {
        if (aiMesh->HasVertexColors(iColorChannel))
        {
          ImportVertexStream stream;
          stream.mySourceDataStride = sizeof(aiMesh->mColors[iColorChannel][0]);
          stream.mySourceData = aiMesh->mColors[iColorChannel];
          stream.mySourceSemantic = VertexSemantics::COLOR;
          stream.mySourceSemanticIndex = iColorChannel;
          importStreams.push_back(stream);
        }
      }

      const ShaderVertexInputLayout* expectedLayout = &ShaderVertexInputLayout::ourDefaultModelLayout;
      const ShaderVertexInputElementList& expectedInputList = expectedLayout->getVertexElementList();

      // Check if we need additional patching-streams when the model-shaders expect more data than this model has

      FixedArray<ImportVertexStream, Rendering::kMaxNumGeometryVertexAttributes> actualImportStreams;
      actualImportStreams.resize(expectedInputList.size());

      FixedArray<void*, Rendering::kMaxNumGeometryVertexAttributes> patchingDatas;

      for (uint32 i = 0u; i < expectedInputList.size(); ++i)
      {
        const ShaderVertexInputElement& expectedElem = expectedInputList[i];

        bool foundInImportStreams = false;
        for (uint32 k = 0u; k < importStreams.size(); ++k)
        {
          ImportVertexStream& stream = importStreams[k];

          if (expectedElem.mySemantics != stream.mySourceSemantic)
            continue;

          foundInImportStreams = true;
          
          // Stride-mismatch? Then we have to patch! 
          // Either we have more data we need or not enough. Below accounts for both cases
          if (stream.mySourceDataStride != expectedElem.mySizeBytes) 
          {
            void* patchedData = malloc(expectedElem.mySizeBytes * aiMesh->mNumVertices);
            patchingDatas.push_back(patchedData);

            for (uint32 iVertex = 0u; iVertex < aiMesh->mNumVertices; ++iVertex)
            {
              uint8* dest = ((uint8*)patchedData) + iVertex * expectedElem.mySizeBytes;
              uint8* src = ((uint8*)stream.mySourceData) + iVertex * stream.mySourceDataStride;

              memcpy(dest, src, glm::min(expectedElem.mySizeBytes, stream.mySourceDataStride));

              if (expectedElem.mySizeBytes > stream.mySourceDataStride)
                memset(dest + stream.mySourceDataStride, 0, (expectedElem.mySizeBytes - stream.mySourceDataStride));
            }

            stream.mySourceDataStride = expectedElem.mySizeBytes;
            stream.mySourceData = patchedData;
          }

          actualImportStreams[i] = stream;
        }

        // Is this semantic missing entirely? 
        if (!foundInImportStreams)
        {
          void* patchedData = malloc(expectedElem.mySizeBytes * aiMesh->mNumVertices);
          patchingDatas.push_back(patchedData);
          memset(patchedData, 0, expectedElem.mySizeBytes * aiMesh->mNumVertices);

          actualImportStreams[i].mySourceDataStride = expectedElem.mySizeBytes;
          actualImportStreams[i].mySourceData = patchedData;
          actualImportStreams[i].mySourceSemantic = expectedElem.mySemantics;
          actualImportStreams[i].mySourceSemanticIndex = expectedElem.mySemanticIndex;
        }
      }
            
      // Construct the vertex layout description.
      // After doing the patching-work above, this can be set up to exactly match the input layout expected by the shaders
      Rendering::GeometryVertexLayout vertexLayout;

      uint32 offset = 0u;
      for (uint32 i = 0u; i < expectedInputList.size(); ++i)
      {
        const ShaderVertexInputElement& expectedInput = expectedInputList[i];

        GeometryVertexElement vertexElem;
        vertexElem.u32OffsetBytes = offset;
        vertexElem.eFormat = expectedInput.myFormat;
        vertexElem.mySemanticIndex = expectedInput.mySemanticIndex;
        vertexElem.eSemantics = expectedInput.mySemantics;
        vertexElem.name = expectedInput.myName;
        vertexElem.u32SizeBytes = expectedInput.mySizeBytes;

        offset += vertexElem.u32SizeBytes;
        vertexLayout.addVertexElement(vertexElem);
      }

      const uint uSizeVertexBufferBytes = vertexLayout.getStrideBytes() * aiMesh->mNumVertices;
      void* pData = FANCY_ALLOCATE(uSizeVertexBufferBytes, MemoryCategory::GEOMETRY);

      if (!pData)
      {
        LOG_ERROR("Failed to allocate vertex buffer");
        return nullptr;
      }

      // Construct an interleaved vertex array
      for (uint iVertex = 0u; iVertex < aiMesh->mNumVertices; ++iVertex)
      {
        for (uint iVertexElement = 0u; iVertexElement < vertexLayout.getNumVertexElements(); ++iVertexElement)
        {
          const Rendering::GeometryVertexElement& vertexElem = vertexLayout.getVertexElement(iVertexElement);
          uint destInterleavedOffset = iVertex * vertexLayout.getStrideBytes() + vertexElem.u32OffsetBytes;
          uint srcOffset = iVertex * actualImportStreams[iVertexElement].mySourceDataStride;

          uint8* dest = ((uint8*)pData) + destInterleavedOffset;
          uint8* src = ((uint8*)actualImportStreams[iVertexElement].mySourceData) + srcOffset;

          memcpy(dest, src, vertexElem.u32SizeBytes);
        }
      }

      vertexDatas.push_back(pData);
      numVertices.push_back(aiMesh->mNumVertices);
      meshDesc.myVertexLayouts.push_back(vertexLayout);

      for (uint32 i = 0u; i < patchingDatas.size(); ++i)
        free(patchingDatas[i]);
      patchingDatas.clear();
      
      /// Construct the index buffer
#if defined (FANCY_IMPORTER_USE_VALIDATION)
      // Ensure that we have only triangles
      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        ASSERT(aiMesh->mFaces[i].mNumIndices == 3u, "Unsupported face type");
      }
#endif  // FANCY_IMPORTER_USE_VALIDATION

      uint32* indices = FANCY_NEW(uint32[aiMesh->mNumFaces * 3u], MemoryCategory::GEOMETRY);

      for (uint i = 0u; i < aiMesh->mNumFaces; ++i)
      {
        const aiFace& aFace = aiMesh->mFaces[i];

        ASSERT(sizeof(indices[0]) == sizeof(aFace.mIndices[0]));
        memcpy(&indices[i * 3u], aFace.mIndices, sizeof(uint32) * 3u);
      }

      // indexBufParams.uNumElements = aiMesh->mNumFaces * 3u;
      indexDatas.push_back(indices);
      numIndices.push_back(aiMesh->mNumFaces * 3u);
    }

    mesh = Rendering::RenderCore::CreateMesh(meshDesc, vertexDatas, indexDatas, numVertices, numIndices);
    ASSERT(mesh != nullptr);

    AiMeshList aiMeshList;
    aiMeshList.resize(aMeshCount);
    memcpy(&aiMeshList[0], &someMeshes[0], sizeof(aiMesh*) * aMeshCount);
    myWorkingData.localMeshCache.push_back(std::pair<AiMeshList, SharedPtr<Geometry::Mesh>>(aiMeshList, mesh));

    for (uint32 i = 0u; i < vertexDatas.size(); ++i)
    {
      FANCY_FREE(vertexDatas[i], MemoryCategory::GEOMETRY);
    }
    vertexDatas.clear();

    for (uint32 i = 0u; i < indexDatas.size(); ++i)
    {
      FANCY_DELETE_ARR(indexDatas[i], MemoryCategory::GEOMETRY);
    }
    indexDatas.clear();
    
    return mesh;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Rendering::Material> SceneImporter::createOrRetrieveMaterial(const aiMaterial* _pAmaterial)
  {
    // Did we already import this material?
    MaterialCacheMap::iterator cacheIt = myWorkingData.mapAiMatToMat.find(_pAmaterial);
    if (cacheIt != myWorkingData.mapAiMatToMat.end())
    {
      return cacheIt->second;
    }
        
    // Retrieve the material properties most relevant for us
    aiString szAname;
    bool hasName = _pAmaterial->Get(AI_MATKEY_NAME, szAname) == AI_SUCCESS;

    aiColor3D color_diffuse;
    bool hasColor = _pAmaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color_diffuse) == AI_SUCCESS;

    aiColor3D color_specular;
    bool hasSpecularColor = _pAmaterial->Get(AI_MATKEY_COLOR_SPECULAR, color_specular) == AI_SUCCESS;

    aiColor3D color_ambient;
    bool hasAmbientColor = _pAmaterial->Get(AI_MATKEY_COLOR_AMBIENT, color_ambient) == AI_SUCCESS;

    aiColor3D color_emissive;
    bool hasEmissiveColor = _pAmaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color_emissive) == AI_SUCCESS;

    aiColor3D color_transparent;
    bool hasTransparentColor = _pAmaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, color_transparent) == AI_SUCCESS;

    aiBlendMode blend_func;
    bool hasBlendFunc = _pAmaterial->Get(AI_MATKEY_BLEND_FUNC, blend_func) == AI_SUCCESS;

    float opacity;
    bool hasOpacity = _pAmaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS;

    float specularPower;
    bool hasSpecularPower = _pAmaterial->Get(AI_MATKEY_SHININESS, specularPower) == AI_SUCCESS;

    float specular;
    bool hasSpecular = _pAmaterial->Get(AI_MATKEY_SHININESS_STRENGTH, specular) == AI_SUCCESS;

    SharedPtr<Texture> pDiffuseTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_DIFFUSE, 0u);
    SharedPtr<Texture> pNormalTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_NORMALS, 0u);
    SharedPtr<Texture> pSpecularTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_SPECULAR, 0u);
    SharedPtr<Texture> pSpecPowerTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_SHININESS, 0u);
    SharedPtr<Texture> pOpacityTex = createOrRetrieveTexture(_pAmaterial, aiTextureType_OPACITY, 0u);

    bool hasDiffuseTex = pDiffuseTex != nullptr;
    bool hasNormalTex = pDiffuseTex != nullptr;
    bool hasSpecTex = pSpecularTex != nullptr;
    bool hasSpecPowerTex = pSpecPowerTex != nullptr;
    bool hasOpacityTex = pOpacityTex != nullptr;

    // Fancy only supports textures where opacity is combined with diffuse and specPower is combined with specular
    if (hasSpecTex && hasSpecPowerTex && pSpecPowerTex != pSpecularTex)
    {
      LOG_WARNING("Fancy doesn't support storing specular power in a separate texture. Consider putting it in spec.a");
    }

    if (hasDiffuseTex && hasOpacityTex && pOpacityTex != pDiffuseTex)
    {
      LOG_WARNING("Fancy doesn't support storing opacity in a separate texture. Consider putting it in diff.a");
    }

    // Find/Create matching Shaders
    //---------------------------------------------------------------------------//
    GpuProgramPermutation permutation;
    if (hasDiffuseTex) 
      permutation.addFeature(GpuProgramFeature::FEAT_ALBEDO_TEXTURE);
    if (hasNormalTex) 
      permutation.addFeature(GpuProgramFeature::FEAT_NORMAL_MAPPED);
    if (hasSpecTex)
    {
      permutation.addFeature(GpuProgramFeature::FEAT_SPECULAR);
      permutation.addFeature(GpuProgramFeature::FEAT_SPECULAR_TEXTURE);
    }

    GpuProgramPipelineDesc pipelineDesc;
    GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::VERTEX];
    shaderDesc->myPermutation = permutation;
    shaderDesc->myShaderFileName = "MaterialForward";
    shaderDesc->myShaderStage = static_cast<uint32>(ShaderStage::VERTEX);
    shaderDesc = &pipelineDesc.myGpuPrograms[(uint32)ShaderStage::FRAGMENT];
    shaderDesc->myPermutation = permutation;
    shaderDesc->myShaderFileName = "MaterialForward";
    shaderDesc->myShaderStage = static_cast<uint32>(ShaderStage::FRAGMENT);
    SharedPtr<GpuProgramPipeline> pipeline = RenderCore::CreateGpuProgramPipeline(pipelineDesc);
    
    // Find/Create a matching MaterialPass
    //---------------------------------------------------------------------------//
    MaterialPassDesc matPassDesc;
    matPassDesc.m_BlendStateDesc = BlendStateDesc::GetDefaultSolid();
    matPassDesc.m_DepthStencilStateDesc = DepthStencilStateDesc::GetDefaultDepthNoStencil();
    matPassDesc.m_GpuProgramPipelineDesc = pipelineDesc;
    matPassDesc.m_eCullMode = static_cast<uint32>(CullMode::BACK);
    matPassDesc.m_eFillMode = static_cast<uint32>(FillMode::SOLID);
    matPassDesc.m_eWindingOrder = static_cast<uint32>(WindingOrder::CCW);
    
    MaterialPass* pMaterialPass = MaterialPass::FindFromDesc(matPassDesc);
    if (pMaterialPass == nullptr)
    {
      pMaterialPass = FANCY_NEW(MaterialPass, MemoryCategory::MATERIALS);
      pMaterialPass->SetFromDescription(matPassDesc);
      MaterialPass::Register(pMaterialPass);
    }
    //---------------------------------------------------------------------------//


    // Find/Create a matching MaterialPassInstance
    //---------------------------------------------------------------------------//
    MaterialPassInstanceDesc mpiDesc;
    mpiDesc.myMaterialPass = matPassDesc;
    mpiDesc.myReadTextures[0u] = RenderCore::GetDefaultDiffuseTexture()->GetDescription();
    mpiDesc.myReadTextures[1u] = RenderCore::GetDefaultNormalTexture()->GetDescription();
    mpiDesc.myReadTextures[2u] = RenderCore::GetDefaultSpecularTexture()->GetDescription();

    if (nullptr != pDiffuseTex)
      mpiDesc.myReadTextures[0u] = pDiffuseTex->GetDescription();
    if (nullptr != pNormalTex)
      mpiDesc.myReadTextures[1u] = pNormalTex->GetDescription();
    if (nullptr != pSpecularTex)
      mpiDesc.myReadTextures[2u] = pSpecularTex->GetDescription();
    SharedPtr<MaterialPassInstance> pSolidForwardMpi = myGraphicsWorld.CreateMaterialPassInstance(mpiDesc);
    //---------------------------------------------------------------------------//

    // Find/Create a matching Material
    //---------------------------------------------------------------------------//
    MaterialDesc matDesc;
    matDesc.myPasses[(uint32)EMaterialPass::SOLID_FORWARD] = mpiDesc;
    
    if (hasColor)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::DIFFUSE_REFLECTIVITY] = (color_diffuse.r + color_diffuse.g + color_diffuse.b) * (1.0f / 3.0f);
    if (hasSpecular)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::SPECULAR_REFLECTIVITY] = specular;
    if (hasOpacity)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::OPACITY] = opacity;
    if (hasSpecularPower)
      matDesc.myParameters[(uint32)EMaterialParameterSemantic::SPECULAR_POWER] = specularPower;
    
    SharedPtr<Material> pMaterial = myGraphicsWorld.CreateMaterial(matDesc);
    
    //---------------------------------------------------------------------------//

    return pMaterial;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Rendering::Texture> SceneImporter::createOrRetrieveTexture(
    const aiMaterial* _pAmaterial, uint32 _aiTextureType, uint32 _texIndex)
  {
    uint32 numTextures = _pAmaterial->GetTextureCount(static_cast<aiTextureType>(_aiTextureType));
    if (numTextures == 0u)
    {
      return nullptr;
    }

    ASSERT(numTextures > _texIndex);

    aiString szATexPath;
    _pAmaterial->Get(AI_MATKEY_TEXTURE(_aiTextureType, _texIndex), szATexPath);

    String szTexPath = String(szATexPath.C_Str());
    
    if (!PathService::isAbsolutePath(szTexPath))
    {
      String absSceneFolderPath = myWorkingData.szCurrScenePathInResources;
      PathService::convertToAbsPath(absSceneFolderPath);
      PathService::removeFilenameFromPath(absSceneFolderPath);
      szTexPath = absSceneFolderPath + szTexPath;
    }

    PathService::removeFolderUpMarkers(szTexPath);
    String texPathInResources = PathService::toRelPath(szTexPath);

    return RenderCore::CreateTexture(texPathInResources);
  }
//---------------------------------------------------------------------------//
  std::string SceneImporter::GetCachePathForMesh()
  {
    return "Mesh_" + myWorkingData.szCurrScenePathInResources + "_" 
      + StringUtil::toString(myWorkingData.u32NumCreatedMeshes++);
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO