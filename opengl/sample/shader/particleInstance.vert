#version 410 core

uniform mat4 view;
uniform mat4 proj;

uniform vec3 res;
uniform float t;
uniform vec4 mouse;

out vec2 localPos;

#define MAX_SPEED 0.50
#define MIN_SPEED 0.10

float hash( float n )
{
    return fract( sin( n ) * 43758.5453 );
}

vec2 mousePosition()
{
    return ( mouse.xy - 0.5 * res.xy ) / res.y;
}

bool mouseHeld()
{
    return mouse.z > 0.0;
}

void main()
{
    // Four corners
    const vec2 corners[4] = vec2[4]( vec2( -1.0, -1.0 ), vec2( 1.0, -1.0 ), vec2( -1.0, 1.0 ), vec2( 1.0, 1.0 ) );
    float id              = float( gl_InstanceID );
    float aspect          = res.x / res.y;
    // Note that x: [-aspect/2, aspect/2], y: [-0.5, 0.5]
    vec3 pos = vec3( ( hash( id * 13.7 ) - 0.5 ) * 2, ( hash( id * 27.3 ) - 0.5 ) * 2, ( hash( id * 10.1 ) - 0.5 ) * 2 );
    // TODO: pos and vel needs to be updated with a transform feedback pass
    // vec2 pos = texelFetch( u_pos, gl_InstanceID ).xy;
    float speed = mix( MAX_SPEED, MIN_SPEED, hash( id * 41.9 ) );
    float phase = hash( id * 17.3 ) * 6.2831;
    float angle = phase + t;
    vec2 mouse  = mousePosition();

    pos += vec3( sin( angle ), -cos( angle ), sin( angle * 0.73 + hash( id * 7.1 ) * 6.283185 ) ) * speed * 0.5;

    if( mouseHeld() )
    {
        vec2 dir             = mouse.xy - pos.xy;
        float dist           = length( dir );
        const float strength = 0.5;

        if( dist > 0.001 )
            pos.xy += ( dir / dist ) * strength;
    }

    float radius = mix( 0.001, 0.005, speed / MAX_SPEED );
    localPos     = corners[gl_VertexID];


    // 3D
    mat4 model  = mat4( 1.0 );
    model[0][0] = radius;
    model[1][1] = radius;
    model[2][2] = radius;
    model[3]    = vec4( vec3( 0 ), 1.0 ); // Shift worldPos

    vec4 cpos = view * vec4( pos, 1 );
    cpos.xy += radius * localPos;

    // out_pos.xy += corners[gl_VertexID] * screen_w;

    gl_Position = proj * cpos; // proj * view * model * vec4( localPos, 0.0, 1.0 );
}