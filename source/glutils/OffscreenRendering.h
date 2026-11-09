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

    size_t getWidth() const { return static_cast<std::size_t>(textureWidth); }
    size_t getHeight() const { return static_cast<std::size_t>(textureHeight); }
    size_t getChannels() const { return 3; }

    void initGL();
    void destroyGL();

protected:
    void downloadImageData(GLenum format);
    
private:
    bool initialized = false;
    GLsizei textureWidth = 4096;
    GLsizei textureHeight = 4096;
    GL::GLTexture target;
    RenderToTexture renderToTexture;
    std::vector<uint8_t> imageData;
};
