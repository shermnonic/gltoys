#include <geometry/Geometry2.h>

int main(int argc, char* argv[])
{
    Icosahedron icosahedron;
    icosahedron.create();
    icosahedron.writeOBJ("icosahedron.obj");
    return 0;
}
