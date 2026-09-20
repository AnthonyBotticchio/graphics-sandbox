#version 410 core

layout( location = 0 ) in vec3 aFragPos;
layout( location = 1 ) in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D groundTexture;
uniform float t;

void main()
{
    vec3 p = aFragPos;

    vec3 dpdx = dFdx( p );
    vec3 dpdy = dFdy( p );

    vec3 sun = normalize( vec3( sin( t * 0.4 ), 1.0, cos( t * 0.4 ) ) );
    vec3 n   = cross( dpdx, dpdy );

    n = normalize( n );

    vec3 rgb = texture( groundTexture, TexCoord * 5000 ).xyz;

    float f = 0.2 + max( dot( n, sun ), 0.f );

    FragColor = vec4( rgb * f, 1 );
}
