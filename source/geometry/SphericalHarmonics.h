#pragma once
#include <geometry/Icosahedron.h>

class SphericalHarmonics : public Icosahedron
{
  public:
    SphericalHarmonics();
    
    void create(int level = -1) override;
    void update() override;
    
    void setLM(int l, int m);
    int getL() const { return m_l; }
    int getM() const { return m_m; }

  private:
    int m_l = 4;
    int m_m = 0;
};
