#pragma once

#include <glutils/MeshBuffer.h>
#include <vector>

class GlitchSphereGeometry : public MeshBuffer
{
public:
    struct SphereParams {
        std::vector<float> center = { 0.f, 0.f, 0.f };
        float radius = 1.f;
        int resolution = 46;
        SphereParams() {};
        SphereParams(float radius_, int resolution_) : radius(radius_), resolution(resolution_) {}
        SphereParams(float cx, float cy, float cz, float radius_, int resolution_) : center({ cx, cy, cz }), radius(radius_), resolution(resolution_) {}
    };

    struct GlitchParams {
        float t = 0.f;
        float lambda = 1.f;
        int colormap = 0;
    };

    GlitchSphereGeometry( MeshVertexAttribute attributes = MeshVertexAttribute::Normal | MeshVertexAttribute::Color | MeshVertexAttribute::UV )
    : MeshBuffer( MeshPrimitiveType::Triangles, attributes )
    {}
    
    void createSphereGeometryWithGlitch( float cx, float cy, float cz, float radius, int resolution, float t, float lambda, int colormap=0);

    void createSphereGeometryWithGlitch(const SphereParams& sphereParams, const GlitchParams& glitchParams);

    void setColormap(int colormap);
    
    SphereParams m_sphereParams;
    GlitchParams m_glitchParams;
};
