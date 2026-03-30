#include <geometry/SimpleGeometry.h>

#include <algorithm>
#include <cassert>
#include <cstdio> // fprintf, fopen, FILE
#include <set>
#include <stdexcept>

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

void SimpleGeometry::updateFacesAdjacency()
{
    facesAdjacentToVertex.clear();
    for(int fi=0; fi < num_faces(); ++fi)
    {
        Face const face = get_face(fi);
        facesAdjacentToVertex[face.vi[0]].insert(fi);
        facesAdjacentToVertex[face.vi[1]].insert(fi);
        facesAdjacentToVertex[face.vi[2]].insert(fi);
    }
}

void SimpleGeometry::computeFaceNormals()
{
    if(facesAdjacentToVertex.size() != num_vertices())
    {
        updateFacesAdjacency();
    }

    faceNormals.resize(num_faces()*3);

    auto setFaceNormal = [this](int fi, vec3 const& n)
    {
        auto i = 3*fi;
        faceNormals[i+0] = n.x;
        faceNormals[i+1] = n.y;
        faceNormals[i+2] = n.z;
    };

    auto cross = [](vec3 const& a, vec3 const& b)
    {
        return a.cross(b);
    };

    for(int fi=0; fi < num_faces(); ++fi)
    {
        Face const face = get_face(fi);

        // Assume consistent orientation of face indices
        vec3 v0 = get_vertex(face.vi[0]);
        vec3 v1 = get_vertex(face.vi[1]);
        vec3 v2 = get_vertex(face.vi[2]);
        setFaceNormal(fi, cross(v1 - v0, v2 - v0).normalized());
    }
}

void SimpleGeometry::recomputeVertexNormals()
{
    computeFaceNormals();

    auto getFaceNormal = [this](int fi) -> vec3
    {
        auto const i = 3*fi;
        return { faceNormals[i], faceNormals[i+1], faceNormals[i+2] };
    };

    for(int vi=0; vi < num_vertices(); ++vi)
    {
        if(facesAdjacentToVertex.contains(vi))
        {
            vec3 n(0,0,0);

            for(int fi : facesAdjacentToVertex[vi])
            {
                n += getFaceNormal(fi);
            }
            n.normalize();

            set_normal(vi, n);
        }
        else
        {
            throw std::logic_error("Encountered singular vertex");
        }
    }
}

class Matrix
{
public:
    Matrix(std::size_t rows, std::size_t cols)
    : rows(rows), cols(cols), data(new float[rows*cols]) {}
    ~Matrix() { delete [] data; }
    float& operator () (std::size_t row, std::size_t col) 
    { return data[row*cols+col]; }
    float const& operator () (std::size_t row, std::size_t col) const 
    { return data[row*cols+col]; }
private:
    std::size_t rows = 0;
    std::size_t cols = 0;
    float* data = nullptr;
};

void SimpleGeometry::unifyVertices()
{
    std::size_t n = static_cast<std::size_t>(num_vertices());
    Matrix D(n, n);
    for(auto i=0; i < n; ++i)
    {
        D(i,i) = 0.f;

        for(auto j=i+1; j < n; ++j)
        {
            D(i,j) = (get_vertex(j) - get_vertex(i)).magnitude();
            D(j,i) = D(i,j);
        }
    }
    
    float const threshold = 1e-5f;

    std::vector<std::size_t> mapSameVertexToSmallestIndex(n);
    for(auto i=0; i < n; ++i) mapSameVertexToSmallestIndex[i] = i;

    for(auto i=0; i < n; ++i)
    {
        for(auto j=0; j < n; ++j)
        {
            if(i!=j)
            {
                if(D(i,j) < threshold)
                {
                    mapSameVertexToSmallestIndex[j] = std::min<std::size_t>(i, mapSameVertexToSmallestIndex[j]);
                }
            }
        }
    }

    SimpleGeometry unifiedGeometry;
    std::vector<std::size_t> newVertexIndex(n);
    for(auto i=0; i < n; ++i)
    {
        if(mapSameVertexToSmallestIndex[i] == i)
        {
            newVertexIndex[i] = unifiedGeometry.add_vertex_and_normal(get_vertex(i), get_normal(i));
        }
        else if(mapSameVertexToSmallestIndex[i] < i)
        {
            newVertexIndex[i] = newVertexIndex[mapSameVertexToSmallestIndex[i]];
        }
        else
        {
            throw std::logic_error("Unifying vertices failed");
        }
    }

    // Update faces
    std::size_t degeneratedTriangles = 0;
    int highestVertexIndex = -1;
    for(auto fi=0; fi < num_faces(); ++fi)
    {
        auto const f = get_face(fi);
        Face f_new( newVertexIndex[f.vi[0]], newVertexIndex[f.vi[1]], newVertexIndex[f.vi[2]] );
        if(f_new.vi[0] == f_new.vi[1] || f_new.vi[0] == f_new.vi[2] || f_new.vi[1] == f_new.vi[2])
        {
            degeneratedTriangles++;
        }
        else
        {
            unifiedGeometry.add_face(f_new);
        }

        highestVertexIndex = std::max(std::max(f_new.vi[0], f_new.vi[1]), f_new.vi[2]);
    }

    if(highestVertexIndex > unifiedGeometry.num_vertices())
    {
        std::cout << "Highest new vertex index " << highestVertexIndex << " is larger than number of unified vertices " << unifiedGeometry.num_vertices() << "\n";
        throw std::logic_error("Unifying faces failed");
    }
    

    if(degeneratedTriangles>0)
    {
        std::cout << "Removed " << degeneratedTriangles << " degenerated triangles\n";
    }
    
    newVertexIndex.clear();

    if(unifiedGeometry.num_vertices() < num_vertices() && unifiedGeometry.num_faces() <= num_faces())
    {
        std::cout << "Unified " << num_vertices() << " vertices to " << unifiedGeometry.num_vertices() << "\n";
        std::cout << "Unified " << num_faces() << " faces to " << unifiedGeometry.num_faces() << "\n";

        m_vdata = std::move(unifiedGeometry.m_vdata);
        m_ndata = std::move(unifiedGeometry.m_ndata);
        m_fdata = std::move(unifiedGeometry.m_fdata);
    }
    else
    {
        std::cout << "No vertices to unify\n";
    }    
}

