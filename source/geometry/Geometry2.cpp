#include "Geometry2.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <cstdlib> // for rand() and RAND_MAX
#include <cstdio>
#include <set>
#include <tuple>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

using std::abs;

//==============================================================================
//	SimpleGeometry
//==============================================================================

SimpleGeometry::Face::Face() 
	{ vi[0]=vi[1]=vi[2]=0; }
SimpleGeometry::Face::Face( int i[3] )
	{ vi[0]=i[0]; vi[1]=i[1]; vi[2]=i[2]; }
SimpleGeometry::Face::Face( int i, int j, int k )
	{ vi[0]=i; vi[1]=j; vi[2]=k; }

void SimpleGeometry::clear()
{
	m_vdata.clear();
	m_ndata.clear();
	m_fdata.clear();
}

void SimpleGeometry::reserve_vertices( int n )
{
	m_vdata.reserve( 3*(n-8) );
	m_ndata.reserve( 3*(n-8) );
}

void SimpleGeometry::reserve_faces( int n )
{
	m_fdata.reserve( 3*n );
}

int SimpleGeometry::num_vertices() const { return (int)m_vdata.size()/3; }
int SimpleGeometry::num_faces()    const { return (int)m_fdata.size()/3; }

void SimpleGeometry::set_vertex( int i, vec3 v )
{
	assert( i < m_vdata.size()/3 );
	m_vdata[i*3  ] = v.x;
	m_vdata[i*3+1] = v.y;
	m_vdata[i*3+2] = v.z;
}

void SimpleGeometry::set_normal( int i, vec3 n )
{
	assert( i < m_ndata.size()/3 );
	m_ndata[i*3  ] = n.x;
	m_ndata[i*3+1] = n.y;
	m_ndata[i*3+2] = n.z;
}

vec3 SimpleGeometry::get_vertex( int i ) const
{
	return vec3( m_vdata[i*3], m_vdata[i*3+1], m_vdata[i*3+2] );
}

vec3 SimpleGeometry::get_normal( int i ) const
{
	return vec3( m_ndata[i*3], m_ndata[i*3+1], m_ndata[i*3+2] );
}

SimpleGeometry::Face SimpleGeometry::get_face( int i ) const
{
	return Face( m_fdata[i*3+0], m_fdata[i*3+1], m_fdata[i*3+2] );
}

int SimpleGeometry::add_face( Face f )
{
	for( int i=0; i < 3; ++i )
		m_fdata.push_back( f.vi[i] );
	assert( m_fdata.size()%3 == 0 );
	return (int)m_fdata.size()/3 - 1;
}

int SimpleGeometry::add_vertex_and_normal( vec3 v, vec3 n )
{	
	for( int i=0; i < 3; ++i ) 
	{
		m_vdata.push_back( v[i] );
		m_ndata.push_back( n[i] );
	}
	assert( m_vdata.size() == m_ndata.size() );
	assert( m_vdata.size()%3 == 0 );
	return (int)m_vdata.size()/3 - 1;
}

float* SimpleGeometry::get_vertex_ptr()  { return &m_vdata[0]; }

float* SimpleGeometry::get_normal_ptr ()  { return &m_ndata[0]; }

int*   SimpleGeometry::get_index_ptr()   { return &m_fdata[0]; }

bool SimpleGeometry::writeOBJ( const char* filename ) const
{
	FILE* f = fopen( filename, "w" );
	if( !f )
	{
		printf( "Could not write OBJ '%s'!", filename );
		return false;
	}

	size_t nv = m_vdata.size()/3;
	bool has_normals = m_ndata.size() == m_vdata.size();
	for( size_t i=0; i < nv; ++i )
	{		
		fprintf( f, "v %12.11f %12.11f %12.11f \n", 
		         m_vdata[3*i+0], m_vdata[3*i+1], m_vdata[3*i+2] );

		if( has_normals )
			fprintf( f, "vn %12.11f %12.11f %12.11f \n", 
					 m_ndata[3*i+0], m_ndata[3*i+1], m_ndata[3*i+2] );
	}

	size_t nf = m_fdata.size()/3;
	for( size_t i=0; i < nf; ++i )
	{
		// Note that face indices are 1-based in OBJ!
		fprintf( f, "f %4d %4d %4d \n",
		         m_fdata[3*i+0]+1, m_fdata[3*i+1]+1, m_fdata[3*i+2]+1 );
	}

	fclose(f);
	return true;
}


