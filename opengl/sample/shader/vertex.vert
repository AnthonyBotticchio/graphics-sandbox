#version 410 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aColor; // the color variable has attribute position 1

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

    mat2 rot_mat = mat2(
        cos(theta), -sin(theta), 
        sin(theta), cos(theta)
    );


    // vec2 rot = vec2(aPos.x * c - aPos.y * s, aPos.x * s + aPos.y * c);

    ourColor = vec3(aColor.x * t_c - aColor.y * t_s, aColor.y * t_c + aColor.x * t_s, aColor.z); // set ourColor to the input color we got from the vertex data
    gl_Position = vec4(rot_mat * aPos.xy + offset, aPos.z, 1.0);
}       