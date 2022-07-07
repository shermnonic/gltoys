#include "MCubesObjectRenderer.h"
#include <utils/ComputeThreads.h>

#define MCUBES_PARALLEL // Comment out to disable parallel compute (for debugging purposes)
//#define MCUBES_PARALLEL_INSTANT_UPDATE // Uncomment to trigger instant update for each slice (will lead to flicker)

#ifdef MCUBES_PARALLEL_INSTANT_UPDATE
std::mutex g_mutex_set_dirty;
#endif

void MCubesObjectRenderer::clear()
{
    if(computeThreadsPtr)
    {
        delete computeThreadsPtr;
        computeThreadsPtr = nullptr;
    }
    objects.clear();
    glmesh.clear();
    numObjects = 0;
}

bool MCubesObjectRenderer::create(unsigned nslices)
{
    clear();

    numObjects = nslices;

    bool ok = true;
    if(ok)
    {
        objects.resize(nslices);
        for(unsigned i=0; i < numObjects; ++i)
        {
            objects[i] = std::make_shared<MCubesObject>();
        }

        glmesh.resize(nslices);
        for(unsigned i=0; i < numObjects; ++i)
        {
            glmesh[i].setMeshBuffer(objects[i]);
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

void MCubesObjectRenderer::update(float x,float y,float z,float scale,float iso,int pot)
{
    static bool compute_launched = false;
    isComputing = computeThreadsPtr ? computeThreadsPtr->numDirty()>0 : false;

    if(isComputing)
        return;

    if(compute_launched && !isComputing)
    {
        // update all slices at once
        for(unsigned i=0; i < numObjects; ++i)
        {
            glmesh[i].setDirty();
            glmesh[i].prepare();
        }
        compute_launched = false;
    }

    bool recompute_needed = false;
    for(unsigned i=0; i < numObjects; ++i)
        recompute_needed |= objects[i]->update(x,y,z,scale,iso,pot,i,numObjects);

    if(recompute_needed)
    {
#ifdef MCUBES_PARALLEL
  #ifdef MCUBES_PARALLEL_INSTANT_UPDATE
        if(computeThreadsPtr->numDirty()==0)
  #endif
            computeThreadsPtr->launchAll();
        compute_launched = true;
#else
        for(unsigned i=0; i < numObjects; ++i)
        {
            objects[i]->compute(i);
            glmesh[i].setDirty();
            glmesh[i].prepare();
        }
#endif
    }
}

void MCubesObjectRenderer::draw(int i)
{
    if(i>=0 && i<(int)numObjects)
        glmesh[i].draw();
}

void MCubesObjectRenderer::draw()
{
    for(unsigned i=0; i < numObjects; ++i)
        glmesh[i].draw();
}
