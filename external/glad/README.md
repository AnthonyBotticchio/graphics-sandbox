# GLAD

Vendored output from GLAD 0.1.36, generated for OpenGL 4.1 core, C/C++, with
the loader enabled, KHR headers included, and no additional extensions.

Generator settings:
https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D4.1&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=core&loader=on

Equivalent command (using GLAD 0.1.36):

```sh
python3 -m glad --profile core --api gl=4.1 --generator c --spec gl --extensions '' --out-path external/glad
```

Generation downloads the current Khronos specifications. The generated files
are committed so normal builds need neither Python nor network access.
The sample loads functions through GLFW's `glfwGetProcAddress`.

See LICENSE and the copyright notice in include/KHR/khrplatform.h.
