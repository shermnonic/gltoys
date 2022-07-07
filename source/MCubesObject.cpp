#include "MCubesObject.h"

#include <fx/MarchingCubes.h>
#include <fx/PerlinNoise.h>
#include <fx/TilingSimplexFlowNoise.h>

#include <vector>
#include <functional>


void MCubesObject::compute(float scale, float iso, unsigned N, unsigned slice, unsigned nslices)
{
    struct Params
    {
        float x0 = 123.3456f;
        float y0 = 732.5489f;
        float z0 = 129.3983f;
    };

    auto samplefun_sphere = [](float x,float y,float z,void*) 
    {
        return std::sqrt(x*x + y*y + z*z); 
    };

    auto samplefun_noise = [](float x,float y,float z,void* userdata) -> float
    {
        Params p = userdata ? *(Params*)userdata : Params();

        int octaves = 3;
        float perstistence = 0.75;
        float noise = PerlinNoise::fabsnoise( x+p.x0,y+p.y0,z+p.z0, octaves,perstistence );
            
        // center-sphere cut-out
        return fabs(noise) - (0.5f / (x*x + y*y + (z-1.f)*(z-1.f)));
    };

    auto samplefun_psrdnoise = [](float x,float y,float z,void* userdata) -> float
    {
        Params p = userdata ? *(Params*)userdata : Params();
        float g[3];
        return TilingSimplexFlowNoise::psrdnoise3(x+p.x0,y+p.y0,z+p.z0, 11,11,11, 0, g[0],g[1],g[2]);
    };

    auto samplefun_psrdnoise_gradient = [](float x,float y,float z,float& grad_x,float& grad_y,float& grad_z, void* userdata)
    {
        Params p = userdata ? *(Params*)userdata : Params();
        TilingSimplexFlowNoise::psrdnoise3(x+p.x0,y+p.y0,z+p.z0, 11,11,11, 0, grad_x, grad_y, grad_z);
    };

    const size_t MAX_POINTS_PER_CUBE    = 12;
    const size_t MAX_TRIANGLES_PER_CUBE = 5;

    this->setNumVertices(0);
    this->setNumIndices(0);
    this->resize( N*N * MAX_POINTS_PER_CUBE    / nslices, 
                  N*N * MAX_TRIANGLES_PER_CUBE / nslices );

    unsigned total_num_triangles=0;
    unsigned total_num_points=0;
    unsigned index=0;
        
    unsigned zistep=N/nslices, zi0=slice*zistep, ziend=(slice+1)*zistep;
    for(unsigned zi=zi0; zi < ziend; ++zi)
        for(unsigned yi=0; yi < N; ++yi)
            for(unsigned xi=0; xi < N; ++xi)
            {
                assert( index==total_num_points );

                this->ensure( MAX_POINTS_PER_CUBE, MAX_TRIANGLES_PER_CUBE );

                unsigned num_triangles=0;
                unsigned num_points=0;

                float x = 2.f*(xi/float(N-1) - .5f) - scale*.5f;
                float y = 2.f*(yi/float(N-1) - .5f) - scale*.5f;
                float z = 2.f*(zi/float(N-1) - .5f) - scale*.5f;

                struct { float x0,y0,z0; } pos{ fPosX,fPosY,fPosZ };
                MarchingCubes::triangulate( x, y, z, 
                    //samplefun_psrdnoise, samplefun_psrdnoise_gradient,
                    samplefun_noise, nullptr, 
                    iso, scale,
                    this->getVertexData(index), 
                    this->getNormalData(index), 
                    this->getIndexData(total_num_triangles), index,
                    num_triangles, num_points, (void*)&pos);

                index += num_points;
                total_num_triangles += num_triangles;
                total_num_points += num_points;

                this->setNumVertices( total_num_points );
                this->setNumIndices( total_num_triangles*3 );
            }
}

void MCubesObject::compute(int slice)
{
    compute(fScale,fIsovalue,2<<iSizePot,slice,nSlices);
}

bool MCubesObject::update(float posx, float posy, float posz, float scale, float iso, int pow2, unsigned slice, unsigned nslices)
{
    pow2 = std::max(std::min(pow2,7),1);
    if(posx != fPosX || posy != fPosY || posz != fPosZ || scale != fScale || iso != fIsovalue || pow2 != iSizePot || nslices != nSlices)
    {
        fPosX = posx;
        fPosY = posy;
        fPosZ = posz;
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
