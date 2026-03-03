#include <geometry/Penrose.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

Penrose::Penrose() { setDefaultGenerator(); }

void Penrose::reserve_faces(int n)
{
    // Reserve memory for face type attribute
    m_faceType.reserve(n);

    // Call super implementation
    SimpleGeometry::reserve_faces(n);
}

int Penrose::add_face(SimpleGeometry::Face f, int type)
{
    // Set face type
    m_faceType.push_back(type);
    // Call super implementation
    return SimpleGeometry::add_face(f);
}

int Penrose::add_face(SimpleGeometry::Face f)
{
    // Distinguish face type by thresholding angle at apex (= first vertex).
    // Note that we do not check for isoscele nor exact angles 36� / 108�.
    float *vp = get_vertex_ptr();
    vec3 AB = get_vertex(f.vi[1]) - get_vertex(f.vi[0]),
         AC = get_vertex(f.vi[2]) - get_vertex(f.vi[0]);
    float theta =
        std::acos(AB.scalarprod(AC) / (AB.magnitude() * AC.magnitude()));
    int type = (theta < 50.f) ? Red : Blue;

    return add_face(f, type);
}

void Penrose::add_face_subdivision(SimpleGeometry::Face f, int type, int levels)
{
    const double goldenRatio = (1. + sqrt(5.)) / 2.;

    if (levels == 0)
    {
        // insert face for real
        add_face(f, type);
        return;
    }

    // get vertices
    vec3 a = get_vertex(f.vi[0]), b = get_vertex(f.vi[1]),
         c = get_vertex(f.vi[2]);

    // get normals
    vec3 na = get_normal(f.vi[0]), nb = get_normal(f.vi[1]),
         nc = get_normal(f.vi[2]);

    if (type == Red)
    {
        // split into two triangles

        // new vertex p
        vec3 p = a + (b - a) / (float)goldenRatio;
        // new normal by interpolating normals at a and b
        vec3 np = na + (nb - na) / (float)goldenRatio;
        np.normalize();
        int pi = add_vertex_and_normal(p, np);
        // new faces
        Face pca(pi, f.vi[2], f.vi[0]);
        Face cpb(f.vi[2], pi, f.vi[1]);

        add_face_subdivision(pca, Blue, levels - 1);
        add_face_subdivision(cpb, Red, levels - 1);
    }
    else if (type == Blue)
    {
        // split into three triangles

        vec3 q = b + (a - b) / (float)goldenRatio,
             r = b + (c - b) / (float)goldenRatio;

        vec3 nq = nb + (na - nb) / (float)goldenRatio,
             nr = nb + (nc - nb) / (float)goldenRatio;
        nq.normalize();
        nr.normalize();

        int qi = add_vertex_and_normal(q, nq),
            ri = add_vertex_and_normal(r, nr);

        Face rca(ri, f.vi[2], f.vi[0]);
        Face qrb(qi, ri, f.vi[1]);
        Face rqa(ri, qi, f.vi[0]);

        add_face_subdivision(rca, Blue, levels - 1);
        add_face_subdivision(qrb, Blue, levels - 1);
        add_face_subdivision(rqa, Red, levels - 1);
    }
    else
    {
        // This should not happen!
    }
}

void Penrose::create(int levels)
{
    if (levels < 0)
        levels = m_levels;
    else
        m_levels = levels;

    // Copy generator vertices (FIXME: direct copy would be much faster!)
    for (int i = 0; i < m_generator.num_vertices(); i++)
        add_vertex_and_normal(m_generator.get_vertex(i),
                              m_generator.get_normal(i));

    // Start with all faces "Red"
    for (int i = 0; i < m_generator.num_faces(); i++)
        add_face_subdivision(m_generator.get_face(i), Red, levels);

    constexpr bool WheelGenerator = false;
    if constexpr (WheelGenerator)
    {
        // Origin
        add_vertex_and_normal(vec3(0, 0, 0), vec3(0, 0, 1));

        // Wheel around origin
        for (int i = 0; i < 10; i++)
        {
            double phi = (double)i * 2. * M_PI / 10.;
            add_vertex_and_normal(vec3(std::cos(phi), std::sin(phi), 0),
                                  vec3(0, 0, 1));
        }

        // Subdivide red triangles
        for (int i = 0; i < 10; i++)
        {
            int j = (i + 0) % 10 + 1, k = (i + 1) % 10 + 1;

            // Mirror every second triangle
            if (i % 2)
                add_face_subdivision(Face(0, j, k), Red, levels);
            else
                add_face_subdivision(Face(0, k, j), Red, levels);
        }
    }
}

void Penrose::setDefaultGenerator()
{
    clear();

    SimpleGeometry geom;

    // Origin
    geom.add_vertex_and_normal(vec3(0, 0, 0), vec3(0, 0, 1));

    // Wheel around origin
    for (int i = 0; i < 10; i++)
    {
        double phi = (double)i * 2. * M_PI / 10. + M_PI / 2.;
        geom.add_vertex_and_normal(vec3(static_cast<float>(std::cos(phi)),
                                        static_cast<float>(std::sin(phi)), 0.f),
                                   vec3(0.f, 0.f, 1.f));
    }

    // Subdivide red triangles
    for (int i = 0; i < 10; i++)
    {
        int j = (i + 0) % 10 + 1, k = (i + 1) % 10 + 1;

        // Mirror every second triangle
        if (i % 2)
            geom.add_face(Face(0, j, k));
        else
            geom.add_face(Face(0, k, j));
    }

    // Copy
    m_generator = geom;
}

void Penrose::setGenerator(const SimpleGeometry &geom)
{
    // Copy
    m_generator = geom;
}
