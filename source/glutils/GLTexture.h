#ifndef GLTEXTURE_H
#define GLTEXTURE_H
#include "GLConfig.h"
#include <stdexcept>
#include <cassert>

#ifdef GL_NAMESPACE
namespace GL
{
#endif

/// Simple facade for OpenGL texture functions
class GLTexture
{
public:
    GLTexture();
    ~GLTexture();

    ///@name Facade
    ///@{
    bool create( GLenum target = GL_TEXTURE_2D );
    bool destroy();

    bool bind( int texunit=-1 ); // -1 implies not to call glActiveTexture
    void unbind();

    bool image( GLint level, GLint internalformat,
        GLsizei width, GLsizei height, GLsizei depth, 
        GLint border, GLenum format, GLenum type, void *data );
    bool image( GLint level, GLint internalformat,
        GLsizei width, GLsizei height, 
        GLint border, GLenum format, GLenum type, void *data );
    bool image( GLint level, GLint internalformat,
        GLsizei width, 
        GLint border, GLenum format, GLenum type, void *data );

    bool subImage( GLint level, 
        GLint xoffset, GLint yoffset, GLint zoffset,
        GLsizei width, GLsizei height, GLsizei depth,
        GLenum format, GLenum type, void *data, bool bind = true );
    bool subImage(GLint level, 
        GLint xoffset, GLint yoffset,
        GLsizei width, GLsizei height,
        GLenum format, GLenum type, void *data, bool bind = true );
    bool subImage(GLint level, 
        GLint xoffset,
        GLsizei width, 
        GLenum format, GLenum type, void *data, bool bind = true );
        
    bool setParameter( GLenum pname, GLint value );
    bool setParameter( GLenum pname, GLfloat value ); 
    ///@} Facade
    
    ///@name Convenience
    ///@{
    
    GLenum target() const;
    GLuint name() const;
    
    ///@{ Get texture size (set on call to \a image()) 
    int width () const { return m_width;  }
    int height() const { return m_height; }
    int depth () const { return m_depth;  }
    ///@}
    
    void swap( GLTexture &tex ) 
    { 
        // swap members
        std::swap( m_id    , tex.m_id     );
        std::swap( m_target, tex.m_target );
        std::swap( m_valid , tex.m_valid  );
        std::swap( m_width , tex.m_width  );
        std::swap( m_height, tex.m_height );
        std::swap( m_depth , tex.m_depth  );
    }
    
    bool setWrapMode( GLint mode );
    bool setFilterMode( GLint mode );
    bool setFilterMode( GLint min_mode, GLint mag_mode );
    ///@} Convenience

protected:
    void setSize( GLint w, GLint h=0, GLint d=0 )
    {
        m_width=w; m_height=h; m_depth=d;
    }
    
    void invalidate()
    {
        m_id = 0;
        m_valid = false;
    }

private:
    bool m_valid;
    GLuint m_id;
    GLenum m_target;
    GLint m_width, m_height, m_depth;  ///< texture size according to last \a Image() call, 0 indicates undefined size
};

#ifdef GL_NAMESPACE
}
#endif

namespace std
{
inline void swap(GL::GLTexture &a, GL::GLTexture &b)
{
    a.swap(b);
}
}

#endif // GLTEXTURE_H
