#pragma once

#include <glutils/MeshBuffer.h>

struct MCubesObject : public MeshBuffer
{
    float fScale = 1/16.f;
    float fIsovalue = .5f;
    int iSizePot = 5;
    int nSlices = 1;

    float fPosX;
    float fPosY;
    float fPosZ;

    void compute(float scale, float iso, unsigned N, unsigned slice=0, unsigned nslices=1);
    void compute(int slice=0);

    bool update(float posx, float posy, float posz, float scale, float iso, int pow2, unsigned slice=0, unsigned nslices=1);
    bool create();
};
