#include <geometry/Icosahedron.h>

void Icosahedron::add_face_subdivision(Face f, int levels)
{
    if (levels == 0)
    {
        // insert face for real
        add_face(f);
        return;
    }

    // get vertices
    vec3 v1 = get_vertex(f.vi[0]), 
         v2 = get_vertex(f.vi[1]),
         v3 = get_vertex(f.vi[2]);

    // new vertices
    vec3 v12 = v1 + v2;
    vec3 v13 = v1 + v3;
    vec3 v23 = v2 + v3;

    // normalize such that the new vertices again lie on the unit sphere surface
    v12.normalize();
    v13.normalize();
    v23.normalize();

    float const foo = (float)m_platonicConstantZ;

    // corresponding indices
    int i1 = f.vi[0], i2 = f.vi[1], i3 = f.vi[2],
        i12 = add_vertex_and_normal(v12 * foo, v12),
        i13 = add_vertex_and_normal(v13 * foo, v13),
        i23 = add_vertex_and_normal(v23 * foo, v23);

    // 4 new faces (in same orientation)
    add_face_subdivision(Face(i1, i12, i13), levels - 1);
    add_face_subdivision(Face(i12, i2, i23), levels - 1);
    add_face_subdivision(Face(i13, i23, i3), levels - 1);
    add_face_subdivision(Face(i12, i23, i13), levels - 1);
}

void Icosahedron::create(int levels)
{
    clear();

    if (levels < 0)
        levels = m_levels;
    else
        m_levels = levels;

    float const tao =
        (float)m_platonicConstantX; // for true icosahedron use golden
                                    // ratio (1+sqrt(5))/2 = 1.61803399
    vec3 vdata[12] = {vec3(1, tao, 0),   vec3(-1, tao, 0),  vec3(1, -tao, 0),
                      vec3(-1, -tao, 0), vec3(0, 1, tao),   vec3(0, -1, tao),
                      vec3(0, 1, -tao),  vec3(0, -1, -tao), vec3(tao, 0, 1),
                      vec3(-tao, 0, 1),  vec3(tao, 0, -1),  vec3(-tao, 0, -1)};

    static int tindices[20][3] = { // ccw winding
        {0, 1, 4},  {1, 9, 4},  {4, 9, 5},  {5, 9, 3},  {2, 3, 7},
        {3, 2, 5},  {7, 10, 2}, {0, 8, 10}, {0, 4, 8},  {8, 2, 10},
        {8, 4, 5},  {8, 5, 2},  {1, 0, 6},  {11, 1, 6}, {3, 9, 11},
        {6, 10, 7}, {3, 11, 7}, {11, 6, 7}, {6, 0, 10}, {9, 1, 11}};

    // exact memory calulation
    int const n = (int)(20.0 * pow(4.0, (double)levels));
    reserve_vertices(n - 8);
    reserve_faces(n);

    // insert vertices
    for (int i = 0; i < 12; ++i)
    {
        // add normalized vertices lying on the surface of the unit sphere
        vec3 v = vdata[i];
        v.normalize();
        add_vertex_and_normal(v, v);
    }

    // insert faces (at the end of subdivision process)
    for (int i = 0; i < 20; ++i)
    {
        add_face_subdivision(Face(tindices[i]), levels);
    }

    
    unifyVertices();
}