//==============================================================================
//	Icosahedron
//==============================================================================

void Icosahedron::add_face_subdivision( Face f, int levels )
{
	if( levels == 0 )
	{
		// insert face for real
		add_face( f );
		return;
	}

	// get vertices
	vec3 v1 = get_vertex( f.vi[0] ),
		 v2 = get_vertex( f.vi[1] ),
		 v3 = get_vertex( f.vi[2] );
	
	// new vertices
	vec3 v12=v1,v13=v1,v23=v2;
	v12 += v2;
	v13 += v3;
	v23 += v3;
#if 0
	// test subdivision
	v12 /= 2.f;
	v13 /= 2.f;
	v23 /= 2.f;
#else
	// normalize such that the new vertices again lie on the unit sphere surface
	v12.normalize();
	v13.normalize();
	v23.normalize();

	float foo = (float)m_platonicConstantZ;
	v12 *= foo;
	v13 *= foo;
	v23 *= foo;
#endif
	
	// corresponding indices
	int i1  = f.vi[0],
	    i2  = f.vi[1],
	    i3  = f.vi[2],
		i12 = add_vertex_and_normal( v12, v12/v12.magnitude() ),
		i13 = add_vertex_and_normal( v13, v13/v13.magnitude() ),
		i23 = add_vertex_and_normal( v23, v23/v23.magnitude() );
	
	// 4 new faces
	add_face_subdivision( Face( i1,  i12, i13 ), levels-1 );
	add_face_subdivision( Face( i12, i2,  i23 ), levels-1 );
	add_face_subdivision( Face( i13, i23, i3  ), levels-1 );
	add_face_subdivision( Face( i12, i23, i13 ), levels-1 );
}

void Icosahedron::create( int levels )
{
	if( levels < 0 ) 
		levels = m_levels;
	else
		m_levels = levels;

	float tao = (float)m_platonicConstantX; // for true icosahedron use golden ratio (1+sqrt(5))/2 = 1.61803399
	vec3 vdata[12] = { vec3(1,tao,0),vec3(-1,tao,0),vec3(1,-tao,0),vec3(-1,-tao,0),
					   vec3(0,1,tao),vec3(0,-1,tao),vec3(0,1,-tao),vec3(0,-1,-tao),
					   vec3(tao,0,1),vec3(-tao,0,1),vec3(tao,0,-1),vec3(-tao,0,-1) };

	static int tindices[20][3] = { 
	{0,1,4},{1,9,4},{4,9,5},{5,9,3},{2,3,7},{3,2,5},{7,10,2},{0,8,10},{0,4,8},{8,2,10},{8,4,5},{8,5,2},{1,0,6},{11,1,6},{3,9,11},{6,10,7},{3,11,7},{11,6,7},{6,0,10},{9,1,11}
	};

	// exact memory calulation
	int n = (int)(20.0*pow(4.0,(double)levels));
	reserve_vertices( n - 8 );
	reserve_faces   ( n );
	   
	// insert vertices
	for( int i=0; i < 12; ++i )
	{
		// add normalized vertices lying on the surface of the unit sphere
		vec3 v = vdata[i];
		v.normalize();
		add_vertex_and_normal( v, v );
	}
	
	// insert faces (at the end of subdivision process)
	for( int i=0; i < 20; ++i ) 
	{
		add_face_subdivision( Face(tindices[i]), levels );
	}
}

//==============================================================================
//	Penrose tiling
//==============================================================================

Penrose::Penrose()
: m_levels(1)
{
	setDefaultGenerator();
}

void Penrose::reserve_faces( int n )
{
	// Reserve memory for face type attribute
	m_faceType.reserve( n );
	
	// Call super implementation
	SimpleGeometry::reserve_faces( n );
}

