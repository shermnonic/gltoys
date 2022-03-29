#include "MeshBufferIO.h"
#include "MeshBuffer.h"

void writeOBJ(std::ostream& os, const MeshBuffer& mb)
{
    const std::string endl = "\n";

    const size_t numVertices = mb.numVertices();
    for (size_t i = 0; i < numVertices; ++i)
    {
        const float* v = mb.getVertexData(i);
        os << "v " << v[0] << " " << v[1] << " " << v[2] << endl;
    }

    for (size_t i = 0; i < numVertices; ++i)
    {
        const float* n = mb.getNormalData(i);
        os << "n " << n[0] << " " << n[1] << " " << n[2] << endl;
    }

    const size_t m = mb.getNumVertsPerPrimitive();
    const size_t numIndices = mb.numIndices() / m;
    for (size_t i = 0; i < numIndices; ++i)
    {
        const unsigned* f = mb.getIndexData(i);
        os << "f";
        for (size_t j = 0; j < m; ++j)
            os << " " << f[j] + 1 << "//" << f[j] + 1; // OBJ indices are 1-based
        os << endl;
    }
}
