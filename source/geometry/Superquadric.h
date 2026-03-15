#pragma once
#include <geometry/SimpleGeometry.h>

class Superquadric : public SimpleGeometry
{
public:
    enum class Mode
    {
        Quadric,
        TensorGlyph
    };

    struct QuadricParameters
    {
        double alpha = 1.0;
        double beta = 1.0;
    };

    struct TensorParameters
    {
        double planarity = 0.0;
        double linearity = 0.0;
        double sharpness = 6.0;
    };

    Superquadric(Mode mode = Mode::Quadric)
    : mode(mode)
    {
        create();
    };

    void create(int unused = -1);

    Mode getMode() const { return mode; }

    double getAlpha() const { return quadricParameters.alpha; }
    double getBeta() const { return quadricParameters.beta; }

    void setQuadric(double alpha, double beta)
    {
        mode = Mode::Quadric;
        quadricParameters.alpha = alpha;
        quadricParameters.beta = beta;
    }

    void setParameters(TensorParameters newTensorParameters)
    {
        mode = Mode::TensorGlyph;
        tensorParameters = newTensorParameters;
    }

    TensorParameters getTensorParameters() const
    {
        return tensorParameters;
    }

  private:
    Mode mode = Mode::Quadric;
    QuadricParameters quadricParameters;
    TensorParameters tensorParameters;
};
