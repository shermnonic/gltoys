#include <geometry/Superquadric.h>

#include <cmath>

namespace
{

    /// Sign function
    double sgn(double value) { return (value >= 0.) ? +1. : -1.; }

    /// Signed absolute power
    double spow(double base, double exponent)
    {
        return sgn(base) * std::pow(std::abs(base), exponent);
    }

    /// Superquadric function around z-axis
    vec3 qz(double theta, double phi, double alpha, double beta)
    {
        double sphi = spow(std::sin(phi), beta);
        return vec3((float)(spow(std::cos(theta), alpha) * sphi),
                    (float)(spow(std::sin(theta), alpha) * sphi),
                    (float)spow(std::cos(phi), beta));
    }

    /// Superquadric function around x-axis
    vec3 qx(double theta, double phi, double alpha, double beta)
    {
        double sphi = spow(std::sin(phi), beta);
        return vec3((float)(spow(std::cos(phi), beta)),
                    (float)(-spow(std::sin(theta), alpha) * sphi),
                    (float)(spow(std::cos(theta), alpha) * sphi));
    }

    /// Superquadric tensor function parameterized over planarity (cp) and
    /// linearity (cl)
    vec3 superquadric_tensor(double cp, double cl, double gamma, double theta,
                             double phi)
    {
        if (cl >= cp)
            return qx(theta, phi, std::pow(1. - cp, gamma),
                      std::pow(1. - cl, gamma));
        // cl < cp
        return qz(theta, phi, std::pow(1. - cl, gamma),
                  std::pow(1. - cp, gamma));
    }

}

void Superquadric::create(int newResolutionFactor)
{
    if(newResolutionFactor > 0)
    {
        resolutionFactor = newResolutionFactor;
    }

    clear();

    int const res = 18 * std::max(1, resolutionFactor);

    double theta_step = (2. * M_PI) / (double)res;
    double phi_step = M_PI / (double)res;

    int n = (int)std::floor(2. * M_PI / theta_step);
    int m = (int)std::floor(M_PI / phi_step + 1.0);
    // n*m == num_vertices()

    // Sample vertices
    for (int i = 0; i < n; i++)
    {
        double theta = (double)i * theta_step;
        for (int j = 0; j < m; j++)
        {
            double phi = (double)j * phi_step;

            vec3 v;
            switch (mode)
            {
            default:
            case Mode::Quadric:
                // FIXME: Decide when to use qx or qz automatically.
                v = qx(theta, phi, quadricParameters.alpha, quadricParameters.beta);
                break;
            case Mode::TensorGlyph:
                v = superquadric_tensor(tensorParameters.planarity, tensorParameters.linearity, tensorParameters.sharpness, theta, phi);
                break;
            }

            add_vertex_and_normal(v, v / v.magnitude());
        }
    }

    // Establish faces
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
        {
            // v3___v2
            //  |   |
            //  |___|
            // v0   v1
            int v0 = i * m + j, v1 = i * m + (j + 1) % m,
                v2 = ((i + 1) % n) * m + (j + 1) % m,
                v3 = ((i + 1) % n) * m + j;

            // Triangulate quad face
            add_face(Face(v0, v1, v3));
            add_face(Face(v1, v2, v3));
        }
}
