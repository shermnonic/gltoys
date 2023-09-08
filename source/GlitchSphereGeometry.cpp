#include "GlitchSphereGeometry.h"

#include <cmath> // sin, cos, sqrt

void GlitchSphereGeometry::createSphereGeometryWithGlitch(const SphereParams& sphereParams, const GlitchParams& glitchParams)
{
    createSphereGeometryWithGlitch(sphereParams.center[0], sphereParams.center[1], sphereParams.center[2], sphereParams.radius, sphereParams.resolution, glitchParams.t, glitchParams.lambda, glitchParams.colormap);
}

void GlitchSphereGeometry::createSphereGeometryWithGlitch( float cx, float cy, float cz, float radius, int resolution, float t, float lambda, int colormap )
{
    if(resolution < 0)
    {
        resolution = static_cast<int>(m_cache.resolution);
    }

    if(resolution==0)
    {
        // TODO: Deal gracefully with 0 value, and probably also other too small values.
    }

    m_sphereParams = SphereParams(cx,cy,cz,radius,resolution);
    m_glitchParams = GlitchParams{t,lambda,colormap};
 
    const bool hasResolutionChanged = resolution != m_cache.resolution;
    
    if(hasResolutionChanged)
    {
        m_cache.update(static_cast<size_t>(resolution));
    }
 
    const size_t n = m_cache.resolution;
    const float dpi = m_cache.deltaPi;
    
    const size_t num_rows = n/2;
    const size_t num_cols = n;
    const size_t num_verts = 2*num_rows * num_cols;
    const size_t num_tris = num_rows*num_cols*2;
    
    if(hasResolutionChanged)
    {
        MeshBuffer::resize( num_verts, num_tris );
    }

    size_t index_count = 0;
    size_t vertex_count = 0;
    for(size_t row=0; row < num_rows; ++row)
    {
        const size_t row_index = row * 2 * num_cols;

        // two circles for upper/lower latitude of current row
        for(size_t r=0; r < 2; ++r)
        {
            const size_t thetaIndex = row + r;

            // glitch modulation f1
            const float theta = thetaIndex * dpi;
            const float f1 = (1.f-lambda) + lambda*(float)std::sin(theta+t);

            const float cos_theta = m_cache.cosTable[thetaIndex];
            const float sin_theta = m_cache.sinTable[thetaIndex];

            for(size_t col=0; col < num_cols; ++col)
            {
                const size_t phiIndex = col;

                // glitch modulation f2
                const float phi = phiIndex * dpi;
                const float f2 = (1.f-lambda) + lambda*(float)std::sin(phi+t);

                const float cos_phi = m_cache.cosTable[phiIndex];
                const float sin_phi = m_cache.sinTable[phiIndex];

                const size_t vi = row_index + r*num_cols + col;

                const float f[3] = {
                    sin_theta * cos_phi * (r == 0 ? f1 : f2),
                    sin_theta * sin_phi * (r == 0 ? f2 : f1),
                    cos_theta
                };
                
                float* vp = MeshBuffer::getVertexData(vi);
                vp[0] = cx + radius*f[0];
                vp[2] = cz + radius*f[1];
                vp[1] = cy + radius*f[2];

                if (this->hasNormals())
                {
                    float* np = MeshBuffer::getNormalData(vi);
                    if (lambda > 0.f)
                    {
                        float length = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
                        np[0] = f[0] / length;
                        np[1] = f[1] / length;
                        np[2] = f[2] / length;
                    }
                    else
                    {
                        np[0] = f[0];
                        np[1] = f[1];
                        np[2] = f[2];
                    }
                }

                if (this->hasUVs())
                {
                    float* uv = MeshBuffer::getUVData(vi);
                    uv[0] = col / (float)num_cols;
                    uv[1] = 2 * (row + 1) / (float)num_cols;
                }

                vertex_count++;
            }
        }
        
        if(hasResolutionChanged)
        {
            // connectivity
            unsigned* indices = MeshBuffer::getIndexData();
            const size_t triangle_vertex_order[6] = {0,2,1, 3,2,0}; //{0,1,2, 2,3,0};
            for(size_t col=0; col < num_cols; ++col)
            {
                unsigned i0 = static_cast<unsigned>(row_index + col);
                unsigned i1 = static_cast<unsigned>(row_index + ((col+1)%num_cols)); // connect last to first latitudal vertex

                // 0 -- 3
                // | \  |
                // |  \ |
                // 1 -- 2
            
                // slpit quad into 2 triangles
                const size_t quad_indices[4] = {i0, i0+num_cols, i1+num_cols, i1};
                for(auto i : triangle_vertex_order)
                    indices[index_count++] = static_cast<unsigned>(quad_indices[i]);
            }
        }
    }

    if(hasResolutionChanged)
    {
        MeshBuffer::setNumVertices(vertex_count);
        MeshBuffer::setNumIndices(index_count);
        assert(vertex_count == num_verts);
        assert(index_count == num_tris*3);
    }

    if(hasResolutionChanged || colormap != m_lastColormap)
    {
        setColormap(colormap);
    }
}

