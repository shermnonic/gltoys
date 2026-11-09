#pragma once

#include <functional>
#include <string>

/// Render high-resolution screenshot and save as TGA file with given prefix and epoch timestamp to disk.
/// Uses a fixed resolution of 8096 for the longer reference size and keeps the aspect ratio.
void renderScreenshot(std::function<void(int, int)> const& renderFrame, int referenceWidth, int referenceHeight, std::string prefix);