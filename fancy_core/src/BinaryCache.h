#ifndef INCLUDE_BINARYCACHE_H
#define INCLUDE_BINARYCACHE_H

#include "FancyCorePrerequisites.h"
#include "PathService.h"
#include "Texture.h"
#include "Serializer.h"
#include "StringUtil.h"

namespace Fancy{ namespace Rendering{
struct TextureDesc;
} }

namespace Fancy { namespace Geometry {
struct MeshDesc;
class Mesh;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class BinaryCache
  {
    public:
  //---------------------------------------------------------------------------//
    static String getCacheFilePathAbs(const String& aPathInResources);
    static bool write(Rendering::Texture* aTexture, void* someData, uint32 aDataSize);
    static bool write(Geometry::Mesh* aMesh, void** someVertexDatas, void** someIndexDatas);
    static bool read(Rendering::Texture** aTexture, const Rendering::TextureDesc& aDesc, uint32 aTimeStamp);
    static bool read(Geometry::Mesh** aMesh, const Geometry::MeshDesc& aDesc, uint32 aTimeStamp);
  //---------------------------------------------------------------------------//      
};
} }  // end of namespace Fancy::IO 

#endif  // INCLUDE_BINARYCACHE_H