int Penrose::add_face( SimpleGeometry::Face f, int type )
{
	// Set face type
	m_faceType.push_back( type );
	// Call super implementation
	return SimpleGeometry::add_face( f );	
}

int Penrose::add_face( SimpleGeometry::Face f )
{
	// Distinguish face type by thresholding angle at apex (= first vertex).
	// Note that we do not check for isoscele nor exact angles 36� / 108�.
	float* vp = get_vertex_ptr();
	vec3 AB = get_vertex( f.vi[1] ) - get_vertex( f.vi[0] ),
	     AC = get_vertex( f.vi[2] ) - get_vertex( f.vi[0] );
	float theta = acos( AB.scalarprod( AC ) / (AB.magnitude() * AC.magnitude()) );
	int type = ( theta < 50.f ) ? Red : Blue;
	
	return add_face( f, type );
}

void Penrose::add_face_subdivision( SimpleGeometry::Face f, int type, int levels )
{
	const double goldenRatio = (1. + sqrt(5.)) / 2.;
	
	if( levels == 0 )
	{
		// insert face for real
		add_face( f, type );
		return;
	}
	
	// get vertices
	vec3 a = get_vertex( f.vi[0] ),
	     b = get_vertex( f.vi[1] ),
	     c = get_vertex( f.vi[2] );
	
	// get normals
	vec3 na = get_normal( f.vi[0] ),
	     nb = get_normal( f.vi[1] ),
	     nc = get_normal( f.vi[2] );
	
	if( type == Red )
	{
		// split into two triangles
		
		// new vertex p
		vec3 p = a + (b - a) / (float)goldenRatio;
		// new normal by interpolating normals at a and b
		vec3 np = na + (nb - na) / (float)goldenRatio;
		np.normalize();
		int pi = add_vertex_and_normal( p, np );		
		// new faces
		Face pca( pi, f.vi[2], f.vi[0] );
		Face cpb( f.vi[2], pi, f.vi[1] );
		
		add_face_subdivision( pca, Blue, levels-1 );
		add_face_subdivision( cpb, Red,  levels-1 );
	}
	else if( type == Blue )
	{
		// split into three triangles
		
		vec3 q = b + (a - b) / (float)goldenRatio,
		     r = b + (c - b) / (float)goldenRatio;
		
		vec3 nq = nb + (na - nb) / (float)goldenRatio,
		     nr = nb + (nc - nb) / (float)goldenRatio;
		nq.normalize();
		nr.normalize();
		
		int qi = add_vertex_and_normal( q, nq ),
		    ri = add_vertex_and_normal( r, nr );
		
		Face rca( ri, f.vi[2], f.vi[0] );
		Face qrb( qi, ri, f.vi[1] );
		Face rqa( ri, qi, f.vi[0] );
		
		add_face_subdivision( rca, Blue, levels-1 );
		add_face_subdivision( qrb, Blue, levels-1 );
		add_face_subdivision( rqa, Red,  levels-1 );
	}
	else
	{
		// This should not happen!
	}	
}

void Penrose::create( int levels )
{
	if( levels<0 )
		levels = m_levels;
	else
		m_levels = levels;

	// Copy generator vertices (FIXME: direct copy would be much faster!)
	for( int i=0; i < m_generator.num_vertices(); i++ )
		add_vertex_and_normal( m_generator.get_vertex(i), m_generator.get_normal(i) );

	// Start with all faces "Red"
	for( int i=0; i < m_generator.num_faces(); i++ )
		add_face_subdivision( m_generator.get_face(i), Red, levels );

	constexpr bool WheelGenerator = false;
	if constexpr(WheelGenerator)
	{
		// Origin
		add_vertex_and_normal( vec3( 0,0,0 ), vec3(0,0,1) );
		
		// Wheel around origin
		for( int i=0; i < 10; i++ )
		{
			double phi = (double)i * 2.*M_PI / 10.;
			add_vertex_and_normal(
				vec3( cos(phi), sin(phi), 0 ),
				vec3( 0,0,1 ) );
		}
		
		// Subdivide red triangles
		for( int i=0; i < 10; i++ )
		{
			int j = (i+0) % 10 + 1,
				k = (i+1) % 10 + 1;

			// Mirror every second triangle
			if( i%2 )		
				add_face_subdivision( Face( 0, j, k ), Red, levels );
			else
				add_face_subdivision( Face( 0, k, j ), Red, levels );
		}
	}
}