void GlitchSphereGeometry::setColormap(int colormap)
{
    if (!this->hasColors())
        return;

    if (colormap < 0)
        return;

    const size_t n = m_sphereParams.resolution;
    const size_t num_rows = n / 2;
    const size_t num_cols = n;

    for (size_t row = 0; row < num_rows; ++row)
    {
        const size_t row_index = row * 2 * num_cols;

        // two circles for upper/lower latitude of current row
        for (size_t r = 0; r < 2; ++r)
        {
            for (size_t col = 0; col < num_cols; ++col)
            {
                const size_t vi = row_index + r * num_cols + col;

                float* cp = MeshBuffer::getColorData(vi);

                auto setColor = [&cp](double r, double g, double b, double a)
                {
                    cp[0] = static_cast<float>(r);
                    cp[1] = static_cast<float>(g);
                    cp[2] = static_cast<float>(b);
                    cp[3] = static_cast<float>(a);
                };

                int type = 2;
                int i = (int)col;
                switch (colormap) {
                default:
                case 0: setColor(1., 1., 1., 1.); break;
                case 1: setColor(0.1 * (i % 5), 0.1 * (i % 5), 0.6 * (i % 5), 0.1); break;
                case 2: setColor((float)0.5 * (i % 3), 0.3 * (i % 3), 0.1 * (i % 3), 0.1); break;
                case 3: setColor((float)0.5 * (i % 2), 0.5 * (i % 2), 0.5 * (i % 2), 0.4); break;
                case 4: setColor((i % 4) * (i % 9) / (4 * 9) * 0.9, (i % 2) * (i % 3) / (2 * 3) * 0.9, (i % 7) * (i % 11) / (7 * 11) * 0.8, 0.1); break;
                case 5: setColor(i % 2, (float)(i % 8 + 1) / 0.8f, (float)(i % 4) / 2.f, 0.02);  break;
                case 6: setColor((float)(i % 4) * (i % 9) / (4 * 9) * 0.9, (float)(i % 2) * (i % 3) / (2 * 3) * 0.9, (float)(i % 7) * (i % 11) / (7 * 11) * 0.8, 0.1); break;
                case 7: setColor((float)0.5 * (i % 3), 0.3 * (i % 3), 0.1 * (i % 3), 0.1); break;
                case 8: setColor((float)0.5 * (i % 3), 0.3 * (i % 3), 0.1 * (i % 3), 0.5); break;
                case 9: setColor(0.2 * (i % 7 + 3), 0.2 * (i % 3), 0.1 * (i % 4), 0.17); break;
                case 10: setColor(0.2 * (i % 5), 0.2 * (i % 5), 0.2 * (i % 5), 0.1); break;
                };
            }
        }
    }
}

inline void GlitchSphereGeometry::Cache::update(size_t n)
{
    if(this->resolution != n)
    {
        this->resolution = n;
        constexpr float PI = 3.1415926f;
        constexpr float TWOPI = 2 * PI;

        const float dpi = (float)TWOPI/n;
        this->deltaPi = dpi;

        cosTable.resize(n+1);
        sinTable.resize(n+1);
        for(size_t i=0; i < n+1; ++i)
        {
            float alpha = i * dpi;
            cosTable[i] = (float)std::cos(alpha);
            sinTable[i] = (float)std::sin(alpha); // too lazy
        }
    }
}
