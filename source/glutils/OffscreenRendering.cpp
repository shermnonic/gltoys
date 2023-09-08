#include <glutils/OffscreenRendering.h>
#include <glutils/GLError.h>
#include <iostream>

OffscreenRendering::OffscreenRendering(size_t width, size_t height)
: width(width)
, height(height)
{
    textureWidth = std::max(textureWidth, static_cast<GLsizei>(width));
    textureHeight = std::max(textureHeight, static_cast<GLsizei>(height));
}

void OffscreenRendering::initGL()
{
    bool ok = true;
    ok |= target.create(GL_TEXTURE_2D);
    ok |= target.image(0, GL_RGBA32F, this->textureWidth, this->textureHeight,
                       0, GL_RGBA, GL_FLOAT, nullptr);
    if(ok)
    {
        ok |= renderToTexture.init_no_depthbuffer(target.name());
    }
    target.unbind();

    if(!ok)
    {
        std::cerr << "OffscreenRendering::initGL() - Could not create texture" << std::endl;
    }
    initialized = ok;
}

void OffscreenRendering::destroyGL()
{
    renderToTexture.deinit();
    target.destroy();
}

void OffscreenRendering::renderBegin()
{
    if(!initialized)
    {
        initGL();
    }

    if(initialized)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
    
        if(renderToTexture.bind(target.name()))
        {
            int viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            glViewport(0,0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        }
        else
        {
            std::cerr << "OffscreenRendering::renderBegin() - Could not bind render texture" << std::endl;
        }
    }
}

void OffscreenRendering::renderEnd()
{
    if(initialized)
    {
        downloadImageData();
        renderToTexture.unbind();
        glPopAttrib();
    }
}

const uint8_t* OffscreenRendering::getRawImageData() const
{
    return imageData.data();
}

void OffscreenRendering::downloadImageData(GLenum format)
{
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // allocate buffer
    size_t bytes_per_pixel = 0;
    switch( format ) 
    {
        case GL_DEPTH_COMPONENT:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_LUMINANCE: 
        case GL_ALPHA:           bytes_per_pixel = 1; break;
        case GL_LUMINANCE_ALPHA: bytes_per_pixel = 2; break;
        default:
        case GL_RGB:
        case GL_BGR:             bytes_per_pixel = 3; break;
        case GL_RGBA:
        case GL_BGRA:            bytes_per_pixel = 4; break;
    }
    size_t size = viewport[2]*viewport[3]*bytes_per_pixel;
    imageData.resize(size);
    
    if(!imageData.empty())
    {
        // read back framebuffer
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3],
                     format, GL_UNSIGNED_BYTE, imageData.data());
    }
}
