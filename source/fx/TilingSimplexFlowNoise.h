#pragma once

// Tiling simplex noise from Stefan Gustavson
// 
// This code is a C++ adaption of the GLSL code from https://github.com/stegu/psrdnoise/blob/main/src/psrdnoise3-min.glsl.
// The original code by Stefan Gustavson is released under an MIT license:
// 
// psrdnoise (c) Stefan Gustavson and Ian McEwan,
// ver. 2021-12-02, published under the MIT license:
// https://github.com/stegu/psrdnoise/
namespace TilingSimplexFlowNoise
{
    float psrdnoise3( 
        float x, float y, float z,
        float period_x, float period_y, float period_z,
        float alpha,
        float& grad_x, float& grad_y, float& grad_z);
}
