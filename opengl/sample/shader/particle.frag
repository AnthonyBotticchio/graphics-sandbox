#version 410 core

in vec2 localPos;
out vec4 FragColor;

void main()
{
    float d     = length( localPos );
    float edge  = fwidth( d );
    float alpha = 1.0 - smoothstep( 1.0 - edge, 1.0, d );

    // gl_FragDepth = ( alpha > 0.0 ) ? gl_FragCoord.z : 1.0;

    FragColor = vec4( vec3( 0.76, 0.65, 0.45 ) * ( 1 - alpha ), alpha );
}