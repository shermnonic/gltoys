#pragma once
#include <memory>
#include "MeshBuffer.h"
#include "GLConfig.h"

class GLMeshObject
{
public:
    bool prepare();
    void draw() const;

    void setMeshBuffer( std::shared_ptr<MeshBuffer> pbuf );
    void setDirty();

protected:
    bool ensureUploaded();
    bool ensureAllocated();

    bool create();
    bool allocate( size_t num_verts, size_t num_indices );
    bool upload() const;
    
private:
    std::shared_ptr<MeshBuffer> m_pMeshBuffer;
    
    GLuint m_vao =0;
    GLuint m_vbo =0;
    GLuint m_ibo =0;

    GLenum m_type =GL_TRIANGLES;
    bool m_hasNormals = true;
    bool m_hasColors = false;

    bool m_initialized = false;
    bool m_dirty       = true;
    size_t m_numVertsAllocated = 0;
    size_t m_numIndicesAllocated = 0;
    size_t m_numVerts=0;
    size_t m_numIndices=0;
};
