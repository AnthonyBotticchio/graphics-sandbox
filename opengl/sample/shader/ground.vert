#version 410 core

layout( location = 0 ) in vec3 aPos;      // the position variable has attribute position 0
layout( location = 1 ) in vec2 aTexCoord; // the texture variable has attribute position 2

layout( location = 0 ) out vec3 aFragPos;
layout( location = 1 ) out vec2 TexCoord;
layout( location = 2 ) out float out_t;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform float t;

void main()
{
    float t_c = cos( t );
    float t_s = sin( t );

    vec4 wpos = model * vec4( aPos, 1.0 );

    aFragPos = wpos.xyz;
    TexCoord = aTexCoord;
    out_t = t;

    gl_Position = proj * view * wpos;
}
