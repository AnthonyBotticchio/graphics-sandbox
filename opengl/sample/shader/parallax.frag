#version 410 core

out vec4 FragColor;

uniform vec3 res;
uniform vec4 mouse;
uniform float t;

#define PARTICLE_COUNT 100

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

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    const float maxSpeed = 0.50;
    const float minSpeed = 0.10;

    float s_t    = sin( t );
    float c_t    = cos( t );
    float aspect = res.x / res.y;
    vec2 mouse   = mousePosition();
    // Center coordinates around (0, 0).
    // Correct for screen aspect ratio so circles stay circular.
    vec2 uv = ( fragCoord - 0.5 * res.xy ) / res.y;

    vec3 color = vec3( 0.0 );

    for( int i = 0; i < PARTICLE_COUNT; i++ )
    {
        float id = float( i );

        // Give every particle deterministic pseudo-random properties.
        float phase = hash( id * 17.3 ) * 6.235;
        vec2 pos    = vec2( ( hash( id * 13.7 ) - 0.5 ) * aspect, hash( id * 27.3 ) - 0.5 );
        float speed = mix( minSpeed, maxSpeed, hash( id * 41.9 ) );

        float angle = phase + t;
        pos += vec2( sin( angle ), -cos( angle ) ) * speed * 0.5;

        if( mouseHeld() )
        {
            vec2 dir             = mouse - pos;
            float dist           = length( dir );
            const float strength = 0.5;

            if( dist > 0.001 )
                pos += ( dir / dist ) * strength;
        }

        // Distance from this pixel to the particle.
        float d = length( uv - pos );

        // Particle radius.
        float radius = mix(0.002, 0.015, speed / maxSpeed);

        // Hard-ish circular core.
        float particle = 1.0 - smoothstep( radius, radius + 0.005, d );

        color += vec3( particle );
    }

    fragColor = vec4( color, 1.0 );
}

void main()
{
    mainImage( FragColor, gl_FragCoord.xy );
}