std::vector<int> SimpleGeometry::computeOneRing(int i) const
{
    assert(facesAdjacentToVertex.size() == num_vertices());

    if(!facesAdjacentToVertex.contains(i))
    {
        return {};
    }

    // Get unique vertices of faces adjacent to center vertex v_i
    std::set<int> uniqueRingVertices;
    for (auto const adjacentFaceIndex : facesAdjacentToVertex.at(i))
    {
        Face const f = get_face(adjacentFaceIndex);
        
        assert(f.vi[0] == i || f.vi[1] == i || f.vi[2] == i);
        
        // Avoid checking for index i, just add it here multiple times and remove later
        uniqueRingVertices.insert(f.vi[0]);
        uniqueRingVertices.insert(f.vi[1]);
        uniqueRingVertices.insert(f.vi[2]);
    }
    uniqueRingVertices.erase(i); // Remove index i again

    // Order vertices according to angles in normal plane of vi
    std::vector<int> ringVertices( uniqueRingVertices.begin(), uniqueRingVertices.end() );

    vec3 const vi = get_vertex(i);
    vec3 const ni = get_normal(i);

    // Project into normal plane at vertex i
    auto project = [&vi, &ni](vec3 const& vj)
    {
        vec3 ej = vj - vi;
        ej -= ni * ej.scalarprod(ni);
        return ej.normalized();
    };
    
    // Measure angle against first projected edge
    vec3 const e0n = project(get_vertex(ringVertices[0]));
    vec3 const e0n_cross_ni = e0n.cross(ni);

    // @todo: With C++23 use zip view to simplify and make IndexAngle obsolete
    struct IndexAngle
    {
        int index;
        float angle;
    };

    std::vector<IndexAngle> vangle(ringVertices.size());
    vangle[0] = { ringVertices[0], 0.f };
    for (int k = 1; k < ringVertices.size(); ++k)
    {
        vec3 const ejn = project(get_vertex(ringVertices[k]));

        float angle = std::acos(ejn.scalarprod(e0n));
        float const sign = std::signbit(e0n_cross_ni.scalarprod(ejn));
        if (sign < 0.f)
        {
            angle = (float)M_PI - angle;
        }

        vangle[k] = { ringVertices[k], angle };
    }

    std::ranges::sort(vangle, [](auto const& a, auto const& b) { return a.angle < b.angle; });

    for(auto k=0; k < ringVertices.size(); ++k)
    {
        ringVertices[k] = vangle[k].index;
    }

    return ringVertices;
}

void SimpleGeometry::makeDual()
{
    if(facesAdjacentToVertex.size() != num_vertices())
    {
        updateFacesAdjacency();
    }

    SimpleGeometry result;

    int nverts = num_vertices();
    for (auto i0 = 0; i0 < nverts; ++i0)
    {
        // Compute dual to one-ring
        auto const N = computeOneRing(i0);

        vec3 const v0 = get_vertex(i0);
        vec3 const n0 = get_normal(i0);

        // New vertices
        std::vector<int> newIndices;
        newIndices.push_back(result.add_vertex_and_normal(v0, n0));

        constexpr bool SanityCheckJustCopyOneRing = true;

        if constexpr (SanityCheckJustCopyOneRing)
        {
            for (size_t i = 0; i < N.size(); ++i)
                newIndices.push_back(
                    result.add_vertex_and_normal(get_vertex(N[i]), n0));
        }
        else
        {
            for (size_t i = 0; i < N.size() - 1; ++i)
            {
                vec3 const vi = get_vertex(N[i]);
                vec3 const vj = get_vertex(N[i + 1]);
                newIndices.push_back(
                    result.add_vertex_and_normal((v0 + vi + vj) / 3.f, n0));
            }
        }

        // New faces
        for (size_t i = 1; i < newIndices.size() - 1; ++i)
            result.add_face(
                Face(newIndices[0], newIndices[i], newIndices[i + 1]));
    }

    result.recomputeVertexNormals();

    *this = result;
}
