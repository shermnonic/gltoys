#pragma once

#include "Vector.h" // vec3
#include <map>
#include <set>
#include <vector>

/// Indexed face set in linear tightly packed vertex/index buffers
/// Restricted to triangle facets and vertex normals.
/// Internally getters map to more convenient custom vector/face type.
class SimpleGeometry
{
  public:
    SimpleGeometry() = default;
    virtual ~SimpleGeometry() = default;

    virtual void create(int param) {}

    virtual void setLevels(int levels) {}
    virtual int getLevels() const { return 0; }

    /// Return number of vertices/normals
    int num_vertices() const;
    /// Return number of faces
    int num_faces() const;

    ///@{ Get pointer to tightly packed buffer
    float *get_vertex_ptr();
    float *get_normal_ptr();
    unsigned *get_index_ptr();

    float const* get_vertex_ptr() const { return get_vertex_ptr(); };
    float const* get_normal_ptr() const { return get_normal_ptr(); };
    unsigned const* get_index_ptr() const { return get_index_ptr(); };
  ///@}

    void clear();

    bool writeOBJ(const char *filename) const;

    /// Return one-ring vertex indices for vertex i (expensive!)
    //void get_one_ring(int i, std::vector<int> &N) const;

    /// Create mesh dual, written as new geometry to res
    //void make_dual(SimpleGeometry &res) const;

    void recomputeVertexNormals();

    void unifyVertices();

  protected:
    void computeFaceNormalsAndAdjacency();


    /// Reserve memory for vertices
    void reserve_vertices(int n);
    /// Reserve memory for face indices
    virtual void reserve_faces(int n);

    /// Internal triangle face type
    /// Assume identical indices for vertices/normals
    struct Face
    {
        Face();
        Face(int i[3]);
        Face(int i, int j, int k);
        int vi[3];
    };

    const std::vector<float> &vbuffer() const { return m_vdata; };

  public:
    vec3 get_vertex(int i) const;
    vec3 get_normal(int i) const;
    Face get_face(int j) const;

    void set_vertex(int i, vec3 v);
    void set_normal(int i, vec3 n);

    /// Insert face
    virtual int add_face(Face f);

    /// Insert vertex with normal, returns vertex index
    /// (Yes, you *have* to supply a normal :-)
    int add_vertex_and_normal(vec3 v, vec3 n);

  private:
    std::vector<float> m_vdata;
    std::vector<float> m_ndata; // vertex normals
    std::vector<unsigned> m_fdata;

    std::vector<float> faceNormals;

    std::map<int, std::set<int>> facesAdjacentToVertex;
};
