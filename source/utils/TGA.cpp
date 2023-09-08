#include <utils/TGA.h>
#include <fstream>

TGAHeader::TGAHeader(size_t width, size_t height, TGAFormat format)
{
    this->width[0] = static_cast<uint8_t>(width%256);
    this->width[1] = static_cast<uint8_t>(width/256);
    this->height[0] = static_cast<uint8_t>(height%256);
    this->height[1] = static_cast<uint8_t>(height/256);
    const size_t channels = format == TGAFormat::RGBA ? 4 : 3;
    this->pixelSize = static_cast<uint8_t>(channels * 8);
}

bool saveTGA(std::filesystem::path filename, TGAFormat format, size_t width, size_t height, const uint8_t* imageData)
{
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    if(file.is_open())
    {
        // WORKAROUND: Force width to be multiple of 4
        width -= width%4;
        
        const size_t channels = format==TGAFormat::RGBA ? 4 : 3;

        TGAHeader header(width, height, format);

        file.write(reinterpret_cast<const char*>(&header), sizeof(TGAHeader));
        file.write(reinterpret_cast<const char*>(imageData), width*height*channels);
        file.close();
        
        return true;
    }
    else
    {
        return false;
    }
}
