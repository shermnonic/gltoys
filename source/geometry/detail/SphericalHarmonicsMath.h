#pragma once

#include <geometry/Vector.h>

namespace detail
{

    template <typename T> T clamp(const T &val, T min, T max)
    {
        return (val < min) ? min : ((val > max) ? max : val);
    }

    // return a point sample of a Spherical Harmonic basis function
    // l is the band, range [0..N]
    // m in the range [-l..l]
    // theta in the range [0..Pi]
    // phi in the range [0..2*Pi]
    double SH(int l, int m, double theta, double phi);

    double dSH(int l, int m, double theta, double phi, double &dtheta,
               double &dphi);

    // Assume v is normalized
    double SH(vec3 v, int l, int m);

}
