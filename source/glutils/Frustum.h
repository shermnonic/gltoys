#pragma once
// Frustum.h
// mnemonic 2004

class Frustum
{
public:
    Frustum();
    ~Frustum();

    /// Extract viewing frustum from current MODELVIEW and PROJECTION matrix
    void extract_frustum( float modl[16], float proj[16], bool normalize=true );

    /// Test if axis-aligned boundingbox is inside frustum
    /// (x,y,z) is the center of the aabb and size its half diameter
    /// Returns
    ///   -1  if outside 
    ///    0  if inside
    ///    1  if intersecting
    int clip_aabb( float aabb_min[3], float aabb_max[3] ) const;

private:
    // 6-sided viewing frustum
    // a plane is represented by the equation
    // ax + by + cz + d = 0
    float planes[6][4];

    static inline void cube_from_aabb(float aabb_min[3], float aabb_max[3], float cube[8][3]);
    static inline float distance( const float p[4], const float vec[3] );
    static inline void normalize_plane( float p[4] );
};
