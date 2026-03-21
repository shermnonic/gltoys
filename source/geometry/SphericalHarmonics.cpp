#include <geometry/SphericalHarmonics.h>
#include <geometry/detail/SphericalHarmonicsMath.h>

using namespace detail;

SphericalHarmonics::SphericalHarmonics()
{
    create();
}

void SphericalHarmonics::setLM(int l, int m)
{
    m_l = clamp(l, 0, 12);
    m_m = clamp(m, -l, l);
}

void SphericalHarmonics::update()
{
    // Evaluate SH function on vertices (translated into spherical coordinates)
    for (int i = 0; i < num_vertices(); ++i)
    {
        vec3 v = get_vertex(i);
        v.normalize(); // Normalize to project onto unit sphere

        constexpr bool A = true;
        constexpr bool B = false;

        if constexpr (A)
        {
            double dtheta, dphi; // Gradient
            double theta = std::acos(v.z), phi = std::atan2(v.y, v.x);
            double sh = dSH(m_l, m_m, theta, phi, dtheta, dphi);
            vec3 n;
            if constexpr (B)
            {
                n = vec3(std::sin(dtheta) * std::cos(dphi),
                         std::sin(dtheta) * std::sin(dphi), std::cos(dtheta));
            }
            else
            {
                theta -= dtheta;
                phi -= dphi;
                n = vec3((float)(std::sin(theta) * std::cos(phi)),
                         (float)(std::sin(theta) * std::sin(phi)),
                         (float)std::cos(theta));
            }
            n.normalize();
            set_vertex(i, v * static_cast<float>(std::abs(
                                  sh))); // FIXME: Why absolute value of SH?
            set_normal(i, n);
        }
        else
        {
            set_vertex(i, v * static_cast<float>(std::abs(SH(v, m_l, m_m))));

            // Finite difference normal (too tired to implement SH gradient)
            float const delta = 0.0001;
            vec3 dx(delta, 0.f, 0.f), dy(0.f, delta, 0.f), dz(0.f, 0.f, delta);
            vec3 grad(.5 * (std::abs(SH(v + dx, m_l, m_m)) -
                            std::abs(SH(v - dx, m_l, m_m))),
                      .5 * (std::abs(SH(v + dy, m_l, m_m)) -
                            std::abs(SH(v - dy, m_l, m_m))),
                      .5 * (std::abs(SH(v + dz, m_l, m_m)) -
                            std::abs(SH(v - dz, m_l, m_m))));
            grad.normalize();
            set_normal(i, grad);
        }
    }

    recomputeVertexNormals();
}

void SphericalHarmonics::create(int level)
{
    // Sample vertices on sphere via an subdivided icosahedron
    Icosahedron::create(level);

    update();
}
