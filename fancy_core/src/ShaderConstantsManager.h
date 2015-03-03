#ifndef INCLUDE_SHADERCONSTANTSUPDATER_H
#define INCLUDE_SHADERCONSTANTSUPDATER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "FixedArray.h"
#include "ObjectName.h"

// forward decls
namespace Fancy { namespace Rendering {
  class Renderer;
  class MaterialPassInstance;
} }

namespace Fancy { namespace Scene {
  class SceneNode;
  class Camera;
} }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class ConstantSemantics
  {
      NONE = 0,  // set manually

    // -------- PER_LAUNCH -------- //
    
    // -------- PER_FRAME -------- //

    // -------- PER_VIEWPORT -------- //
      RENDERTARGET_SIZE,  // (w, h, 1/w, 1/h)
    
    // -------- PER_STAGE -------- //

    // -------- PER_CAMERA -------- //
      VIEW_MATRIX,
      VIEW_INVERSE_MATRIX,
      PROJECTION_MATRIX,
      PROJECTION_INVERSE_MATRIX,
      VIEWPROJECTION_MATRIX,
      VIEWPROJECTION_INVERSE_MATRIX,
      NEARFAR_PARAMETERS, // (n, f, n/f, 1/f)
      CAMERA_POSITION_WORLDSPACE,

    // -------- PER_LIGHT -------- //
      DIRLIGHT_PARAMETERS,
      POINTLIGHT_PARAMETERS,
      SPOTLIGHT_PARAMETERS,
      LIGHT_COLORINTENSITY,
      LIGHT_POSITION_WORLDSPACE,
      LIGHT_POSITION_VIEWSPACE,

   // -------- PER_MATERIAL -------- //
      DIFFUSE_MATERIAL_COLORINTENSITY,
      SPECULAR_MATERIAL_COLORINTENSITY,

   // -------- PER_DRAW -------- //
      WORLD_MATRIX,
      WORLD_INVERSE_MATRIX,
      WORLDVIEW_MATRIX,
      WORLDVIEW_INVERSE_MATRIX,
      WORLDVIEWPROJECTION_MATRIX,
      WORLDVIEWPROJECTION_INVERSE_MATRIX,

      NUM,

      PER_LAUNCH_BEGIN = 0,
      PER_LAUNCH_END = 0,
      
      PER_FRAME_BEGIN = 0,
      PER_FRAME_END = 0,

      PER_VIEWPORT_BEGIN = RENDERTARGET_SIZE,
      PER_VIEWPORT_END = VIEW_MATRIX,

      PER_STAGE_BEGIN = 0,
      PER_STAGE_END = 0,

      PER_CAMERA_BEGIN = VIEW_MATRIX,
      PER_CAMERA_END = DIRLIGHT_PARAMETERS,

      PER_MATERIAL_BEGIN = DIFFUSE_MATERIAL_COLORINTENSITY,
      PER_MATERIAL_END = WORLD_MATRIX,

      PER_DRAW_BEGIN = WORLD_MATRIX,
      PER_DRAW_END = NUM
  };
//---------------------------------------------------------------------------//
 
//---------------------------------------------------------------------------//
  struct ConstantBufferElement
  {
    ConstantBufferElement() : 
      uOffsetBytes(0u), uSizeBytes(0u), eFormat(DataFormat::NONE), uFormatComponentCount(1u) {}

    ObjectName name;
    uint32 uOffsetBytes;  // Byte-offset from the start of the buffer
    uint32 uSizeBytes;  // Overall size of the element (==sizeof(eFormat) * uFormatComponentCount)
    DataFormat eFormat;
    uint8 uFormatComponentCount;  // Multiplier for eFormat. Used for multi-component elements (e.g. Matrices)
  };
//---------------------------------------------------------------------------//
  struct ShaderConstantsUpdateStage
  {
    const Renderer* pRenderer;
    const Scene::SceneNode* pModelNode;
    const Scene::Camera* pCamera;
    const MaterialPassInstance* pMaterial;
  };
//---------------------------------------------------------------------------//
  class ShaderConstantsManager
  {
    public:
      static ShaderConstantsManager& getInstance() {static ShaderConstantsManager instance; return instance;}
      /// Updates all constants in the provided constantbuffer-type
      static void update(ConstantBufferType eType);
      /// Returns false if no constant buffer exists for this type
      static bool hasBackingBuffer(ConstantBufferType eType);
      /// Registers an element obtained from shader-reflection
      static void registerElement(const ConstantBufferElement& element, ConstantSemantics eElementSemantic, ConstantBufferType eConstantBufferType);
      static void registerBufferWithSize(ConstantBufferType _eConstantBufferType, uint32 _requiredSizeBytes);

      /// Binds all constant buffers to the pipeline
      static void bindBuffers();

      static ConstantSemantics getSemanticFromName(const ObjectName& clName);
      static ConstantBufferType getConstantBufferTypeFromName(const ObjectName& clName);

      static ShaderConstantsUpdateStage updateStage;
    private:
      ShaderConstantsManager();
      ~ShaderConstantsManager();
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering

#endif  // INCLUDE_SHADERCONSTANTSUPDATER_H