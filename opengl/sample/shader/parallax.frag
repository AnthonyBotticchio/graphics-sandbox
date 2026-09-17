#version 410 core

out vec4 FragColor;

uniform vec3 res;
uniform vec4 mouse;
uniform float t;

#define PARTICLE_COUNT 100

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

vec2 mousePosition()
{
    return (mouse.xy - 0.5 * res.xy) / res.y;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    float s_t = sin(t);
    float c_t = cos(t);
    float aspect = res.x / res.y;
    // Center coordinates around (0, 0).
    // Correct for screen aspect ratio so circles stay circular.
    vec2 uv = (fragCoord - 0.5 * res.xy) / res.y;

    vec3 color = vec3(0.0);

    for (int i = 0; i < PARTICLE_COUNT; i++)
    {
        float id = float(i);

        // Give every particle deterministic pseudo-random properties.
        float x = (hash(id * 13.7) - 0.5) * aspect;
        float y = hash(id * 27.3) - 0.5;

        float speed = mix(
            0.10,
            0.50,
            hash(id * 41.9)
        );

        // Move upward continuously.
        y += mousePosition().y * speed;
        x += mousePosition().x * speed;
    
        // Wrap back to the bottom.
        // y = fract(y);
        

        // Convert 0..1 into roughly -0.5..0.5.
        y = clamp(y, -0.5, 0.5);
        x = clamp(x, -aspect/2.0, aspect/2.0);

        vec2 pos = vec2(x, y);

        // Distance from this pixel to the particle.
        float d = length(uv - pos);

        // Particle radius.
        float radius = 0.008;
        
        // if(i % 2 == 0)
        //    radius *= sin(iTime);

        // Hard-ish circular core.
        float particle =
            1.0 - smoothstep(radius, radius + 0.003, d);

        color += vec3(particle);
    }

    fragColor = vec4(color, 1.0);
}

void main()
{
    mainImage(FragColor, gl_FragCoord.xy);
}

