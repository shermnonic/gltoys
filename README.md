# gltoys

Some of my old realtime OpenGL effects as small stand-alone C++ programs to toy around with.

## Build Instructions

Install dependencies via [vcpkg](https://vcpkg.io/)
```
> vcpkg install glew glfw3 imgui glm
```

Generate build files via cmake 
```
> cmake -S source -B build -DCMAKE_TOOLCHAIN_FILE=<Path-to-VCPKG>\scripts\buildsystems\vcpkg.cmake
```
(See also [vcpkg cmake integration notes](https://github.com/microsoft/vcpkg/blob/master/docs/users/integration.md).)

Tested on Windows 10.