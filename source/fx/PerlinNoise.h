#pragma once

/// Perlin Noise 3D
/// Java reference implementation adapted from http://mrl.nyu.edu/~perlin/noise/.
/// Some functions adapted from GPUGems2 noise chapter.
/// Used value type float for comparison w/ GPU noise shader.
/// Max Hermann, August 7, 2010
namespace PerlinNoise
{
    /// 3D Perlin noise
    float noise( float x, float y, float z );
    
    /// Turbulence (same as fBm w/ abs)
    float turbulence( float x, float y, float z, 
                      int octaves, float lacunarity=2.0, float gain=0.5);

    /// Fractal sum (same as turbulence w/o abs)
    float fBm       ( float x, float y, float z, 
                      int octaves, float lacunarity=2.0, float gain=0.5 );

    /// Ridged multifractal
    /// See "Texturing & Modeling, A Procedural Approach", Chapter 12
    float ridgedmf  ( float x, float y, float z, 
                      int octaves, float lacunarity=2.0, float gain=0.5,
                      float offset=1.0 );
    
    float fabsnoise( float x, float y, float z,
                     int octaves, float persistance );
}
