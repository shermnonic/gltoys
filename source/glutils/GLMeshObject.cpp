#include "GLMeshObject.h"
#include "GLError.h"
#include <algorithm>

bool GLMeshObject::prepare()
{
    return ensureAllocated() && ensureUploaded();
}

void GLMeshObject::draw() const
{
    assert( m_initialized );
    if( m_dirty )
        return;

    if( m_numIndices>0 )
    {
        glBindVertexArray(m_vao);
        glDrawElements(m_type, static_cast<GLsizei>(m_numIndices), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
        glBindVertexArray(0);
        GL::checkGLError("GLMesh::draw()");
    }
}

void GLMeshObject::setMeshBuffer( std::shared_ptr<MeshBuffer> pbuf )
{
    m_pMeshBuffer = pbuf;
    m_type = static_cast<GLenum>(pbuf->getPrimitiveType());
    m_hasNormals = pbuf->hasNormals();
    m_hasColors = pbuf->hasColors();
    setDirty();
}

void GLMeshObject::setDirty()
{
    m_numVerts   = m_pMeshBuffer->numVertices();
    m_numIndices = m_pMeshBuffer->numIndices();
    m_dirty = true;
}

bool GLMeshObject::ensureUploaded()
{
    if( m_dirty )
    {
        m_dirty = !upload();
    }
    return !m_dirty;
}

bool GLMeshObject::ensureAllocated()
{
    if( !m_initialized )
        if( !create() )
            return false;

    bool needs_realloc = m_numVerts > m_numVertsAllocated || m_numIndices > m_numIndicesAllocated;
    if( needs_realloc )
        return allocate( m_numVerts, m_numIndices );
    
    return true;
}

bool GLMeshObject::create()
{
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);
    glGenVertexArrays(1, &m_vao);
    GL::checkGLError("GLMesh::create()");
    m_initialized = true;
    return true;
}

bool GLMeshObject::allocate( size_t num_verts, size_t num_indices )
{
    struct FloatPointer {
        GLuint index;
        GLint channels;
        GLboolean normalized;
        GLsizei stride;
        bool active;
    };

    const std::vector<FloatPointer> pointers {
        { 0, 3, GL_FALSE, 0, true },
        { 1, 3, GL_FALSE, 0, m_hasNormals },
        { 3, 4, GL_FALSE, 0, m_hasColors } };

    auto getTotalNumFloats = [&num_verts, &pointers]() -> size_t
    {
        size_t total = 0;
        for (const auto& p : pointers)
        {
            if (p.active)
                total += p.channels * num_verts;
        }
        return total;
    };

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, getTotalNumFloats()*sizeof(float), nullptr, GL_STATIC_DRAW);

    size_t offsetInFloats = 0;
    for (const auto& p : pointers)
    {
        if (p.active)
        {
            glEnableVertexAttribArray(p.index);
            const void* ptr = reinterpret_cast<void*>(offsetInFloats * sizeof(float));
            glVertexAttribPointer(p.index, p.channels, GL_FLOAT, p.normalized, p.stride, ptr);
            offsetInFloats += p.channels * num_verts;
        }
        else
        {
            glDisableVertexAttribArray(p.index);
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*num_indices, nullptr, GL_STATIC_DRAW);

    glBindVertexArray(0);

    bool success = GL::checkGLError("GLMesh::allocate()");
    if( success )
    {
        m_numVertsAllocated = num_verts;
        m_numIndicesAllocated = num_indices;
    }
    else
    {
        m_numVertsAllocated = 0;
        m_numIndicesAllocated = 0;
    }
    return success;
}

bool GLMeshObject::upload() const
{
    GL::checkGLError("GLMesh::upload() - begin");

    assert(m_pMeshBuffer->numVertices() == m_numVerts);
    assert(m_numVerts <= m_numVertsAllocated);
    assert(m_pMeshBuffer->numIndices() == m_numIndices);
    assert(m_numIndices <= m_numIndicesAllocated);

    size_t vsize = m_numVerts*3*sizeof(float);
    size_t vofs = m_numVertsAllocated*3*sizeof(float);
    size_t isize = m_numIndices*sizeof(unsigned);

    size_t csize = m_numVerts * 4 * sizeof(float);
    size_t cofs = vofs + vsize;

    if (vsize > 0 && isize > 0)
    {
        size_t offsetInFloats = 0;

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        auto bufferSubData = [](size_t offsetInFloats, size_t count, size_t channels, const float* data)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 
                static_cast<GLintptr>(offsetInFloats * sizeof(float)), 
                static_cast<GLsizeiptr>(count * channels * sizeof(float)), 
                data);
        };

        bufferSubData(offsetInFloats, m_numVerts, 3, m_pMeshBuffer->getVertexData());
        offsetInFloats += m_numVertsAllocated * 3;

        if (m_hasNormals)
        {
            bufferSubData(offsetInFloats, m_numVerts, 3, m_pMeshBuffer->getNormalData());
            offsetInFloats += m_numVertsAllocated * 3;
        }
        if (m_hasColors)
        {
            bufferSubData(offsetInFloats, m_numVerts, 4, m_pMeshBuffer->getColorData());
            offsetInFloats += m_numVertsAllocated * 4;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, isize, m_pMeshBuffer->getIndexData());

        glBindVertexArray(0);
    }

    return GL::checkGLError("GLMesh::upload()");
}    
