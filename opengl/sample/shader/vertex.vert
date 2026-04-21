#version 410 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1
// layout (location = 2) in vec3 tex;

out vec3 ourColor; // output a color to the fragment shader

uniform float t;
uniform vec2 offset;
uniform float theta;

void main()
{
    float c = cos(theta);
    float s = sin(theta);
    float t_c = cos(t);
    float t_s = sin(t);
    
    mat2 rot = mat2(
        c, -s,
        s,  c
    );

    mat2 t_rot = mat2(
        t_c, -t_s,
        t_s,  t_c
    );

    ourColor = vec3(t_rot * aColor.xy, t_c * aColor.z + t_s * t_s * aColor.z); // set ourColor to the input color we got from the vertex data
    gl_Position = vec4(rot * aPos.xy + offset, aPos.z, 1.0);
}