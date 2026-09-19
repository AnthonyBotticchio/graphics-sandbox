#version 410 core

#define MAX_LOADED_GRID_WIDTH 3
#define MAX_LOADED_GRID_SIZE  ( MAX_LOADED_GRID_WIDTH * MAX_LOADED_GRID_WIDTH )

uniform int verticesPerSide;
uniform vec2 heightmapBottomLeft;
uniform float heightmapScale;
uniform sampler2DArray heightmaps;

out vec3 oPosition;
out vec3 oNormal;

vec3 getTexCoord( vec2 p )
{
    vec2 gridPos = ( p - heightmapBottomLeft ) / heightmapScale;
    return vec2( fract( gridPos ), gridPos.y * MAX_LOADED_GRID_WIDTH + gridPos.x );
}

float elevationAt( vec3 p )
{
    vec3 texc = getTexCoord( p );
    return texture( heightmap, texc ).r;
}

vec3 normalAt( vec2 p )
{
    vec3 texc = getTexCoord( p );
    vec2 h    = vec2( 1.f ) / vec2( textureSize( heightmap ) );

    float l = texture( heightmap, texc + vec3( -h.x, 0, 0 ) );
    float r = texture( heightmap, texc + vec3( h.x, 0, 0 ) );
    float b = texture( heightmap, texc + vec3( 0, -h.y, 0 ) );
    float t = texture( heightmap, texc + vec3( 0, h.y, 0 ) );

    vec2 grad = vec2( r - l, t - b ) / ( heightmapScale * h );

    return normalize( -grad.x, -grad.y, 1 );
}

void main()
{
    int ix = gl_VertexID % verticesPerSide;
    int iy = gl_VertexID / verticesPerSide;

    vec2 gridPos = 3 * vec2( float( ix ), float( iy ) ) / float( verticesPerSide );

    vec2 p = heightmapBottomLeft + heightmapScale * gridPos;

    oPosition = vec3( p, elevationAt( p ) );
    oNormal   = normalAt( p );

    gl_Positions = proj * view * vec4( oPosition, 1 );
}