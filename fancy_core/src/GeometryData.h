#ifndef INCLUDE_GEOMETRYDATA_H
#define INCLUDE_GEOMETRYDATA_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "OpenGLprerequisites.h"
#include "GeometryVertexLayout.h"
#include "GpuBuffer.h"
#include "StaticManagedObject.h"
#include "ObjectName.h"

namespace Fancy { namespace Geometry {
//---------------------------------------------------------------------------//
  class GeometryData {
    public:
      GeometryData();
      ~GeometryData();

      const Rendering::GeometryVertexLayout& getGeometryVertexLayout() const {return m_vertexLayout;}
      const Rendering::GpuBuffer* getVertexBuffer() const {return m_pVertexBuffer.get();}
      const Rendering::GpuBuffer* getIndexBuffer() const {return m_pIndexBuffer.get();}
      uint32 getNumVertices() const {return m_pVertexBuffer ? m_pVertexBuffer->GetNumElements() : 0u; }
      uint32 getNumIndices() const {return m_pIndexBuffer ? m_pIndexBuffer->GetNumElements() : 0u; }
      uint32 getVertexStrideBytes() const {return m_vertexLayout.getStrideBytes();}
      
      void setVertexBuffer(const SharedPtr<Rendering::GpuBuffer>& _pVertexBuffer) {m_pVertexBuffer = _pVertexBuffer;}
      void setIndexBuffer(const SharedPtr<Rendering::GpuBuffer>& _pIndexBuffer) {m_pIndexBuffer = _pIndexBuffer;}
      void setVertexLayout(const Rendering::GeometryVertexLayout& _rVertexLayout) {m_vertexLayout = _rVertexLayout;}

    protected:
      Rendering::GeometryVertexLayout m_vertexLayout;
      SharedPtr<Rendering::GpuBuffer> m_pVertexBuffer;  // TODO: put into array for multiple vertex-streams?
      SharedPtr<Rendering::GpuBuffer> m_pIndexBuffer;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Geometry::

#endif  // INCLUDE_GEOMETRYDATA_H