#version 410 core

in vec2 localPos;
out vec4 FragColor;

void main()
{
    float d     = length( localPos );
    float edge  = fwidth( d );
    float alpha = 1.0 - smoothstep( 1.0 - edge, 1.0, d );

    if( alpha <= 0.0 || d >= 1.0 )
        discard; // These particles write depth, so we don't want alpha = 0 to fill the fragment and block further particles
    // gl_FragDepth = ( alpha > 0.0 ) ? gl_FragCoord.z : 1.0;

    FragColor = vec4( vec3( 0.75, 0.65, 0.45 ) * ( 1 - alpha ), 1.0 );
}
