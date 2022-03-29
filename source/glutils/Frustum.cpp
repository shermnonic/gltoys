// Frustum.cpp
// mnemonic 2004

#include "Frustum.h"
#include <math.h> // for: sqrt()

Frustum::Frustum()
{
}

Frustum::~Frustum()
{
}

// TODO: pq test of 2 extreme points rather than testing all 6 points!
int Frustum::clip_aabb( float aabb_min[3], float aabb_max[3] ) const
{
    // edgge-vertices of the cube represented by the aabb
    float cube[8][3];
    cube_from_aabb( aabb_min, aabb_max, cube );

    // test against every plane
    for( int i=0; i < 6; i++ )
    {
        if( distance( planes[i], cube[0] ) > 0 ) continue;
        if( distance( planes[i], cube[1] ) > 0 ) continue;
        if( distance( planes[i], cube[2] ) > 0 ) continue;
        if( distance( planes[i], cube[3] ) > 0 ) continue;
        if( distance( planes[i], cube[4] ) > 0 ) continue;
        if( distance( planes[i], cube[5] ) > 0 ) continue;
        if( distance( planes[i], cube[6] ) > 0 ) continue;
        if( distance( planes[i], cube[7] ) > 0 ) continue;

        // if we get here, it isn't in the Frustum
        return -1;
    }

    return 1;
}

void Frustum::normalize_plane( float p[4] )
{
    float mag = (float)sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2] );

    p[0] /= mag;
    p[1] /= mag;
    p[2] /= mag;
    p[3] /= mag;
}

inline float Frustum::distance( const float p[4], const float vec[3] )
{
    return p[0]*vec[0] + p[1]*vec[1] + p[2]*vec[2] + p[3];
}

// This code comes from gametutorials.com 
void Frustum::extract_frustum( float modl[16], float proj[16], bool normalize )
{
    float   clip[16]; // This will hold the clipping planes

    // Now that we have our modelview and projection matrix, if we combine these 2 matrices,
    // it will give us our clipping planes.  To combine 2 matrices, we multiply them.

    clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
    clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
    clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
    clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

    clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
    clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
    clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
    clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

    clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
    clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
    clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

    clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
    clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
    clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];
    
    // Now we actually want to get the sides of the Frustum.  To do this we take
    // the clipping planes we received above and extract the sides from them.

    // This will extract the RIGHT side of the Frustum
    planes[0][0] = clip[ 3] - clip[ 0];
    planes[0][1] = clip[ 7] - clip[ 4];
    planes[0][2] = clip[11] - clip[ 8];
    planes[0][3] = clip[15] - clip[12];

    // This will extract the LEFT side of the Frustum
    planes[1][0] = clip[ 3] + clip[ 0];
    planes[1][1] = clip[ 7] + clip[ 4];
    planes[1][2] = clip[11] + clip[ 8];
    planes[1][3] = clip[15] + clip[12];

    // This will extract the BOTTOM side of the Frustum
    planes[2][0] = clip[ 3] + clip[ 1];
    planes[2][1] = clip[ 7] + clip[ 5];
    planes[2][2] = clip[11] + clip[ 9];
    planes[2][3] = clip[15] + clip[13];

    // This will extract the TOP side of the Frustum
    planes[3][0] = clip[ 3] - clip[ 1];
    planes[3][1] = clip[ 7] - clip[ 5];
    planes[3][2] = clip[11] - clip[ 9];
    planes[3][3] = clip[15] - clip[13];

    // This will extract the BACK side of the Frustum
    planes[4][0] = clip[ 3] - clip[ 2];
    planes[4][1] = clip[ 7] - clip[ 6];
    planes[4][2] = clip[11] - clip[10];
    planes[4][3] = clip[15] - clip[14];

    // This will extract the FRONT side of the Frustum
    planes[5][0] = clip[ 3] + clip[ 2];
    planes[5][1] = clip[ 7] + clip[ 6];
    planes[5][2] = clip[11] + clip[10];
    planes[5][3] = clip[15] + clip[14];

    if( normalize )
    {
        for( int i=0; i < 6; i++ )
            normalize_plane( planes[i] );
    }
}

void Frustum::cube_from_aabb( float aabb_min[3], float aabb_max[3], float cube[8][3] )
{
    //
    //   3---------------2     edge 0 is aabb_min
    //   |\             /|     edge 6 is aabb_max
    //   |  \         /  |
    //   |   7-------6   |
    //   |   |       |   |
    //   |   |       |   |
    //   |   4-------5   |
    //   |  /         \  |
    //   |/             \|
    //   0---------------1
    //

    cube[0][0] = aabb_min[0];
    cube[0][1] = aabb_min[1];
    cube[0][2] = aabb_min[2];

    cube[1][0] = aabb_max[0];
    cube[1][1] = aabb_min[1];
    cube[1][2] = aabb_min[2];

    cube[2][0] = aabb_max[0];
    cube[2][1] = aabb_max[1];
    cube[2][2] = aabb_min[2];

    cube[3][0] = aabb_min[0];
    cube[3][1] = aabb_max[1];
    cube[3][2] = aabb_min[2];

    cube[4][0] = aabb_min[0];
    cube[4][1] = aabb_min[1];
    cube[4][2] = aabb_max[2];

    cube[5][0] = aabb_max[0];
    cube[5][1] = aabb_min[1];
    cube[5][2] = aabb_max[2];

    cube[6][0] = aabb_max[0];
    cube[6][1] = aabb_max[1];
    cube[6][2] = aabb_max[2];

    cube[7][0] = aabb_min[0];
    cube[7][1] = aabb_max[1];
    cube[7][2] = aabb_max[2];
}
