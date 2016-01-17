#ifndef INCLUDE_GPUBUFFERGL4_H
#define INCLUDE_GPUBUFFERGL4_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#if defined (RENDERER_OPENGL4)

#include "FixedArray.h"
#include "ObjectName.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GpuBufferGL4
  {
    public:
      GpuBufferGL4();
      ~GpuBufferGL4();

      bool isLocked() const { return m_clStateInfos.isLocked; }
      bool isLockedPersistent() const { return m_clStateInfos.isLockedPersistent; }
      bool isValid() const { return !m_vGLhandles.empty(); }
      GLuint getGLhandle() const { return m_vGLhandles[getBufferIndex()]; }
      uint getTotalSizeBytes() const { return m_clParameters.uTotalSizeBytes; }
      uint32 getNumElements() const { return m_clParameters.uNumElements; }
      GpuBufferCreationParams getParameters() const { return static_cast<GpuBufferCreationParams>(m_clParameters); }

      void setBufferData(void* pData, uint uOffsetElements = 0, uint uNumElements = 0);
      void create(const GpuBufferCreationParams& clParameters, void* pInitialData = nullptr);
      void destroy();
      void* lock(GpuResoruceLockOption eLockOption, uint uOffsetElements = 0u, uint uNumElements = 0u);
      void unlock();

      ObjectName getName() const { return myName; }
      void setName(const ObjectName& _aName) { myName = _aName; }
       
    private:
  //---------------------------------------------------------------------------//
      struct BufferParametersGL 
        : public GpuBufferCreationParams
      {
        BufferParametersGL() : eInitialBufferTargetGL(0), eBindingQueryType(0),
          uAccessFlagsGL(0), uTotalSizeBytes(0) {}

        /// The OpenGL-binding point the buffer was initially created for
        GLenum eInitialBufferTargetGL;
        GLenum eBindingQueryType;
        GLuint uAccessFlagsGL;
        uint uTotalSizeBytes;
      };
  //---------------------------------------------------------------------------//
      struct BufferState {
        BufferState() : 
          isLocked(0u), 
          isLockedPersistent(0u) {}

        uint isLocked : 1;
        uint isLockedPersistent : 1;
      };
  //---------------------------------------------------------------------------//
      enum class MultiBufferingStrategy {
        NONE = 0,
        BUFFERS,  // toggling between distinct buffers
        OFFSETS   // toggling between offsets in one larger buffer
      };
  //---------------------------------------------------------------------------//
      void _unlock(uint32 uBufferIndex);
      uint32 getBufferIndex() const {return m_uDerivedInternalBufferCount == 1u ? 0u 
                                        : MultiBuffering::getCurrentBufferIndex();}

      ObjectName myName;
      BufferParametersGL m_clParameters;
      BufferState m_clStateInfos;
      uint32 m_uDerivedInternalBufferCount;
      MultiBufferingStrategy m_eMultiBufferStrategy;

      FixedArray<GLuint, MultiBuffering::kGpuMultiBufferingCount> m_vGLhandles;
      void* m_pCachedLockPtr;
  //---------------------------------------------------------------------------//
  };

} } } // end of namespace Fancy::Rendering::GL4

#endif

#endif  // INCLUDE_GPUBUFFERGL4_H