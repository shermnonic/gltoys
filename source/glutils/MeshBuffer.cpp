#include "MeshBuffer.h"
#include <stdexcept>

MeshBuffer::MeshBuffer(MeshPrimitiveType type, MeshVertexAttribute attributes)
    : m_type(type),
      m_attributes(attributes)
{
    switch (type)
    {
    case MeshPrimitiveType::Lines:     NumVertsPerPrimitive = 2; break;
    case MeshPrimitiveType::Triangles: NumVertsPerPrimitive = 3; break;
    case MeshPrimitiveType::Quads:     NumVertsPerPrimitive = 4; break;
    default:
        throw std::runtime_error("Invalid MeshPrimitiveType");
    }
}

void MeshBuffer::resize(size_t numVerts, size_t numPrimitives)
{
    if (                numVerts * 3 > m_vertices.size()) m_vertices.resize(numVerts * 3);
    if (hasNormals() && numVerts * 3 > m_normals .size()) m_normals .resize(numVerts * 3);
    if (hasColors () && numVerts * 4 > m_colors  .size()) m_colors  .resize(numVerts * 4);
    if (hasUVs    () && numVerts * 2 > m_uvs     .size()) m_uvs     .resize(numVerts * 2);
    
    size_t num_indices = numPrimitives * NumVertsPerPrimitive;
    if (num_indices > m_indices.size()) m_indices.resize(num_indices);
}

void MeshBuffer::ensure(size_t numAdditionalVerts, size_t numAdditionalPrimitives)
{
    while ((numVerticesAllocated() - numVertices()) < numAdditionalVerts)
    {
                          m_vertices.resize(2 * m_vertices.size());
        if (hasNormals()) m_normals .resize(2 * m_normals .size());
        if (hasColors ()) m_colors  .resize(2 * m_colors  .size());
        if (hasUVs    ()) m_uvs     .resize(2 * m_uvs     .size());
    }

    size_t num_additional_indices = numAdditionalPrimitives * NumVertsPerPrimitive;
    while ((numIndicesAllocated() - numIndices()) < num_additional_indices)
    {
        m_indices.resize(2 * m_indices.size());
    }
}

bool MeshBuffer::merge(const MeshBuffer& other)
{
    if (getPrimitiveType() != other.getPrimitiveType())
        return false;

    const size_t index_ofs = numVertices();
    {
        const size_t n0 = numVertices();
        const size_t n1 = other.numVertices();

        m_vertices.resize(n0 * 3);
        m_vertices.reserve((n0 + n1) * 3);
        m_vertices.insert(m_vertices.end(), other.getVertexData(0), other.getVertexData(0) + n1 * 3);

        if (hasNormals() && other.hasNormals())
        {
            m_normals.resize(n0 * 3);
            m_normals.reserve((n0 + n1) * 3);
            m_normals.insert(m_normals.end(), other.getNormalData(0), other.getNormalData(0) + n1 * 3);
        }
        if (hasColors() && other.hasColors())
        {
            m_colors.resize(n0 * 4);
            m_colors.reserve((n0 + n1) * 4);
            m_colors.insert(m_colors.end(), other.getColorData(0), other.getColorData(0) + n1 * 4);
        }
        if (hasUVs() && other.hasUVs())
        {
            m_uvs.resize(n0 * 2);
            m_uvs.reserve((n0 + n1) * 2);
            m_uvs.insert(m_uvs.end(), other.getUVData(0), other.getUVData(0) + n1 * 2);
        }
    }

    {
        const size_t n0 = numIndices();
        const size_t n1 = other.numIndices();

        m_indices.resize(n0 + n1);

        for (size_t i = 0; i < n1; ++i)
        {
            m_indices[n0 + i] = other.m_indices[i] + (unsigned)index_ofs;
        }
    }
    return true;
}
