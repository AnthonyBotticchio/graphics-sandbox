#version 410 core

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iVelocity;

uniform float dt;
uniform vec3 acceleration;
uniform vec3 mouseTarget;
uniform bool mouseActive;
uniform float attractionStrength;

#define MAX_LOADED_GRID_WIDTH 3

uniform vec2 heightmapBottomLeft;
uniform float heightmapScale;
uniform sampler2DArray heightmaps;

out vec3 oPosition;
out vec3 oVelocity;

vec3 getTexCoord( vec2 p )
{
    vec2 gridPos = clamp( ( p - heightmapBottomLeft ) / heightmapScale, vec2( 0 ), vec2( 3 - 1e-6 ) );
    return vec2( fract( gridPos.x ), gridPos.y * MAX_LOADED_GRID_WIDTH + gridPos.x );
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

    float dstElevation = elevationAt( oPosition.xy );

    const float damping = 0.8;

    // We will collide with the surface
    vec3 tCollision = 0.5;
    if( oPosition.z < dstElevation )
    {
        // for (int i ;...  Do newton's method iterations for better accuracy

        vec3 collisionPos = mix( iPosition, oPosition, tCollision );
        vec3 normal       = normalAt( collisionPos.xy );

        oPosition = collisionPos;
        oVelocity = oVelocity - ( 1.0 + damping ) * dot( oVelocity, normal ) * normal;
    }

    gl_Position = vec4( oPosition, 1.0 );
}
