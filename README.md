# gltoys

Some of my old realtime OpenGL effects as small stand-alone C++ programs to toy around with.

## Build

Dependencies are configured via a [vcpkg](https://vcpkg.io/) manifest, see `source/vcpkg.json`.

Generate build files via cmake e.g. for Windows x64:
```
> cmake -B build -S source -DCMAKE_TOOLCHAIN_FILE=<Path-to-VCPKG>\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
```

Tested on Windows 10.