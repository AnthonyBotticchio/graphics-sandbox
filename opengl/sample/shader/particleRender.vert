#version 410 core

layout (location = 0) in vec3 iPosition;

uniform mat4 view;
uniform mat4 proj;

out vec2 localPos;

// vec2 mousePosition()
// {
//    return ( mouse.xy - 0.5 * res.xy ) / res.y;
// }

// bool mouseHeld()
// {
//     return mouse.z > 0.0;
// }

void main()
{
    // Four corners
    const vec2 corners[4] = vec2[4]( vec2( -1.0, -1.0 ), vec2( 1.0, -1.0 ), vec2( -1.0, 1.0 ), vec2( 1.0, 1.0 ) );

    float radius = 0.005; // mix( 0.001, 0.005, speed / MAX_SPEED );
    localPos     = corners[gl_VertexID];

    // 3D
    vec4 cameraPosition = view * vec4(iPosition, 1.0);
    cameraPosition.xy += radius * localPos;
    gl_Position = proj * cameraPosition;
}
