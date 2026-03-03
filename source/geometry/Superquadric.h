#pragma once
#include <geometry/SimpleGeometry.h>

class Superquadric : public SimpleGeometry
{
  public:
    enum Mode
    {
        Quadric,
        TensorGlyph
    };

    Superquadric()
        : m_mode(Quadric), m_alpha(1.0), m_beta(1.0), m_gamma(6.0) {};

    void create(int unused = -1);

    double alpha() const { return m_alpha; }
    double beta() const { return m_beta; }

    void setQuadric(double alpha, double beta)
    {
        m_mode = Quadric;
        m_alpha = alpha;
        m_beta = beta;
    }

    void setTensorGlyph(double cl, double cp)
    {
        m_mode = TensorGlyph;
        m_cl = cl;
        m_cp = cp;
    }

    void setTensorGlyphSharpness(double gamma) { m_gamma = gamma; }

  private:
    int m_mode;
    double m_alpha, m_beta;     ///< Quadric parameters
    double m_cl, m_cp, m_gamma; ///< Tensor glyph parameters
};
