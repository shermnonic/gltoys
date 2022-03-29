#pragma once

#include <cassert>
#include "Frustum.h"

// Octrees can be done efficiently pointerless, e.g. see
// Lewiner et al. 2010, Fast generation of pointerless octree 
// https://www.cs.jhu.edu/~misha/ReadingSeminar/Papers/Lewiner10.pdf

/// Octree for frustum culling, recursion of axis aligned bounding boxes
template<class Vector3>
class Octree
{
public:
    struct AABB
    {
        Vector3 minCoord, maxCoord;
    };

    typedef std::stack<AABB> Leaves;

    Octree( AABB aabb_ ) 
    : aabb(aabb_)
    {}

    ~Octree()
    {
        for( int i=0; i < 8; i++ )
            if( sons[i] ) { delete sons[i]; sons[i]=nullptr; }
    }
   
    /// Return octree leaves inside the given viewing frustum
    void getVisibleLeaves( const Frustum* frustum, Leaves& leaves ) const
    {
        if( frustum )
        {
            int vis = frustum->clip_aabb( &aabb.minCoord[0], &aabb.maxCoord[0] );
            if( vis < 0 ) return; // node is outside of viewing frustum
        }
        
        if( this->isLeaf() )
        {
            leaves.push( aabb );
        }
        else
        {
            for( int i=0; i < 8; i++ ) if( sons[i] )
            {
                sons[i]->getVisibleLeaves( frust, leaves );
            }
        }
    }

    /// Create complete subtree from given level down to leaves at level zero
    void buildComplete( int level )
    {
        assert(level > 0);
        
        Vector3 d = (aabb.maxCoord - aabb.minCoord) * .5f;
        Vector3 center = aabb_min + d;
            
        sons[0] = new Octree({aabb_min, center});
        sons[1] = new Octree({aabb_min + Vector3( d[0],   0,  0 ) , center + Vector3( d[0],   0,   0 )});
        sons[2] = new Octree({aabb_min + Vector3( d[0],d[1],  0 ) , center + Vector3( d[0],d[1],   0 )});
        sons[3] = new Octree({aabb_min + Vector3(    0,d[1],  0 ) , center + Vector3(    0,d[1],   0 )});
        sons[4] = new Octree({aabb_min + Vector3(    0,   0,d[2] ), center + Vector3(    0,   0,d[2] )});
        sons[5] = new Octree({aabb_min + Vector3( d[0],   0,d[2] ), center + Vector3( d[0],   0,d[2] )});
        sons[6] = new Octree({aabb_min + Vector3( d[0],d[1],d[2] ), center + Vector3( d[0],d[1],d[2] )});
        sons[7] = new Octree({aabb_min + Vector3(    0,d[1],d[2] ), center + Vector3(    0,d[1],d[2] )});

        if( level > 1 )
            for( int i=0; i < 8; i++ )
                sons[i]->buildComplete( level - 1 );
    }    

protected:
    bool isLeaf() const
    {
        return !(sons[0]||sons[1]||sons[2]||sons[3]||sons[4]||sons[5]||sons[6]||sons[7]);
    };

private:
    AABB aabb;
    Octree* sons[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
};
