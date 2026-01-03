#pragma once

#include <glutils/MeshBuffer.h>

#include <array>

struct MCubesObject : public MeshBuffer
{
    enum class DensityFunction
    {
        Sphere,
        PerlinNoise,
        FractalBrownianMotion,
        TilingSimplexFlowNoise
    };
    DensityFunction densityFunction = DensityFunction::PerlinNoise;

    float fScale = 1/16.f;
    float fIsovalue = .5f;
    int iSizePot = 5;
    int nSlices = 1;

    struct Parameters
    {
        std::array<float, 3> position = { 0.f, 0.f, 0.f };

        float octaves = 3;
        float persistence = 0.75f; // only for DensityFunction::PerlinNoise
        float lacunarity = 2.f; // only for DensityFunction::FractalBrownianMotion
        float gain = 0.5f; // only for DensityFunction::FractalBrownianMotion

        bool operator == (Parameters const& other) const
        {
            return position[0] == other.position[0] 
                && position[1] == other.position[1]
                && position[2] == other.position[2]
                && octaves == other.octaves
                && persistence == other.persistence
                && lacunarity == other.lacunarity
                && gain == other.gain;
        };
    };

    Parameters parameters;

    void compute(float scale, float iso, unsigned N, unsigned slice=0, unsigned nslices=1);
    void compute(int slice=0);

    bool update(Parameters const& parameters, DensityFunction function, float scale, float iso, int pow2, unsigned slice=0, unsigned nslices=1);
    bool create();
};
