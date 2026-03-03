#include <geometry/Geometry2.h>
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Write icosahedron...\n";
    Icosahedron icosahedron;
    icosahedron.create(1);
    icosahedron.writeOBJ("icosahedron-1.obj");

    std::cout << "Write penrose...\n";
    Penrose penrose;
    penrose.create();
    penrose.writeOBJ("penrose.obj");

    std::cout << "Write superquadric...\n";
    Superquadric superquadric;
    superquadric.create();
    superquadric.writeOBJ("superquadric.obj");

    std::cout << "Write sh...\n";
    SphericalHarmonics sh;
    sh.create();
    sh.writeOBJ("sh.obj");

    std::cout << "Write shf...\n";
    SHF shf;
    shf.create();
    shf.writeOBJ("shf.obj");

    return 0;
}
