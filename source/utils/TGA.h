#pragma once
#include <cstdint>
#include <filesystem>

enum class TGAFormat
{
    RGB,
    RGBA
};

struct TGAHeader
{
    TGAHeader(size_t width, size_t height, TGAFormat format);
    
    uint8_t lengthIdentificationField = 0;          ///< 0 to omit
    uint8_t colorMapType = 0;                       ///< 0 for no color map included
    uint8_t imageTypeCode = 2;                      ///< 2 for unmapped rgb(a), 3 for b+w
    uint8_t colorMapSpecification[5] = {0,0,0,0,0}; ///< ignored
    uint8_t xOrigin[2] = {0,0};                     ///< (0,0) by default
    uint8_t yOrigin[2] = {0,0};                     ///< (0,0) by default
    uint8_t width [2];                              ///< filled by c'tor
    uint8_t height[2];                              ///< filled by c'tor
    uint8_t pixelSize;                              ///< 24 or 32, filled by c'tor
    uint8_t imageDescriptorByte = 0;                ///< 0
};

bool saveTGA(std::filesystem::path filename, TGAFormat format, size_t width, size_t height, const uint8_t* imageData);
