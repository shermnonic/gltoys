#pragma once
#include <geometry/SimpleGeometry.h>

/// Icosahedron with subdivision, nice regular sphere approximation
class Icosahedron : public SimpleGeometry
{
public:
    static constexpr double GoldenRatio = 1.6180339887498948482045;

	Icosahedron() = default;

	void setPlatonicConstants( double X, double Z )
		{
			m_platonicConstantX = X;
			m_platonicConstantZ = Z;
		}
	double getPlatonicConstantsX() const { return m_platonicConstantX; }
	double getPlatonicConstantsZ() const { return m_platonicConstantZ; }

	void setLevels( int levels ) 
		{ 
			m_levels = levels; 
			if( m_levels <= 0 ) m_levels=1;
		};
	int getLevels() const { return m_levels; }

	/// Create an Icosahedron model where each face is subdivided level times
	/// In the limit the subdivision surface is a sphere.
	virtual void create( int level=-1 );

protected:
	/// Recursive face subdivision routine
	/// For levels=0 the face is inserted into the model
	void add_face_subdivision( Face f, int levels );

private:
	int    m_levels = 4;
	double m_platonicConstantX = GoldenRatio;
    double m_platonicConstantZ = 1.0;
};
