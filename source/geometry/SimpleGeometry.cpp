#include <geometry/SimpleGeometry.h>

#include <cassert>
#include <cstdio> // fprintf, fopen, FILE

SimpleGeometry::Face::Face() { vi[0] = vi[1] = vi[2] = 0; }

SimpleGeometry::Face::Face(int i[3])
{
    vi[0] = i[0];
    vi[1] = i[1];
    vi[2] = i[2];
}

SimpleGeometry::Face::Face(int i, int j, int k)
{
    vi[0] = i;
    vi[1] = j;
    vi[2] = k;
}

void SimpleGeometry::clear()
{
    m_vdata.clear();
    m_ndata.clear();
    m_fdata.clear();
}

void SimpleGeometry::reserve_vertices(int n)
{
    m_vdata.reserve(3 * (n - 8));
    m_ndata.reserve(3 * (n - 8));
}

void SimpleGeometry::reserve_faces(int n) { m_fdata.reserve(3 * n); }

int SimpleGeometry::num_vertices() const { return (int)m_vdata.size() / 3; }

int SimpleGeometry::num_faces() const { return (int)m_fdata.size() / 3; }

void SimpleGeometry::set_vertex(int i, vec3 v)
{
    assert(i < m_vdata.size() / 3);
    m_vdata[i * 3] = v.x;
    m_vdata[i * 3 + 1] = v.y;
    m_vdata[i * 3 + 2] = v.z;
}

void SimpleGeometry::set_normal(int i, vec3 n)
{
    assert(i < m_ndata.size() / 3);
    m_ndata[i * 3] = n.x;
    m_ndata[i * 3 + 1] = n.y;
    m_ndata[i * 3 + 2] = n.z;
}

vec3 SimpleGeometry::get_vertex(int i) const
{
    return vec3(m_vdata[i * 3], m_vdata[i * 3 + 1], m_vdata[i * 3 + 2]);
}

vec3 SimpleGeometry::get_normal(int i) const
{
    return vec3(m_ndata[i * 3], m_ndata[i * 3 + 1], m_ndata[i * 3 + 2]);
}

SimpleGeometry::Face SimpleGeometry::get_face(int i) const
{
    return Face(m_fdata[i * 3 + 0], m_fdata[i * 3 + 1], m_fdata[i * 3 + 2]);
}

int SimpleGeometry::add_face(Face f)
{
    for (int i = 0; i < 3; ++i)
        m_fdata.push_back(f.vi[i]);
    assert(m_fdata.size() % 3 == 0);
    return (int)m_fdata.size() / 3 - 1;
}

int SimpleGeometry::add_vertex_and_normal(vec3 v, vec3 n)
{
    for (int i = 0; i < 3; ++i)
    {
        m_vdata.push_back(v[i]);
        m_ndata.push_back(n[i]);
    }
    assert(m_vdata.size() == m_ndata.size());
    assert(m_vdata.size() % 3 == 0);
    return (int)m_vdata.size() / 3 - 1;
}

float *SimpleGeometry::get_vertex_ptr() { return &m_vdata[0]; }

float *SimpleGeometry::get_normal_ptr() { return &m_ndata[0]; }

unsigned *SimpleGeometry::get_index_ptr() { return &m_fdata[0]; }

bool SimpleGeometry::writeOBJ(const char *filename) const
{
    std::FILE *f = std::fopen(filename, "w");
    if (!f)
    {
        std::printf("Could not write OBJ '%s'!", filename);
        return false;
    }

    size_t nv = m_vdata.size() / 3;
    bool has_normals = m_ndata.size() == m_vdata.size();
    for (size_t i = 0; i < nv; ++i)
    {
        std::fprintf(f, "v %12.11f %12.11f %12.11f \n", m_vdata[3 * i + 0],
                     m_vdata[3 * i + 1], m_vdata[3 * i + 2]);

        if (has_normals)
            std::fprintf(f, "vn %12.11f %12.11f %12.11f \n", m_ndata[3 * i + 0],
                         m_ndata[3 * i + 1], m_ndata[3 * i + 2]);
    }

    size_t nf = m_fdata.size() / 3;
    for (size_t i = 0; i < nf; ++i)
    {
        // Note that face indices are 1-based in OBJ!
        std::fprintf(f, "f %4d %4d %4d \n", m_fdata[3 * i + 0] + 1,
                     m_fdata[3 * i + 1] + 1, m_fdata[3 * i + 2] + 1);
    }

    fclose(f);
    return true;
}
