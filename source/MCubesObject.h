#pragma once

#include <mutex>
#include <vector>
#include <functional>

#include <fx/MarchingCubes.h>
#include <fx/PerlinNoise.h>

#include <glutils/MeshBuffer.h>
#include <glutils/GLMeshObject.h>
#include <glutils/GLError.h>
#include <utils/ComputeThreads.h>


struct MCubesObject : public MeshBuffer
{
    float fScale = 1/16.f;
    float fIsovalue = .5f;
    int iSizePot = 5;
    int nSlices = 1;

    float fPosX;
    float fPosY;
    float fPosZ;

    void compute(float scale, float iso, unsigned N, unsigned slice=0, unsigned nslices=1)
    {
        auto samplefun_sphere = [](float x,float y,float z,void*) 
        {
            return std::sqrt(x*x + y*y + z*z); 
        };

        auto samplefun_noise = [](float x,float y,float z,void* userdata) -> float
        {
            struct Params { float x0,y0,z0; };
            float x0 = 123.3456f;
            float y0 = 732.5489f;
            float z0 = 129.3983f;
            if(userdata)
            {
                Params& p = *(Params*)userdata;
                x0 = p.x0;
                y0 = p.y0;
                z0 = p.z0;
            }

            int octaves = 3;
            float perstistence = 0.75;
            float noise = PerlinNoise::fabsnoise( x+x0,y+y0,z+z0, octaves,perstistence );
            
            // center-sphere cut-out
            return fabs(noise) - (0.5f / (x*x + y*y + (z-1.f)*(z-1.f)));
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
                    MarchingCubes::triangulate( x, y, z, samplefun_noise, iso, scale,
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

    void compute(int slice=0)
    {
        compute(fScale,fIsovalue,2<<iSizePot,slice,nSlices);
    }

    bool update(float posx, float posy, float posz, float scale, float iso, int pow2, unsigned slice=0, unsigned nslices=1)
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

    bool create()
    {
        return true;
    }
};



#define MCUBES_PARALLEL

#ifdef MCUBES_PARALLEL_INSTANT_UPDATE
std::mutex g_mutex_set_dirty;
#endif

struct MCubesObjectRenderer // parallel compute & render
{
    ~MCubesObjectRenderer()
    {
        clear();
    }

    void clear()
    {
        if( computeThreadsPtr ) 
        {
            delete computeThreadsPtr;
            computeThreadsPtr = nullptr;
        }
        objects.clear();
        glmesh.clear();
        numObjects = 0;
    }

public:
    bool create( unsigned nslices=4 )
    {
        clear();
        
        numObjects = nslices;

        bool ok = true;
        if(ok)
        {
            objects.resize(nslices);
            for( unsigned i=0; i < numObjects; ++i ) 
            {
                objects[i] = std::make_shared<MCubesObject>();
            }

            glmesh.resize(nslices);
            for( unsigned i=0; i < numObjects; ++i ) 
            {
                glmesh[i].setMeshBuffer( objects[i] );
                if(!glmesh[i].prepare())
                {
                    ok = false;
                    break;
                }
            }
        }
#ifdef MCUBES_PARALLEL
        if(ok)
        {
            computeThreadsPtr = new ComputeThreads(numObjects,
                [this](int i)
                {
                    objects[i]->compute(i);

                 #ifdef MCUBES_PARALLEL_INSTANT_UPDATE
                    // Trigger instant update for each slice (will lead to flicker)
                    std::lock_guard<std::mutex> guard(g_mutex_set_dirty);
                    this->glmesh[i].setDirty();
                 #endif
                });
        }
#endif
        if(!ok)
            clear();
        return ok;
    }

    void update(float x,float y,float z, float scale,float iso,int pot)
    {
        static bool compute_launched = false;
        isComputing = computeThreadsPtr ? computeThreadsPtr->numDirty()>0 : false;
        
        if( isComputing )
            return;

        if( compute_launched && !isComputing )
        {
            // update all slices at once
            for( unsigned i=0; i < numObjects; ++i )
            {
                glmesh[i].setDirty();
                glmesh[i].prepare();
            }
            compute_launched = false;
        }

        bool recompute_needed = false;
        for( unsigned i=0; i < numObjects; ++i )
            recompute_needed |= objects[i]->update(x,y,z, scale, iso, pot, i, numObjects);

        if( recompute_needed )
        {
#ifdef MCUBES_PARALLEL
         #ifdef MCUBES_PARALLEL_INSTANT_UPDATE
            if( computeThreadsPtr->numDirty()==0 )
         #endif
            computeThreadsPtr->launchAll();
            compute_launched = true;
#else
            for( unsigned i=0; i < numObjects; ++i )
            {
                objects[i]->compute(i);
                glmesh[i].setDirty();
                glmesh[i].prepare();
            }
#endif
        }
    }

    void draw(int i)
    {
        if( i>=0 && i<(int)numObjects)
            glmesh[i].draw();
    }

    void draw()
    {
        for( unsigned i=0; i < numObjects; ++i )
            glmesh[i].draw();
    }
    
    std::vector<std::shared_ptr<MCubesObject>> objects;
    std::vector<GLMeshObject> glmesh;
    unsigned numObjects=0;
    ComputeThreads* computeThreadsPtr = nullptr;
    bool isComputing = false;
};
