#include "renderScreenshot.h"

#include <glutils/OffscreenRendering.h>
#include <utils/TGA.h>

#include <cmath>  // round
#include <ctime>
#include <filesystem>
#include <iostream>

void renderScreenshot(std::function<void(int, int)> const& renderFrame, int referenceWidth, int referenceHeight, std::string prefix)
{
    auto getScreenshotFilename = [=]()
    {
        return prefix + "." + std::to_string(std::time(0)) + ".tga";
    };

    auto scaleLongestEdgeMaxSize = [](int width, int height) -> std::array<int, 2>
    {
        constexpr int MaxSize = 8096;
        const double aspect = width /(double)height;
        if(aspect >= 1.0) 
        {
            return {MaxSize, static_cast<int>(std::round(MaxSize / aspect))};
        }
        else
        {
            return {static_cast<int>(std::round(MaxSize * aspect)), MaxSize};
        }
    };

    const auto [width, height] = scaleLongestEdgeMaxSize(referenceWidth, referenceHeight);

    OffscreenRendering renderer(width, height);
    renderer.initGL();
    renderer.renderBegin();

    renderFrame(width, height);

    renderer.renderEnd();
    renderer.getRawImageData();

    const auto filename = std::filesystem::path(getScreenshotFilename());
    
    std::cout << "Saving screenshot to " << std::filesystem::absolute(filename) << " ..." << std::endl;

    const auto format = renderer.getChannels()==4 ? TGAFormat::RGBA : TGAFormat::RGB;
    saveTGA(filename, format, renderer.getWidth(), renderer.getHeight(), renderer.getRawImageData());

    renderer.destroyGL();
}
