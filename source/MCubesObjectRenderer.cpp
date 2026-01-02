#include "MCubesObjectRenderer.h"
#include <utils/ComputeThreads.h>

constexpr bool ParallelComputation = true; // Set to false to disable parallel compute (for debugging purposes)
constexpr bool ParallelInstantUpdate = false; // Configure instant update for each slice (will lead to flicker)
std::mutex ParallelInstantUpdateMutex;

void MCubesObjectRenderer::clear()
{
    if(computeThreadsPtr)
    {
        delete computeThreadsPtr;
        computeThreadsPtr = nullptr;
    }
    objects.clear();
    meshes.clear();
    numObjects = 0;
}

bool MCubesObjectRenderer::create(unsigned nslices)
{
    clear();

    numObjects = nslices;

    objects.resize(nslices);
    meshes.resize(nslices);
    for(unsigned i=0; i < numObjects; ++i)
    {
        objects[i] = std::make_shared<MCubesObject>();
        meshes[i].setMeshBuffer(objects[i]);
        if(!meshes[i].prepare())
        {
            clear();
            return false;
        }
    }

    if constexpr (ParallelComputation)
    {
        computeThreadsPtr = new ComputeThreads(numObjects,
            [this](int i)
        {
            objects[i]->compute(i);

            if constexpr (ParallelInstantUpdate)
            {
                // Trigger instant update for each slice (will lead to flicker)
                std::lock_guard<std::mutex> guard(ParallelInstantUpdateMutex);
                this->meshes[i].setDirty();
            }
        });
    }

    return true;
}

void MCubesObjectRenderer::update(MCubesObject::Parameters const& parameters, MCubesObject::DensityFunction function, float scale, float iso, int pot)
{
    bool const is_computing = isComputing();

    if(is_computing)
        return;

    if(computeLaunched)
    {
        // update all slices at once
        for(auto& mesh : meshes)
        {
            mesh.setDirty();
            mesh.prepare();
        }
        computeLaunched = false;
    }

    bool recompute_needed = false;
    for(std::size_t i=0; auto object : this->objects)
    {
        if(object)
        {
            recompute_needed |= object->update(parameters, function, scale, iso, pot, i, numObjects);
        }
        i++;
    }

    if(recompute_needed)
    {
        recompute();
    }
}

void MCubesObjectRenderer::draw(int i)
{
    assert(i>=0 && i<(int)numObjects);
    meshes[i].draw();
}

void MCubesObjectRenderer::draw()
{
    for(auto& mesh : meshes)
        mesh.draw();
}

void MCubesObjectRenderer::recompute()
{
    if constexpr (ParallelComputation)
    {
        if constexpr (ParallelInstantUpdate)
        {
            if(computeThreadsPtr->numDirty()==0)
            {
                computeThreadsPtr->launchAll();
            }
        }
        else
        {
            computeThreadsPtr->launchAll();
        }
        computeLaunched = true;
    }
    else
    {
        for(std::size_t i=0; i < numObjects; ++i)
        {
            objects[i]->compute(i);
            meshes[i].setDirty();
            meshes[i].prepare();
        }
    }
}

bool MCubesObjectRenderer::isComputing() const
{
    return computeThreadsPtr ? computeThreadsPtr->numDirty()>0 : false;
}