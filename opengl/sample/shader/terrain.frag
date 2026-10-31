#version 410 core

layout( location = 0 ) in vec3 aFragPos;
layout( location = 1 ) in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D terrainTexture;
uniform float phase;

#define TWO_PI 6.283185f

void main()
{
    vec3 p = aFragPos;

    vec3 dpdx = dFdx( p );
    vec3 dpdy = dFdy( p );

    vec3 sun = normalize( vec3( sin( phase ), 3 * cos( phase ), cos( phase ) ) );
    vec3 n   = cross( dpdx, dpdy );

    n = normalize( n );

    vec3 rgb = texture( terrainTexture, TexCoord * 5000 ).xyz;

    float direct  = max( dot( n, sun ), 0.f );
    float ambient = 0.5;

    if( sun.y <= -1.5f )
        direct = 0.0f;

    float f = ambient + direct;

    FragColor = vec4( rgb * f, 1 );
}
