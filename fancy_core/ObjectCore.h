#pragma once

#include "MeshImporter.h"

#include "EASTL/hash_map.h"

namespace Fancy
{
  class ShaderPipeline;
  struct Material;
  struct Mesh;
  struct MaterialDesc;
  struct MeshPartData;
  struct MeshDesc;

  class ObjectCore
  {
  public:
    enum TextureLoadFlags
    {
      NO_DISK_CACHE = 1 << 0,
      NO_MEM_CACHE = 1 << 1,
      SHADER_WRITABLE = 1 << 2
    };

    enum ResampleFilter
    {
      FILTER_LINEAR = 0,
      FILTER_LANCZOS,

      FILTER_NUM
    };

    ObjectCore() = delete;
    ~ObjectCore() = delete;

    static void Init();
  
    static SharedPtr<Mesh> GetMesh(const MeshDesc& aDesc);
    static SharedPtr<Mesh> CreateMesh(const MeshData& aMeshData);
    static uint64 ComputeMeshVertexHash(const MeshPartData* someMeshPartDatas, uint aNumParts);

    static SharedPtr<Material> GetMaterial(const MaterialDesc& aDesc);
    static SharedPtr<Material> CreateMaterial(const MaterialDesc& aDesc);

    static SharedPtr<TextureView> GetTexture(const char* aPath, uint someLoadFlags = 0);
    static SharedPtr<TextureView> LoadTexture(const char* aPath, uint someLoadFlags = 0); 
    static void ComputeMipmaps(const SharedPtr<Texture>& aTexture, ResampleFilter aFilter = FILTER_LANCZOS);

    static const ShaderPipeline* GetMipDownsampleShader() { return ourMipDownsampleShader.get(); }

  private:
    static eastl::hash_map<uint64, SharedPtr<TextureView>> ourTextureCache;
    static eastl::hash_map<uint64, SharedPtr<Mesh>> ourMeshCache;
    static eastl::hash_map<uint64, SharedPtr<Material>> ourMaterialCache;
    static SharedPtr<ShaderPipeline> ourMipDownsampleShader;
  };
}
