# gltoys

Some of my old realtime OpenGL effects as small stand-alone C++ programs to toy around with.

Two of the toys are in a usable state:
- **toy-glitchsphere**

![386dx25-toy-glitchsphere-teaser](https://user-images.githubusercontent.com/40522065/224380494-5f5b5c64-23e1-42c6-8e8e-d72f4dc25a54.gif)

- **toy-mnoise**

![386dx25-toy-mnoise-teaser](https://user-images.githubusercontent.com/40522065/224380520-4b509900-55cc-4c10-8e58-fceb9a4ec9fc.gif)

Windows binaries can be found in [releases](https://github.com/shermnonic/gltoys/releases).

## Build

Dependencies are configured via a [vcpkg](https://vcpkg.io/) manifest, see `source/vcpkg.json`.

Generate build files via cmake e.g. for Windows x64:
```
> cmake -B build -S source -DCMAKE_TOOLCHAIN_FILE=<Path-to-VCPKG>\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
```

Tested on Windows 10.
