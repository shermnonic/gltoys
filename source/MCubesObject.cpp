#include "MCubesObject.h"

#include <fx/MarchingCubes.h>
#include <fx/PerlinNoise.h>
#include <fx/TilingSimplexFlowNoise.h>

#include <cmath>
#include <functional>
#include <tuple>
#include <vector>

namespace
{
    using Parameters = MCubesObject::Parameters;

    auto samplefun_sphere = [](float x,float y,float z,void*) 
    {
        return std::sqrt(x*x + y*y + z*z); 
    };

    auto samplefun_noise = [](float x,float y,float z,void* userdata) -> float
    {
        Parameters const p = userdata ? *(Parameters*)userdata : Parameters();

        float const noise = 
            PerlinNoise::fabsnoise( 
                x + p.position[0], y + p.position[1], z + p.position[2], 
                p.octaves, p.persistence );
            
        // center-sphere cut-out
        return fabs(noise) - (0.5f / (x*x + y*y + (z-1.f)*(z-1.f)));
    };

    auto samplefun_fbm = [](float x,float y,float z,void* userdata) -> float
    {
        Parameters const p = userdata ? *(Parameters*)userdata : Parameters();

        float const noise = 
            PerlinNoise::fBm( 
                x + p.position[0], y + p.position[1], z + p.position[2], 
                p.octaves, p.lacunarity, p.gain );
            
        // center-sphere cut-out
        return fabs(noise) - (0.5f / (x*x + y*y + (z-1.f)*(z-1.f)));
    };

    auto samplefun_psrdnoise = [](float x,float y,float z,void* userdata) -> float
    {
        Parameters const p = userdata ? *(Parameters*)userdata : Parameters();
        float g[3];
        return TilingSimplexFlowNoise::psrdnoise3(
            x + p.position[0], y + p.position[1], z + p.position[2], 
            11,11,11, 0, g[0],g[1],g[2]);
    };

    auto samplefun_psrdnoise_gradient = [](float x,float y,float z,float& grad_x,float& grad_y,float& grad_z, void* userdata)
    {
        Parameters const p = userdata ? *(Parameters*)userdata : Parameters();
        TilingSimplexFlowNoise::psrdnoise3(
            x + p.position[0], y + p.position[1], z + p.position[2], 
            11,11,11, 0, grad_x, grad_y, grad_z);
    };
}

namespace
{
    auto ceil_cast_division = [](std::size_t a, unsigned b)
    {
        //return a / b;
        return static_cast<std::size_t>(std::ceil(static_cast<double>(a) / static_cast<double>(b)));
    };
}

void MCubesObject::compute(float scale, float iso, unsigned N, unsigned slice, unsigned nslices)
{
    MarchingCubes::SampleFunc const sampleFunc = 
        (densityFunction == DensityFunction::Sphere) ? samplefun_sphere :
        (densityFunction == DensityFunction::PerlinNoise) ? samplefun_noise :
        (densityFunction == DensityFunction::FractalBrownianMotion) ? samplefun_fbm :
        //(densityFunction == DensityFunction::TilingSimplexFlowNoise) ? samplefun_psrdnoise :
        samplefun_noise;
    MarchingCubes::GradientFunc const gradientFunc = nullptr;

    const size_t MAX_POINTS_PER_CUBE    = 12;
    const size_t MAX_TRIANGLES_PER_CUBE = 5;

    auto const NN = N*N; // number of cubes in one z-plane
    auto const totalNumberOfCubes = NN*N;
    auto const numberOfCubesPerSlice = ceil_cast_division(totalNumberOfCubes, nslices);

    this->setNumVertices(0);
    this->setNumIndices(0);
    this->resize( numberOfCubesPerSlice * MAX_POINTS_PER_CUBE, 
                  numberOfCubesPerSlice * MAX_TRIANGLES_PER_CUBE );

    unsigned const cubeStartIndex = numberOfCubesPerSlice*slice;
    unsigned const cubeEndIndex = cubeStartIndex + numberOfCubesPerSlice;

    auto linearIndexToSub = [&](std::size_t linearIndex)
    {
        auto const zi = linearIndex / NN;
        auto const remainder = linearIndex % NN; // same as linearIndex - zi*NN
        return std::tuple{ remainder % N, remainder / N, zi };
        
    };

    unsigned total_num_triangles = 0;
    unsigned total_num_points = 0;
    unsigned index = 0;

    for(unsigned currentCubeIndex = cubeStartIndex; currentCubeIndex < cubeEndIndex; ++currentCubeIndex)
    {
        auto [xi, yi, zi] = linearIndexToSub(currentCubeIndex);

        assert( index==total_num_points );

        this->ensure( MAX_POINTS_PER_CUBE, MAX_TRIANGLES_PER_CUBE );

        unsigned num_triangles=0;
        unsigned num_points=0;

        float x = 2.f*(xi/float(N-1) - .5f) - scale*.5f;
        float y = 2.f*(yi/float(N-1) - .5f) - scale*.5f;
        float z = 2.f*(zi/float(N-1) - .5f) - scale*.5f;

        MarchingCubes::triangulate( x, y, z, 
            sampleFunc, gradientFunc, 
            iso, scale,
            this->getVertexData(index), 
            this->getNormalData(index), 
            this->getIndexData(total_num_triangles), index,
            num_triangles, num_points, (void*)&this->parameters);

        index += num_points;
        total_num_triangles += num_triangles;
        total_num_points += num_points;

        this->setNumVertices( total_num_points );
        this->setNumIndices( total_num_triangles*3 );
    }
}

void MCubesObject::compute(int slice)
{
    compute(fScale, fIsovalue, 2<<iSizePot, slice, nSlices);
}

bool MCubesObject::update(Parameters const& newParameters, DensityFunction function, float scale, float iso, int pow2, unsigned slice, unsigned nslices)
{
    pow2 = std::max(std::min(pow2,7),1);

    if(!(this->parameters == newParameters) 
        || function != densityFunction
        || scale != fScale || iso != fIsovalue || pow2 != iSizePot || nslices != nSlices)
    {
        parameters = newParameters;
        densityFunction = function;
        fScale = scale;
        fIsovalue = iso;
        iSizePot = pow2;
        nSlices = nslices;
        return true;
    }

    return false;
}

bool MCubesObject::create()
{
    return true;
}