void Penrose::setDefaultGenerator()
{
	clear();

	SimpleGeometry geom;

	// Origin
	geom.add_vertex_and_normal( vec3( 0,0,0 ), vec3(0,0,1) );
	
	// Wheel around origin
	for( int i=0; i < 10; i++ )
	{
		double phi = (double)i * 2.*M_PI / 10. + M_PI/2.;
		geom.add_vertex_and_normal(
			vec3( (float)cos(phi), (float)sin(phi), 0.f ),
			vec3( 0.f,0.f,1.f ) );
	}
	
	// Subdivide red triangles
	for( int i=0; i < 10; i++ )
	{
		int j = (i+0) % 10 + 1,
			k = (i+1) % 10 + 1;

		// Mirror every second triangle
		if( i%2 )		
			geom.add_face( Face( 0, j, k ) );
		else
			geom.add_face( Face( 0, k, j ) );
	}

	// Copy
	m_generator = geom;
}

void Penrose::setGenerator( const SimpleGeometry& geom )
{
	// Copy
	m_generator = geom;
}


//==============================================================================
//	Superquadric
//==============================================================================

//-----------------------------------------------------------------------------
//  Superquadric helper functions
//-----------------------------------------------------------------------------

/// Sign function
double sgn( double value )
{
	return (value >= 0.) ? +1. : -1.;
}

/// Signed absolute power
double spow( double base, double exponent )
{
	return sgn(base) * pow(abs(base),exponent);
}

/// Superquadric function around z-axis
vec3 qz( double theta, double phi, double alpha, double beta )
{
	double sphi = spow(sin(phi),beta);
	return vec3( (float)(spow(cos(theta),alpha) * sphi),
	             (float)(spow(sin(theta),alpha) * sphi),
	             (float) spow(cos(phi),beta) );
}

/// Superquadric function around x-axis
vec3 qx( double theta, double phi, double alpha, double beta )
{
	double sphi = spow(sin(phi),beta);
	return vec3((float)( spow(cos(phi),beta)),
	            (float)(-spow(sin(theta),alpha) * sphi),
	            (float)( spow(cos(theta),alpha) * sphi) );
}

/// Superquadric tensor function parameterized over planarity (cp) and linearity (cl)
vec3 superquadric_tensor( double cp, double cl, double gamma, double theta, double phi )
{
	if( cl >= cp )
		return qx( theta, phi, pow(1.-cp,gamma), pow(1.-cl,gamma) );	
	// cl < cp
	return qz( theta, phi, pow(1.-cl,gamma), pow(1.-cp,gamma) );
}

void Superquadric::create( int unused )
{
	clear();

	int res = 32;

	double theta_step = (2.*M_PI)/(double)res;
	double phi_step = M_PI/(double)res;

	int n = (int)std::floor( 2.*M_PI / theta_step );
	int m = (int)std::floor(M_PI / phi_step + 1.0);	
		// n*m == num_vertices()
	
	// Sample vertices
	for( int i=0; i < n; i++ )
	{
		double theta = (double)i*theta_step;		
		for( int j=0; j < m; j++ )
		{
			double phi = (double)j*phi_step;

			vec3 v;
			switch( m_mode )
			{
			default:
			case Quadric:
				// FIXME: Decide when to use qx or qz automatically.
				v = qx( theta, phi, m_alpha, m_beta );
				break;
			case TensorGlyph:
				v = superquadric_tensor( m_cp, m_cl, m_gamma, theta, phi );
				break;
			}

			add_vertex_and_normal( v, v/v.magnitude());			
		}
	}
	
	// Establish faces
	for( int i=0; i < n; i++ )
		for( int j=0; j < m; j++ )
		{
			// v3___v2
			//  |   |
			//  |___|
			// v0   v1
			int v0 = i*m + j,
				v1 = i*m + (j+1)%m,
				v2 = ((i+1)%n)*m + (j+1)%m,
				v3 = ((i+1)%n)*m + j;

			// Triangulate quad face
			add_face( Face(v0,v1,v3) );
			add_face( Face(v1,v2,v3) );
		}
}


