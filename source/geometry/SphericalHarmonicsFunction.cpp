#include <geometry/SphericalHarmonicsFunction.h>
#include <geometry/detail/SphericalHarmonicsMath.h>

#include <cassert>
#include <cmath>
#include <cstdio> // printf (for debugging)
#include <tuple>

using namespace detail;

std::tuple<double, double> polar(const vec3 &v)
{
    // Normalize to project onto unit sphere
    double const l = v.magnitude();
    return {std::acos(v.z / l), std::atan2(v.y / l, v.x / l)};
}

void SphericalHarmonicsFunction::create(int level)
{
    clear();

    // Sample vertices on sphere via an subdivided icosahedron
    Icosahedron::create(level);

    // Cache vertices
    m_vcache = vbuffer();

    // Compute SH basis (expensive, depending on order)
    createBasis();
}

void SphericalHarmonicsFunction::resetCoefficients()
{
    for (int j = 0, l = 0; l < m_order; l++) // band l, linear index j
        for (int m = -l; m <= l; m++, j++)   // range m
        {
            m_coeffs[j] = 0.;
        }

    m_coeffs[0] = 4.0;
}

void SphericalHarmonicsFunction::randomizeCoefficients()
{
    for (int j = 0, l = 0; l < m_order; l++) // band l, linear index j
        for (int m = -l; m <= l; m++, j++)   // range m
        {
            assert(j < m_coeffs.size());
            double r =
                (double)(rand() % RAND_MAX) / (double)RAND_MAX; // random [0,1]
            r = 2. * r - 1.;                                    // map to [-1,1]
            double h = (double)(2 * l + 1); // normalize with band range
            m_coeffs[j] =
                (float)(10. * r / ((double)m_radius[j] * h * (double)m_order));
        }

    m_coeffs[0] = 4.0;
}

void SphericalHarmonicsFunction::symmetrizeCoefficients()
{
    for (int j = 0, l = 0; l < m_order; l++) // band l, linear index j
        for (int m = -l; m <= l; m++, j++)   // range m
        {
            int center = j + l; // index of (l,m=0)
            if (m < 0)
            {
                m_coeffs[j] = 0.0;

                // Average negative and positive order
                // int j_dash = center - m;
                // m_coeffs[j] = .5*(m_coeffs[j] + m_coeffs[j_dash]);
            }
            else if (m > 0)
            {
                m_coeffs[j] = m_coeffs[j];

                // Copy average stored in left half to right half
                // int j_dash = center - m;
                // m_coeffs[j] = m_coeffs[j_dash];
            }
        }
}

vec3 fromPolar(double theta, double phi)
{
    return vec3(static_cast<float>(std::sin(theta) * std::cos(phi)),
                static_cast<float>(std::sin(theta) * std::sin(phi)),
                static_cast<float>(std::cos(theta)));
}

void SphericalHarmonicsFunction::update()
{
    constexpr bool Debug = false;

    for (int i = 0; i < num_vertices(); ++i)
    {
        // Evalute function in SH basis
        SHB s;
        for (int j = 0; j < m_shb.size(); j++)
        {
            s.r += m_coeffs[j] * m_shb[j][i].r;
            s.dphi += m_coeffs[j] * m_shb[j][i].dphi;
            s.dtheta += m_coeffs[j] * m_shb[j][i].dtheta;
        }

        // Displace vertex
        vec3 v(m_vcache[i * 3], m_vcache[i * 3 + 1],
               m_vcache[i * 3 + 2]); // get_vertex( i );
        v.normalize();               // Project onto unit sphere (sanity)
        set_vertex(i, v * (float)s.r);

        // Normal from gradient
        auto const [theta, phi] = polar(v);

        if constexpr (Debug)
        {
            std::printf("vertex %04d: theta=%8.7fpi, phi=%8.7fpi\n", i,
                        theta / M_PI, phi / M_PI);
        }

        vec3 n = fromPolar(theta - s.dtheta, phi - s.dphi);
        n.normalize();

        // Handle zero angles
        constexpr double eps = 0.1;
        if (abs(theta / M_PI) < eps && abs(phi / M_PI) < eps)
        {
            if constexpr (Debug)
            {
                std::printf("*** Encountered zero angles at vertex %d\n", i);
            }
            n = vec3(0.f, 0.f, 1.f);
        }

        set_normal(i, n);
    }
}

void SphericalHarmonicsFunction::createBasis()
{
    // Allocate memory for sample SH basis functions
    m_shb.resize((size_t)m_order * m_order);
    for (int j = 0; j < m_shb.size(); j++)
        m_shb[j].resize((size_t)num_vertices());

    m_radius.resize(m_shb.size());

    // Resize coefficient vector
    m_coeffs.resize(m_shb.size());

    // Evaluate SH function on vertices (translated into spherical coordinates)
    for (int i = 0; i < num_vertices(); ++i)
    {
        // Sample sphere in polar coordinates
        vec3 const v = get_vertex(i);
        auto const [theta, phi] = polar(v);

        // Compute all basis coefficients for current (theta,phi)
        for (int j = 0, l = 0; l < m_order; l++) // band l, linear index j
            for (int m = -l; m <= l; m++, j++)   // range m
            {
                assert(j < m_shb.size()); // sanity

                // Compute SH radius and gradient
                SHB s;
                s.r = dSH(l, m, theta, phi, s.dtheta, s.dphi);
                m_shb[j][i] = s;

                // Store max. radius for each SH basis for later normalization
                m_radius[j] = (i == 0 || (float)s.r > m_radius[j])
                                  ? (float)s.r
                                  : m_radius[j];
            }
    }
}
