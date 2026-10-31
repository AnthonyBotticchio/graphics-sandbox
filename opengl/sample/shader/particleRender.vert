#version 410 core

layout( location = 0 ) in vec3 iPosition;

uniform mat4 view;
uniform mat4 projView;

out vec2 localPos;

void main()
{
    // Four corners
    const vec2 corners[4] = vec2[4]( vec2( -1.0, -1.0 ), vec2( 1.0, -1.0 ), vec2( -1.0, 1.0 ), vec2( 1.0, 1.0 ) );

    float radius = 0.005; // mix( 0.001, 0.005, speed / MAX_SPEED );
    localPos     = corners[gl_VertexID];

    // 3D - Billboarded
    vec3 offset = transpose( mat3( view ) ) * vec3( radius * localPos, 0.0 );
    gl_Position = projView * vec4( iPosition + offset, 1.0 );
}
