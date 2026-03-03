#include <geometry/detail/SphericalHarmonicsMath.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

const double sqrt2 = std::sqrt(2.0);

// See "Spherical Harmonic Lighting: The Gritty Details" by Robin Green, 2003

namespace detail {

double P(int l, int m, double x) {
  // evaluate an Associated Legendre Polynomial P(l,m,x) at x
  double pmm = 1.0;
  if (m > 0) {
    double somx2 = std::sqrt((1.0 - x) * (1.0 + x));
    double fact = 1.0;
    for (int i = 1; i <= m; i++) {
      pmm *= (-fact) * somx2;
      fact += 2.0;
    }
  }
  if (l == m)
    return pmm;
  double pmmp1 = x * (2.0 * m + 1.0) * pmm;
  if (l == m + 1)
    return pmmp1;
  double pll = 0.0;
  for (int ll = m + 2; ll <= l; ++ll) {
    pll = ((2.0 * ll - 1.0) * x * pmmp1 - (ll + m - 1.0) * pmm) / (ll - m);
    pmm = pmmp1;
    pmmp1 = pll;
  }
  return pll;
}

unsigned factorial(unsigned x) {
  const int N = 33;
  static bool first_call = true;
  static unsigned table[N];
  if (first_call) {
    // Compute table in first call
    table[0] = 1;
    for (int i = 1; i < N; i++)
      table[i] = table[i - 1] * i;

    first_call = false;
  }

  // Return tabulated value
  if (x < N)
    return table[x];

  // Compute factorial on-the-fly
  int fac = table[N - 1];
  for (unsigned i = N; i <= x; i++)
    fac *= (int)i;
  return fac;
}

double K(int l, int m) {
  // renormalisation constant for SH function
  double temp = ((2.0 * l + 1.0) * static_cast<double>(factorial(l - m))) /
                (4.0 * M_PI * static_cast<double>(factorial(l + m)));
  return std::sqrt(temp);
}

double SH(int l, int m, double theta, double phi) {
  if (m == 0)
    return K(l, 0) * P(l, m, std::cos(theta));
  else if (m > 0)
    return sqrt2 * K(l, m) * std::cos(m * phi) * P(l, m, std::cos(theta));
  else
    return sqrt2 * K(l, -m) * std::sin(-m * phi) * P(l, -m, std::cos(theta));
}

double dPlm_mu(int l, int m, double cos_theta, double Plm) {
  return (l * cos_theta * Plm - (l + m) * P(l - 1, m, cos_theta)) /
         std::sqrt(1. - cos_theta * cos_theta);
}

double dSH(int l, int m, double theta, double phi, double &dtheta,
           double &dphi) {
  double sh = 0.0;
  if (m == 0) {
    double u = std::cos(theta);
    double Plm = P(l, m, u);
    double Klm = K(l, 0);
    sh = Klm * Plm;

    dphi = 0.0;
    dtheta = Klm * dPlm_mu(l, m, u, Plm) * (-sin(theta));
  } else if (m > 0) {
    double u = std::cos(theta);
    double Plm = P(l, m, u);
    double Klm = K(l, m);
    double cosm = cos(m * phi);
    sh = sqrt2 * Klm * cosm * Plm;

    dphi = sqrt2 * Klm * (-m * std::sin(m * phi)) * Plm;
    dtheta = sqrt2 * Klm * cosm * dPlm_mu(l, m, u, Plm) * (-std::sin(theta));
  } else // m < 0
  {
    double u = std::cos(theta);
    double Plm = P(l, -m, u);
    double Klm = K(l, -m);
    sh = sqrt2 * Klm * std::sin(-m * phi) * Plm;

    dphi = sqrt2 * Klm * (-m * std::cos(m * phi)) * Plm;
    dtheta = sqrt2 * Klm * std::sin(-m * phi) * dPlm_mu(l, -m, u, Plm) *
             (-std::sin(theta));
  }
  return sh;
}

double SH(vec3 v, int l, int m) {
  // Polar coordinates
  double theta = std::acos(v.z), phi = std::atan2(v.y, v.x);
  return SH(l, m, theta, phi);
}

} // namespace detail
