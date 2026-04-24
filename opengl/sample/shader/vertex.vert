#version 410 core

layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
// layout (location = 1) in vec3 aColor; // the color variable has attribute position 1
layout (location = 1) in vec2 aTexCoord; // the texture variable has attribute position 2

// out vec3 ourColor; // output a color to the fragment shader
out vec2 TexCoord; // output a texture to the fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform float t;
uniform float theta;

void main()
{
    float c = cos(theta);
    float s = sin(theta);
    float t_c = cos(t);
    float t_s = sin(t);
    
    mat4 rot_xy = mat4(
        c, s, 0, 0,
        -s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    mat4 t_rot_xy = mat4(
        t_c, -t_s, 0, 0,
        t_s,  t_c, 0, 0,
        0,    0,   1, 0,
        0,    0,   0, 1
    );

    // Homogeneous coordinate for applying translations
    vec4 newPos = vec4(aPos, 1.0);

    gl_Position = vec4( proj * view * model * t_rot_xy * rot_xy * newPos); // Apply translation dot product then rotate
    // ourColor = vec3(t_rot_xy * aColor.xy, t_c * aColor.z + t_s * t_s * aColor.z); 
    TexCoord = aTexCoord;
}