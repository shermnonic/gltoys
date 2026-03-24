# gltoys

Some of my old realtime OpenGL effects as small stand-alone C++ programs to toy around with.

Three of the toys are in a usable state:
- **toy-geometry**
- **toy-glitchsphere**
- **toy-mnoise**

![386dx25-toy-glitchsphere-teaser](https://user-images.githubusercontent.com/40522065/224380494-5f5b5c64-23e1-42c6-8e8e-d72f4dc25a54.gif)

Windows binaries can be found in [releases](https://github.com/shermnonic/gltoys/releases).

## Build

Requires:
- [cmake](https://cmake.org/) for build system
- [vcpkg](https://vcpkg.io/) for dependency management
  - Set `VCPKG_ROOT` environment variable to the vcpkg directory

Build on Linux via:
```bash
cmake --preset default
cmake --build build/default --config Release
```

Launch the toys from the build tree:
```bash
./build/default/source/toy-geometry
./build/default/source/toy-glitchsphere
./build/default/source/toy-mnoise
```
