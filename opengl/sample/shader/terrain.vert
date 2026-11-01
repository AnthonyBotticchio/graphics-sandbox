#version 410 core

// layout( location = 0 ) in vec3 aPos;      // the position variable has attribute position 0
// layout( location = 1 ) in vec2 aTexCoord; // the texture variable has attribute position 2

uniform sampler2D heightmap;
uniform vec2 terrainMin;
uniform float terrainSize;
uniform int meshCells;

uniform mat4 projView;
uniform mat4 model;
uniform float t;

layout( location = 0 ) out vec3 aFragPos;
layout( location = 1 ) out vec2 TexCoord;
layout( location = 2 ) out float out_t;

void main()
{
    int side        = meshCells + 1; // x
    ivec2 grid      = ivec2( gl_VertexID % side, gl_VertexID / side );
    vec2 resolution = vec2( textureSize( heightmap, 0 ) );
    vec2 uv         = vec2( grid ) / float( meshCells );
    vec2 xz         = terrainMin + uv * terrainSize;
    vec2 heightUV   = ( uv * ( resolution - 1.0 ) + 0.5 ) / resolution;
    float height    = textureLod( heightmap, heightUV, 0.0 ).r;
    vec3 position   = vec3( xz.x, height, xz.y );
    vec4 wpos       = model * vec4( position, 1.0 );

    aFragPos = wpos.xyz;
    TexCoord = xz / 4.0f;
    out_t    = t;

    gl_Position = projView * wpos;
}
