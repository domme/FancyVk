#ifndef INCLUDE_TEXTURE_H
#define INCLUDE_TEXTURE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_TEXTURE

#include "StaticManagedObject.h"

namespace Fancy { namespace Rendering {

class Texture : public PLATFORM_DEPENDENT_NAME(Texture), public StaticManagedHeapObject<Texture>
{

};

} } // end of namespace Fancy::Rendering


#endif  // INCLUDE_TEXTURE_H