//==============================================================================
//	SphericalHarmonics
//==============================================================================

// See "Spherical Harmonic Lighting: The Gritty Details" by Robin Green, 2003

double P(int l,int m,double x) 
{ 
	// evaluate an Associated Legendre Polynomial P(l,m,x) at x 
	double pmm = 1.0; 
	if(m>0) 
	{ 
		double somx2 = sqrt((1.0-x)*(1.0+x));
		double fact = 1.0; 
		for(int i=1; i<=m; i++) 
		{ 
			pmm *= (-fact) * somx2; 
			fact += 2.0; 
		} 
	} 
	if(l==m) return pmm; 
	double pmmp1 = x * (2.0*m+1.0) * pmm; 
	if(l==m+1) return pmmp1; 
	double pll = 0.0; 
	for(int ll=m+2; ll<=l; ++ll) 
	{ 
		pll = ( (2.0*ll-1.0)*x*pmmp1-(ll+m-1.0)*pmm ) / (ll-m); 
		pmm = pmmp1; 
		pmmp1 = pll; 
	} 
	return pll; 
} 

unsigned factorial( unsigned x )
{
	const int N = 33;
	static bool first_call = true;
	static unsigned table[N];
	if( first_call )
	{
		// Compute table in first call
		table[0] = 1;
		for( int i=1; i < N; i++ )
			table[i] = table[i-1]*i;

		first_call = false;
	}

	// Return tabulated value
	if( x < N )
		return table[x];
	
	// Compute factorial on-the-fly
	int fac = table[N-1];
	for( unsigned i=N; i <= x; i++ )
		fac *= (int)i;
	return fac;
}

double K(int l, int m) 
{ 
	// renormalisation constant for SH function 
	double temp = ((2.0*l+1.0)*(double)factorial(l-m)) / (4.0*M_PI*(double)factorial(l+m)); 
	return sqrt(temp); 
}

double SH(int l, int m, double theta, double phi) 
{ 
	// return a point sample of a Spherical Harmonic basis function 
	// l is the band, range [0..N] 
	// m in the range [-l..l] 
	// theta in the range [0..Pi] 
	// phi in the range [0..2*Pi] 
	const double sqrt2 = sqrt(2.0); 
	if(m==0) 
		return K(l,0)*P(l,m,cos(theta)); 
	else if(m>0) 
		return sqrt2*K(l,m)*cos(m*phi)*P(l,m,cos(theta)); 
	else 
		return sqrt2*K(l,-m)*sin(-m*phi)*P(l,-m,cos(theta)); 
}

double dPlm_mu( int l, int m, double cos_theta, double Plm )
{
	return (l*cos_theta*Plm - (l+m)*P(l-1,m,cos_theta)) / sqrt(1.-cos_theta*cos_theta);
}

double dSH( int l, int m, double theta, double phi, double& dtheta, double& dphi )
{
	const double sqrt2 = sqrt(2.0); 
	double sh = 0.0;	
	if( m == 0 )
	{	
		double u = cos(theta);
		double Plm = P(l,m,u);
		double Klm = K(l,0);
		sh = Klm*Plm;

		dphi   = 0.0;
		dtheta = Klm * dPlm_mu(l,m,u,Plm) * (-sin(theta));
	}
	else 
	if( m > 0 ) 
	{
		double u = cos(theta);
		double Plm = P(l,m,u);
		double Klm = K(l,m);
		double cosm = cos(m*phi);
		sh = sqrt2*Klm*cosm*Plm; 

		dphi   = sqrt2*Klm*( -m*sin(m*phi) )*Plm;
		dtheta = sqrt2*Klm*cosm * dPlm_mu(l,m,u,Plm) * (-sin(theta));
	}
	else // m < 0
	{
		double u = cos(theta);
		double Plm = P(l,-m,u);
		double Klm = K(l,-m);
		sh = sqrt2*Klm*sin(-m*phi)*Plm; 

		dphi   = sqrt2*Klm*( -m*cos(m*phi) )*Plm;
		dtheta = sqrt2*Klm*sin(-m*phi) * dPlm_mu(l,-m,u,Plm) * (-sin(theta));

	}
	return sh;
}


