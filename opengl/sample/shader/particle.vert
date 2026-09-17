#version 410 core

uniform sampler2D particle_pos;
uniform sampler2D particle_vel;

void main()
{
    ivec2 size = textureSize(particle_pos, 0);

    ivec2 idx = ivec2(gl_InstanceID % size.x, gl_InstanceID / size.x);

    vec3 pos = texelFetch(particle_pos, idx).xyz;
}