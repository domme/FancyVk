#include "AssetStorage.h"
#include "Material.h"
#include "TextureLoader.h"
#include "MaterialDesc.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/PathService.h"
#include "ModelDesc.h"
#include "Model.h"
#include "fancy_core/BinaryCache.h"
#include <fancy_core/Mesh.h>
#include "fancy_core/Texture.h"

using namespace Fancy;

//---------------------------------------------------------------------------//
  void AssetStorage::Clear()
  {
    myMaterials.clear();
    myModels.clear();
    myTextures.clear();
    myMeshes.clear();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> AssetStorage::GetTexture(const char* aPath, uint someFlags /* = 0*/)
  {
    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Resources::FindPath(texPathAbs);
    else
      texPathRel = Resources::FindName(texPathAbs);

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64) someFlags & SHADER_WRITABLE));

    auto it = myTextures.find(texPathRelHash);
    if (it != myTextures.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Material> AssetStorage::CreateMaterial(const MaterialDesc& aDesc)
  {
    const uint64 descHash = aDesc.GetHash();

    auto it = myMaterials.find(descHash);
    if (it != myMaterials.end())
      return it->second;

    TextureViewProperties srvViewProps;
    srvViewProps.myDimension = GpuResourceDimension::TEXTURE_2D;

    SharedPtr<Material> mat(new Material);
    for (uint i = 0u; i < aDesc.mySemanticTextures.size(); ++i)
    {
      SharedPtr<Texture> tex = CreateTexture(aDesc.mySemanticTextures[i].c_str());
      if (tex != nullptr)
        mat->mySemanticTextures[i] = RenderCore::CreateTextureView(tex, srvViewProps);
    }
    
    for (const String& texPath : aDesc.myExtraTextures)
    {
      SharedPtr<Texture> tex = CreateTexture(texPath.c_str());
      if (tex != nullptr)
        mat->myExtraTextures.push_back(RenderCore::CreateTextureView(tex, srvViewProps));
    }

    for (uint i = 0u; i < aDesc.mySemanticParameters.size(); ++i)
      mat->mySemanticParameters[i] = aDesc.mySemanticParameters[i];

    for (float param : aDesc.myExtraParameters)
      mat->myExtraParameters.push_back(param);
    
    myMaterials[descHash] = mat;
    return mat;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> AssetStorage::CreateTexture(const char* aPath, uint someLoadFlags/* = 0*/)
  {
    if (strlen(aPath) == 0)
      return nullptr;

    String texPathAbs = aPath;
    String texPathRel = aPath;
    if (!Path::IsPathAbsolute(texPathAbs))
      texPathAbs = Resources::FindPath(texPathAbs);
    else
      texPathRel = Resources::FindName(texPathAbs);

    if ((someLoadFlags & NO_MEM_CACHE) == 0)
      if (SharedPtr<Texture> texFromMemCache = GetTexture(texPathRel.c_str()))
        return texFromMemCache;

    uint64 texPathRelHash = MathUtil::Hash(texPathRel);
    MathUtil::hash_combine(texPathRelHash, ((uint64) someLoadFlags & SHADER_WRITABLE));

    if ((someLoadFlags & NO_DISK_CACHE) == 0)
    {
      uint64 timestamp = Path::GetFileWriteTime(texPathAbs);
      if (SharedPtr<Texture> texFromDiskCache = BinaryCache::ReadTexture(texPathRel.c_str(), timestamp))
      {
        myTextures[texPathRelHash] = texFromDiskCache;
        return texFromDiskCache;
      }  
    }

    std::vector<uint8> textureBytes;
    TextureLoadInfo texLoadInfo;
    if (!TextureLoader::Load(texPathAbs.c_str(), textureBytes, texLoadInfo))
    {
      LOG_ERROR("Failed to load texture at path %", texPathAbs);
      return nullptr;
    }

    TextureProperties texProps;
    texProps.myDimension = GpuResourceDimension::TEXTURE_2D;
    texProps.path = texPathRel;
    texProps.bIsDepthStencil = false;
    texProps.myWidth = texLoadInfo.width;
    texProps.myHeight = texLoadInfo.height;
    texProps.myDepthOrArraySize = 0u;
    texProps.myAccessType = GpuMemoryAccessType::NO_CPU_ACCESS;
    texProps.myIsShaderWritable = (someLoadFlags & SHADER_WRITABLE) != 0;

    if (!(texLoadInfo.bitsPerChannel == 8u || texLoadInfo.bitsPerChannel == 16u))
    {
      LOG_ERROR("Unsupported bits per channel in texture % (has % bits per channel)", texPathAbs, texLoadInfo.bitsPerChannel);
      return nullptr;
    }

    switch(texLoadInfo.numChannels)
    {
      case 1: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::R_8 : DataFormat::R_16; break;
      case 2: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::RG_8 : DataFormat::RG_16; break;

      // LodePNG assumes that 8-bit RGB or RGBA data is always SRGB. The official "sRGB" info-chunk from the PNG format is not supported
      case 3: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::SRGB_8 : DataFormat::RGB_16; break;
      case 4: texProps.eFormat = texLoadInfo.bitsPerChannel == 8 ? DataFormat::SRGB_8_A_8 : DataFormat::RGBA_16; break;
      default: ASSERT(false, "Unsupported channels");
      return nullptr;
    }

    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.eFormat);
    const uint expectedDataSize = formatInfo.mySizeBytes * texProps.myWidth * texProps.myHeight;
    if (expectedDataSize != textureBytes.size())
    {
      LOG_ERROR("Invalid pixel data loaded from texture %. Expected: %, Actual: %", texPathAbs, expectedDataSize, textureBytes.size());
      return nullptr;
    }
    
    TextureSubData uploadData;
    uploadData.myData = textureBytes.data();
    uploadData.myPixelSizeBytes = (texLoadInfo.bitsPerChannel * texLoadInfo.numChannels) / 8u;
    uploadData.myRowSizeBytes = texLoadInfo.width * uploadData.myPixelSizeBytes;
    uploadData.mySliceSizeBytes = texLoadInfo.width * texLoadInfo.height * uploadData.myPixelSizeBytes;
    uploadData.myTotalSizeBytes = uploadData.mySliceSizeBytes;

    SharedPtr<Texture> tex = RenderCore::CreateTexture(texProps, texPathRel.c_str(), &uploadData, 1u);
    // RenderCore::ComputeMipMaps(tex);

    if (tex != nullptr)
    {
      BinaryCache::WriteTexture(tex.get(), uploadData);

      myTextures[texPathRelHash] = tex;
      return tex;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Model> AssetStorage::CreateModel(const ModelDesc& aDesc)
  {
    const uint64 hash = aDesc.GetHash();

    auto it = myModels.find(hash);
    if (it != myModels.end())
      return it->second;

    SharedPtr<Model> model(new Model);
    model->myMaterial = CreateMaterial(aDesc.myMaterial);
    model->myMesh = GetMesh(aDesc.myMesh);
    
    myModels[hash] = model;
    return model;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> AssetStorage::GetMesh(const MeshDesc& aDesc)
  {
    auto it = myMeshes.find(aDesc.GetHash());
    if (it != myMeshes.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> AssetStorage::CreateMesh(const MeshDesc& aDesc, MeshData* someMeshDatas, uint aNumMeshDatas, uint64 aMeshFileTimestamp /* = 0u */)
  {
    if (aMeshFileTimestamp != 0u)
    {
      SharedPtr<Mesh> meshFromDiskCache = BinaryCache::ReadMesh(aDesc, aMeshFileTimestamp);
      if (meshFromDiskCache)
      {
        myMeshes[meshFromDiskCache->myVertexAndIndexHash] = meshFromDiskCache;
        return meshFromDiskCache;
      }
    }
    
    SharedPtr<Mesh> mesh = RenderCore::CreateMesh(aDesc, someMeshDatas, aNumMeshDatas);
    if (mesh)
    {
      BinaryCache::WriteMesh(mesh.get(), someMeshDatas, aNumMeshDatas);

      myMeshes[aDesc.GetHash()] = mesh;
      return mesh;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
