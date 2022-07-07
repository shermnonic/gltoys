#pragma once

#include "MCubesObject.h"
#include <glutils/GLMeshObject.h>
#include <glutils/GLError.h>
#include <vector>
#include <mutex>

class ComputeThreads;

/// Parallel compute & render
struct MCubesObjectRenderer
{
    ~MCubesObjectRenderer()
    {
        clear();
    }

    void clear();

    bool create(unsigned nslices=4);

    void update(float x,float y,float z,float scale,float iso,int pot);

    void draw(int i);
    void draw();
    
    std::vector<std::shared_ptr<MCubesObject>> objects;
    std::vector<GLMeshObject> glmesh;
    unsigned numObjects=0;
    ComputeThreads* computeThreadsPtr = nullptr;
    bool isComputing = false;
};
