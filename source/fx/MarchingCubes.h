#pragma once

namespace MarchingCubes
{
typedef float (*SampleFunc)( float x, float y, float z, void* userdata );
    
/// Triangulate isosurface inside a cube of a density function via the 
/// marching cubes algorithm, with cube edge length \a scale.
/// Buffers are pre-allocated for storage of up to 5 triangles and 12 points.
/// Normal pointer is optional.
void triangulate( float x, float y, float z, SampleFunc sample, float isovalue, float scale,
                  float* points, float* normals, unsigned* indices, unsigned start_index,
                  unsigned& num_triangles, unsigned& num_points, void* userdata=nullptr );
}
