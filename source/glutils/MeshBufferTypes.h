#pragma once

#include <type_traits>

enum class MeshVertexAttribute : unsigned {
    Normal = 0x01,
    Color  = 0x04,
    UV     = 0x07
};

enum class MeshPrimitiveType : unsigned {
    Unknown   = 0x00,
    Lines     = 0x01, // == GL_LINES
    Triangles = 0x04, // == GL_TRIANGLES
    Quads     = 0x07  // == GL_QUADS
};

// https://softwareengineering.stackexchange.com/questions/194412/using-scoped-enums-for-bit-flags-in-c

inline MeshVertexAttribute operator | (MeshVertexAttribute lhs, MeshVertexAttribute rhs)
{
    using T = std::underlying_type_t<MeshVertexAttribute>;
    return static_cast<MeshVertexAttribute>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline MeshVertexAttribute& operator |= (MeshVertexAttribute& lhs, MeshVertexAttribute rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline bool has(MeshVertexAttribute attributes, MeshVertexAttribute a)
{
    return (static_cast<unsigned>(attributes) & static_cast<unsigned>(a)) > 0;
}
