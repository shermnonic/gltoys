#pragma once
#include <geometry/Icosahedron.h>

class SphericalHarmonicsFunction : public Icosahedron
{
  public:
    SphericalHarmonicsFunction(int order = 10) : m_order(order) {}

    void create(int level = -1);
    void update();

    void resetCoefficients();
    void randomizeCoefficients();
    void symmetrizeCoefficients();

    std::vector<float> &getCoeffs() { return m_coeffs; }
    const std::vector<float> &getCoeffs() const { return m_coeffs; }

  private:
    void createBasis();

    struct SHB
    {
        double r = 0;      // SH value (=radius)
        double dtheta = 0; // Gradient in theta
        double dphi = 0;   // Gradient in phi
    };

    std::vector<std::vector<SHB>> m_shb; // SH basis
    std::vector<float> m_radius;         // Scaling factor
    std::vector<float> m_coeffs;         // SH coefficients
    int m_order;                         // number of SH bands
    std::vector<float> m_vcache;         // vertex cache for icosahedron
};
