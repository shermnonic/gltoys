#pragma once
#include <geometry/SimpleGeometry.h>

/// Generate a Penrose tiling pattern
/// Implementation uses a subdivision approach as described on
///   http://preshing.com/20110831/penrose-tiling-explained/
class Penrose : public SimpleGeometry
{
public:
	enum FaceTypes { Blue, Red };
	
	Penrose();

	void create( int levels=-1 );

	void setDefaultGenerator();
	void setGenerator( const SimpleGeometry& geom );

	void setLevels( int levels ) 
		{ 
			m_levels = levels; 
			if( m_levels <= 0 ) m_levels=1;
		};
	int getLevels() const { return m_levels; }

protected:
	///@{ Implement custom face type attribute
	virtual void reserve_faces( int n );
	virtual int add_face( SimpleGeometry::Face f );
	///@}
	// Internal function called for each added face setting also type attribute
	int add_face( SimpleGeometry::Face f, int type );

	void add_face_subdivision( SimpleGeometry::Face f, int type, int levels );
	
private:	
	int m_levels = 1;	
	std::vector<int> m_faceType;
	SimpleGeometry m_generator;
};