double SH( vec3 v, int l, int m )
{
	// Assume v is normalized
	// Polar coordinates
	double 
		theta = acos( v.z ),
		phi = atan2( v.y, v.x );
	return SH( l,m, theta,phi );
}

template<typename T> T clamp( const T& val, T min, T max )
{
	return (val < min) ? min : ((val > max) ? max : val);
}

void SphericalHarmonics::setLM( int l, int m )
{
	m_l = clamp( l, 0, 12 );
	m_m = clamp( m, -l, l );
}

void SphericalHarmonics::update()
{
	// Evaluate SH function on vertices (translated into spherical coordinates)
	for( int i=0; i < num_vertices(); ++i )
	{
		vec3 v = get_vertex( i );
		v.normalize(); // Normalize to project onto unit sphere

	  #if 1
		double dtheta, dphi; // Gradient
		double theta = acos( v.z ), phi = atan2( v.y, v.x );
		double sh = dSH( m_l, m_m, theta, phi, dtheta, dphi );
		vec3 n;
	   #if 0
		n = vec3( sin(dtheta)*cos(dphi), sin(dtheta)*sin(dphi), cos(dtheta) );
	   #else
		theta -= dtheta;
		phi   -= dphi;
		n = vec3( (float)(sin(theta)*cos(phi)), (float)(sin(theta)*sin(phi)), (float)cos(theta) );
	   #endif
		n.normalize();
		set_vertex( i, v*(float)abs(sh) ); // FIXME: Why absolute value of SH?
		set_normal( i, n );
	  #else
		set_vertex( i, v*(float)abs(SH( v, m_l,m_m )) );

		// Finite difference normal (too tired to implement SH gradient)
		float delta = 0.0001;
		vec3 dx(delta,0.f,0.f),
			 dy(0.f,delta,0.f),
			 dz(0.f,0.f,delta);
		vec3 grad( .5*(abs(SH( v+dx, m_l,m_m )) - abs(SH( v-dx, m_l,m_m ))),
			       .5*(abs(SH( v+dy, m_l,m_m )) - abs(SH( v-dy, m_l,m_m ))),
				   .5*(abs(SH( v+dz, m_l,m_m )) - abs(SH( v-dz, m_l,m_m ))) );
		grad.normalize();
		set_normal( i, grad );
	  #endif
	}

	// TODO: Update normals!
}

void SphericalHarmonics::create( int level )
{
	clear();

	// Sample vertices on sphere via an subdivided icosahedron
	Icosahedron::create( level );

	update();
}


//==============================================================================
//	SHF
//==============================================================================

std::tuple<double, double> polar( const vec3& v )
{
	// Normalize to project onto unit sphere
	double const l = v.magnitude();
	return {
		acos( v.z / l ),
		atan2( v.y / l, v.x / l )
	};
}

void SHF::create( int level )
{
	clear();

	// Sample vertices on sphere via an subdivided icosahedron
	Icosahedron::create( level );

	// Cache vertices	
	m_vcache = vbuffer();

	// Compute SH basis (expensive, depending on order)
	createBasis();
}

void SHF::resetCoefficients()
{
	for( int j=0, l=0; l < m_order; l++ ) // band l, linear index j
		for( int m=-l; m <= l; m++, j++ ) // range m
		{
			m_coeffs[j] = 0.;
		}

	m_coeffs[0] = 4.0;
}

