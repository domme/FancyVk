#ifndef INCLUDE_SCENERENDERDESCRIPTION_H
#define INCLUDE_SCENERENDERDESCRIPTION_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "FixedArray.h"
#include "SmallObjectAllocator.h"
#include <array>

// Forward declarations:
namespace Fancy { namespace Geometry { 
  class GeometryData;
} } 

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct RenderQueueItem
  {
    RenderQueueItem() : myGeometry(nullptr), myMaterial(nullptr) {}
    RenderQueueItem(const Geometry::GeometryData* aGeometry, const Material* aMaterial, const glm::mat4& aWorldMat) :
      myGeometry(aGeometry), myMaterial(aMaterial), myWorldMat(aWorldMat) {}

    glm::mat4 myWorldMat;
    const Geometry::GeometryData* myGeometry;
    const Material* myMaterial;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  struct RenderQueue
  {
    static const uint kMaxNumRenderQueueItems = 4096u;

    RenderQueue();
    ~RenderQueue();

    RenderQueueItem* AddItem();
    void Clear();
    bool IsEmpty() const { return myItems.empty(); }
    const FixedArray<RenderQueueItem, kMaxNumRenderQueueItems>& GetItems() const { return myItems; }

  private:
    // SmallObjectAllocator<RenderQueueItem> myAllocator;
    FixedArray<RenderQueueItem, kMaxNumRenderQueueItems> myItems;
  };
//---------------------------------------------------------------------------//

} }  // end of namespace Fancy::Rendering

#endif // INCLUDE_SCENERENDERDESCRIPTION_H