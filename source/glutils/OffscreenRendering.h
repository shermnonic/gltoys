#include <glutils/GLConfig.h>
#include <glutils/GLTexture.h>
#include <glutils/RenderToTexture.h>
#include <cstdint>
#include <vector>

class OffscreenRendering
{
public:
    OffscreenRendering(size_t width, size_t height);
    
    void renderBegin();
    void renderEnd();

    const uint8_t* getRawImageData() const;

    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    size_t getChannels() const { return 3; }

    void initGL();
    void destroyGL();

protected:
    void downloadImageData(GLenum format=GL_RGB);
    
private:
    bool initialized = false;
    size_t width = 4096;
    size_t height = 2304;
    GLsizei textureWidth = 4096;
    GLsizei textureHeight = 4096;
    GL::GLTexture target;
    RenderToTexture renderToTexture;
    std::vector<uint8_t> imageData;
};