void SHF::randomizeCoefficients()
{
	for( int j=0, l=0; l < m_order; l++ ) // band l, linear index j
		for( int m=-l; m <= l; m++, j++ ) // range m
		{
			assert( j < m_coeffs.size() );
			double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX; // random [0,1]
			r = 2.*r - 1.; // map to [-1,1]
			double h = (double)(2*l+1); // normalize with band range
			m_coeffs[j] = (float)(10.*r / ((double)m_radius[j]*h*(double)m_order));
		}

	m_coeffs[0] = 4.0;
}

void SHF::symmetrizeCoefficients()
{
	for( int j=0, l=0; l < m_order; l++ ) // band l, linear index j
		for( int m=-l; m <= l; m++, j++ ) // range m
		{
			int center = j + l; // index of (l,m=0)
			if( m < 0 )
			{
				m_coeffs[j] = 0.0;
				
				// Average negative and positive order
				//int j_dash = center - m;
				//m_coeffs[j] = .5*(m_coeffs[j] + m_coeffs[j_dash]);
			}
			else
			if( m > 0 )
			{
				m_coeffs[j] = m_coeffs[j];

				// Copy average stored in left half to right half
				//int j_dash = center - m;
				//m_coeffs[j] = m_coeffs[j_dash];
			}
		}
}

vec3 fromPolar( double theta, double phi )
{
	return vec3( (float)(sin(theta)*cos(phi)), (float)(sin(theta)*sin(phi)), (float)cos(theta) );
}

void SHF::update()
{
	constexpr bool Debug = false;

	for( int i=0; i < num_vertices(); ++i )
	{
		// Evalute function in SH basis
		SHB s;
		for( int j=0; j < m_shb.size(); j++ )
		{
			s.r     += m_coeffs[j] * m_shb[j][i].r;
			s.dphi  += m_coeffs[j] * m_shb[j][i].dphi;
			s.dtheta+= m_coeffs[j] * m_shb[j][i].dtheta;
		}

		// Displace vertex		
		vec3 v( m_vcache[i*3], m_vcache[i*3+1], m_vcache[i*3+2] ); //get_vertex( i );
		v.normalize(); // Project onto unit sphere (sanity)
		set_vertex( i, v*(float)s.r );

		// Normal from gradient
		auto const [theta, phi] = polar(v);

		if constexpr(Debug)
		{
			printf("vertex %04d: theta=%8.7fpi, phi=%8.7fpi\n",i,theta/M_PI,phi/M_PI);
		}

		vec3 n = fromPolar( theta-s.dtheta, phi-s.dphi );
		n.normalize();

		// Handle zero angles
		constexpr double eps = 0.1;
		if( abs(theta/M_PI)<eps && abs(phi/M_PI)<eps )
		{
			if constexpr(Debug)
			{
				printf("*** Encountered zero angles at vertex %d\n",i);
			}
			n = vec3(0.f,0.f,1.f);
		}

		set_normal( i, n );
	}
}

void SHF::createBasis()
{
	// Allocate memory for sample SH basis functions
	m_shb.resize( (size_t)m_order*m_order );
	for( int j=0; j < m_shb.size(); j++ )
		m_shb[j].resize( (size_t)num_vertices() );

	m_radius.resize( m_shb.size() );

	// Resize coefficient vector
	m_coeffs.resize( m_shb.size() );

	// Evaluate SH function on vertices (translated into spherical coordinates)
	for( int i=0; i < num_vertices(); ++i )
	{
		// Sample sphere in polar coordinates 		
		vec3 const v = get_vertex( i );
		auto const [theta, phi] = polar(v);	

		// Compute all basis coefficients for current (theta,phi)
		for( int j=0, l=0; l < m_order; l++ ) // band l, linear index j
			for( int m=-l; m <= l; m++, j++ ) // range m
			{
				assert( j  < m_shb.size() ); // sanity

				// Compute SH radius and gradient
				SHB s;
				s.r = dSH( l, m, theta, phi, s.dtheta, s.dphi );
				m_shb[j][i] = s;

				// Store max. radius for each SH basis for later normalization
				m_radius[j] = (i==0 || (float)s.r>m_radius[j]) ? (float)s.r : m_radius[j];
			}
	}
}

