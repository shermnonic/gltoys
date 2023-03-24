// Max Hermann, March 22, 2010
#include "GLConfig.h"
#include "RenderToTexture.h"
#include "GLError.h"
#include <cassert>

using namespace GL;

bool RenderToTexture::init( int width, int height, GLint texid,
                            bool attach_depth )
{
    // FIXME: Width and height are only needed for depth buffer attachement?!
    m_width = width;
    m_height = height;

    // gen FBO
    glGenFramebuffers( 1, &m_fbo );
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    
    // attach texture   
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                            GL_TEXTURE_2D, texid, 0 );
    
    // create and attach Depthbuffer
    if( attach_depth )
    {
#ifndef RENDERTOTEXTURE_DEPTHBUFFER_AS_RENDERBUFFER

        // -- create depth texture
      #if 1
        // my first guess
        GLint  depth_int_format = GL_DEPTH_COMPONENT24;
        GLenum depth_format = GL_DEPTH_COMPONENT;
        GLenum depth_type   = GL_FLOAT;
      #else
        // with stencil buffer
        GLint  depth_int_format = GL_DEPTH24_STENCIL8;
        GLenum depth_format = GL_DEPTH_STENCIL;
        GLenum depth_type   = GL_UNSIGNED_INT_24_8;
      #endif
        glGenTextures( 1, &m_depth );
        glBindTexture( GL_TEXTURE_2D, m_depth );
        glTexImage2D( GL_TEXTURE_2D, 0, depth_int_format, width,height, 0,
                      depth_format, depth_type, 0 );    

        // avoid shadow map comparison
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D, m_depth, 0 );

        //glFramebufferTexture2( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
        //                       GL_TEXTURE_2D, m_depth, 0);
#else
        // gen Renderbuffer
        glGenRenderbuffers( 1, &m_depth );
        glBindRenderbuffer( GL_RENDERBUFFER, m_depth );

        if( !checkGLError("RenderToTexture::init() - after Gen/BindRenderBuffer()") )
            return false;
        
        // init as Depthbuffer
        glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                               m_width, m_height );

        if( !checkGLError("RenderToTexture::init() - after RenderBufferStorage()") )
            return false;

        // attach to FBO
        glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                   GL_RENDERBUFFER, m_depth );

        if( !checkGLError("RenderToTexture::init() - after FrameBufferRenderBuffer()") )
            return false;
#endif
    }   
    m_depth_attached = attach_depth;
    
    unbind();
    m_initialized = true;
    
    return checkGLError("RenderToTexture::init()");
}

bool RenderToTexture::deinit()
{
    unbind();
    if( m_depth_attached )
#ifdef RENDERTOTEXTURE_DEPTHBUFFER_AS_RENDERBUFFER
        glDeleteRenderbuffers( 1, &m_depth );
#else
        glDeleteTextures( 1, &m_depth );
#endif
    glDeleteFramebuffers( 1, &m_fbo );
    
    return checkGLError("RenderToTexture::deinit()");
}

bool RenderToTexture::bind( GLint texid, bool bind_depth )
{
    assert(m_initialized);
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
    
    // attach texture   
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                            GL_TEXTURE_2D, texid, 0 );
    
    if( m_depth_attached && bind_depth )
#ifdef RENDERTOTEXTURE_DEPTHBUFFER_AS_RENDERBUFFER
        glBindRenderbuffer( GL_RENDERBUFFER, m_depth );
#else
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D, m_depth, 0 );
#endif

//  GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
//  glDrawBuffers( 1, buffers );
    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    return checkGLError("RenderToTexture::bind()"); 
}


bool RenderToTexture::unbind()
{
    checkGLError("Before RenderToTexture::unbind()");
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

#ifdef RENDERTOTEXTURE_DEPTHBUFFER_AS_RENDERBUFFER
    if( m_depth_attached )
        glBindRenderbuffer( GL_RENDERBUFFER, 0 );
#endif

    // FIXME: unbinding Framebuffer automatically resets DrawBuffer?

    return checkGLError("RenderToTexture::unbind()");   
}

