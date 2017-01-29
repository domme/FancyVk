#include "FancyCorePrerequisites.h"
#include "MaterialPassInstanceDesc.h"

#include "MaterialPassInstanceDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  uint64 MaterialPassInstanceDesc::GetHash() const
  {
    uint64 hash;
    MathUtil::hash_combine(hash, myMaterialPass.GetHash());
    
    for (const TextureDesc& desc : myReadTextures)
      MathUtil::hash_combine(hash, desc.GetHash());

    for (const TextureDesc& desc : myWriteTextures)
      MathUtil::hash_combine(hash, desc.GetHash());

    for (const GpuBufferDesc& desc : myReadBuffers)
      MathUtil::hash_combine(hash, desc.GetHash());

    for (const GpuBufferDesc& desc : myWriteBuffers)
      MathUtil::hash_combine(hash, desc.GetHash());

    for (const TextureSamplerDesc& desc : myTextureSamplers)
      MathUtil::hash_combine(hash, desc.GetHash());

    return hash;
  }
//---------------------------------------------------------------------------//
  bool MaterialPassInstanceDesc::operator==(const MaterialPassInstanceDesc& anOther) const
  {
    return GetHash() == anOther.GetHash();
  }
//---------------------------------------------------------------------------//
  bool MaterialPassInstanceDesc::IsEmpty() const
  {
    return myMaterialPass.IsEmpty();
  }
//---------------------------------------------------------------------------//
  void MaterialPassInstanceDesc::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&myMaterialPass, "myMaterialPass");
    aSerializer->serializeArray(myReadTextures, "myReadTextures");
    aSerializer->serializeArray(myWriteTextures, "myWriteTextures");
    aSerializer->serializeArray(myReadBuffers, "myReadBuffers");
    aSerializer->serializeArray(myWriteBuffers, "myWriteBuffers");
    aSerializer->serializeArray(myTextureSamplers, "myTextureSamplers");
  }
//---------------------------------------------------------------------------//
} }
