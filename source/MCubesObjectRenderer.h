#pragma once

#include "MCubesObject.h"
#include <glutils/GLMeshObject.h>
#include <glutils/GLError.h>
#include <vector>
#include <mutex>

class ComputeThreads;

/// Parallel compute & render
class MCubesObjectRenderer
{
public:
    ~MCubesObjectRenderer()
    {
        clear();
    }

    void clear();

    bool create(unsigned nslices=4);

    void update(MCubesObject::Parameters const& parameters, MCubesObject::DensityFunction function, float scale, float iso, int pot);

    void draw(int i);
    void draw();

    std::size_t getNumObjects() const
    {
        return numObjects;
    }

    std::shared_ptr<const MCubesObject> getObject(std::size_t i) const
    {
        return objects[i];
    }

    bool isComputing() const;

    float getTotalComputationTimeInMilliseconds() const;

private:
    void recompute();
    bool computeLaunched = false;

    std::vector<std::shared_ptr<MCubesObject>> objects;
    std::vector<GLMeshObject> meshes;
    std::size_t numObjects=0;
    ComputeThreads* computeThreadsPtr = nullptr;
};
