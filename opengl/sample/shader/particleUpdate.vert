#version 410 core

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iVelocity;

uniform float dt;
uniform vec3 acceleration;
uniform vec3 mouseTarget;
uniform bool mouseActive;
uniform float attractionStrength;

uniform sampler2D heightmap;
uniform vec2 terrainMin;
uniform float terrainSize;

out vec3 oPosition;
out vec3 oVelocity;

float elevationAt( vec2 xz )
{
    // Where 0 is the minimum mipmap level - full resolution. textureSize returns an ivec2, so we cast to vec2
    vec2 resolution = vec2( textureSize( heightmap, 0 ) );
    vec2 normalized = ( xz - terrainMin ) / terrainSize;
    vec2 grid       = normalized * ( resolution - 1 );
    vec2 uv         = ( grid + 0.5 ) / resolution;

    return textureLod( heightmap, uv, 0.0 ).r;
}

vec3 normalAt( vec2 p )
{
    vec2 resolution = vec2( textureSize( heightmap, 0 ) );
    vec2 spacing    = vec2( terrainSize ) / ( resolution - 1.0 );

    float l = elevationAt( p + vec2( -spacing.x, 0 ) );
    float r = elevationAt( p + vec2( spacing.x, 0 ) );
    float b = elevationAt( p + vec2( 0, -spacing.y ) );
    float t = elevationAt( p + vec2( 0, spacing.y ) );

    float dhdx = ( r - l ) / ( 2.0 * spacing.x );
    float dhdy = ( t - b ) / ( 2.0 * spacing.y );

    return normalize( vec3( -dhdx, 1.0, -dhdy ) );
}

void main()
{
    const float drag       = 0.999;
    vec3 totalAcceleration = acceleration;
    if( mouseActive )
    {
        vec3 offset = mouseTarget - iPosition;
        // Soften the center: finite force, tending to zero at the target.
        const float softening = 1.2;
        totalAcceleration += attractionStrength * offset / sqrt( dot( offset, offset ) + softening * softening );
    }

    // With the mouse released, retain momentum instead of resetting velocity.
    oVelocity = ( iVelocity + totalAcceleration * dt ) * drag;
    oPosition = iPosition + oVelocity * dt;

    const float damping = 0.8;
    vec2 terrainMax = terrainMin + vec2( terrainSize );
    bool insideTerrain = all( greaterThanEqual( oPosition.xz, terrainMin ) ) &&
                         all( lessThanEqual( oPosition.xz, terrainMax ) );
    float terrainY = elevationAt( oPosition.xz );
    if( insideTerrain && oPosition.y < terrainY )
    {
        oPosition.y = terrainY;
        vec3 normal = normalAt( oPosition.xz );
        float vn = dot( oVelocity, normal );
        if( vn < 0.0 )
            oVelocity -= ( 1.0 + damping ) * vn * normal;
    }

    gl_Position = vec4( oPosition, 1.0 );
}
