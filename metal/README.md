# Metal triangle

A minimal macOS Metal sample written in C++ using metal-cpp and a GLFW window.
The existing C logger reports device selection and initialization errors.

From the repository root, using the project's configured build directory:

```sh
cmake -S . -B build
cmake --build build --target mtl-sample
./build/bin/mtl-sample
```

Install just this sample:

```sh
cmake --install build --component mtl-sample
./install/bin/mtl-sample
```

`sample/src/main.cpp` creates the window, render pipeline, and draw commands.
`sample/shader/triangle.metal` provides three colored vertices and a fragment
shader. The shader is compiled at startup; no offline Metal compiler is needed.
`sample/textures` is reserved for future textures (the triangle needs none).

Resources are copied into `build/shader` and `build/textures`, and installed into
`install/shader` and `install/textures`. Paths are resolved relative to the
executable, so launching from another working directory also works. Rebuild the
target after editing shaders to refresh the build copy. Close the window or
press Command-Q to quit.

The `metal-cpp` CMake interface target exposes `external/metal-cpp` and links the
Metal, Foundation, and QuartzCore frameworks. It does not build a library.
The implementation macros are defined once in `main.cpp`, following Apple's
[metal-cpp setup](https://developer.apple.com/metal/cpp/).

GLFW handles window creation and events with `GLFW_NO_API`. A small C++ helper
uses the Objective-C runtime's C interface to attach a `CA::MetalLayer` to the
window's content view; no Objective-C++ source or ARC is used by the sample.
Metal objects use `NS::SharedPtr` ownership and an autorelease pool per frame.
The draw loop waits for each frame to finish to keep synchronization simple.
