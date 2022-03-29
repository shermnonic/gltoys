#pragma once
#include <vector>
#include <cassert>
#include <algorithm> // min
#include <limits>

#include "MeshBufferTypes.h"

class MeshBuffer
{
public:
    MeshBuffer(MeshPrimitiveType type = MeshPrimitiveType::Triangles, MeshVertexAttribute attributes = MeshVertexAttribute::Normal);

    MeshPrimitiveType getPrimitiveType() const { return m_type; }
    size_t getNumVertsPerPrimitive() const { return NumVertsPerPrimitive; }
    
    MeshVertexAttribute getAttributes() const { return m_attributes; }

    bool hasAttribute(MeshVertexAttribute a) const { return has(m_attributes, a); }
    bool hasNormals() const { return hasAttribute(MeshVertexAttribute::Normal); }
    bool hasColors()  const { return hasAttribute(MeshVertexAttribute::Color);  }
    bool hasUVs()     const { return hasAttribute(MeshVertexAttribute::UV); }

    void resize(size_t numVerts, size_t numPrimitives);

    void ensure(size_t numAdditionalVerts, size_t numAdditionalPrimitives);

    bool merge(const MeshBuffer& other);
    
    void setVertices( std::vector<float> v ) { assert(v.size()%3==0); m_vertices = v; }
    void setNormals ( std::vector<float> n ) { assert(hasNormals() && n.size()%3==0); m_normals = n; }
    void setColors  ( std::vector<float> c ) { assert(hasColors()  && c.size()%4==0); m_colors  = c; }
    void setUVs     ( std::vector<float> t ) { assert(hasUVs()     && t.size()%2==0); m_uvs     = t; }
    void setIndices ( std::vector<unsigned> i ) { m_indices = i; }

    size_t numVerticesAllocated() const { return m_vertices.size() / 3; }
    size_t numIndicesAllocated () const { return m_indices .size(); }

    size_t numVertices() const { return std::min(m_numVertices,numVerticesAllocated()); }
    size_t numIndices()  const { return std::min(m_numIndices, numIndicesAllocated ()); }

    void setNumVertices( size_t n ) { m_numVertices = n; }
    void setNumIndices ( size_t n ) { m_numIndices  = n; }
    
          float*    getVertexData( size_t vidx=0 )       { assert(3*vidx<m_vertices.size()); return m_vertices.data() + 3*vidx; }
          float*    getNormalData( size_t vidx=0 )       { assert(3*vidx<m_normals .size()); return m_normals .data() + 3*vidx; }
          float*    getUVData    ( size_t vidx=0 )       { assert(2*vidx<m_uvs     .size()); return m_uvs     .data() + 2*vidx; }
          float*    getColorData ( size_t vidx=0 )       { assert(4*vidx<m_colors  .size()); return m_colors  .data() + 4*vidx; }
          unsigned* getIndexData ( size_t pidx=0 )       { assert(NumVertsPerPrimitive*pidx<m_indices .size()); return m_indices .data() + NumVertsPerPrimitive*pidx; }
    
    const float*    getVertexData( size_t vidx=0 ) const { assert(3*vidx<m_vertices.size()); return m_vertices.data() + 3*vidx; }
    const float*    getNormalData( size_t vidx=0 ) const { assert(3*vidx<m_normals .size()); return m_normals .data() + 3*vidx; }
    const float*    getUVData    ( size_t vidx=0 ) const { assert(2*vidx<m_uvs     .size()); return m_uvs     .data() + 2*vidx; }
    const float*    getColorData ( size_t vidx=0 ) const { assert(4*vidx<m_colors  .size()); return m_colors  .data() + 4*vidx; }
    const unsigned* getIndexData ( size_t pidx=0 ) const { assert(NumVertsPerPrimitive*pidx<m_indices .size()); return m_indices .data() + NumVertsPerPrimitive*pidx; }
    
private:
    MeshPrimitiveType m_type;
    MeshVertexAttribute m_attributes;
    size_t NumVertsPerPrimitive;

    std::vector<float>    m_vertices;
    std::vector<float>    m_normals;
    std::vector<float>    m_uvs;
    std::vector<float>    m_colors;
    std::vector<unsigned> m_indices;

    size_t m_numVertices = std::numeric_limits<size_t>::max();
    size_t m_numIndices  = std::numeric_limits<size_t>::max();
};
