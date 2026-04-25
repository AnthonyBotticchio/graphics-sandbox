float oscilate(float t)
{
    return 0.5 + 0.5 * cos(t);
}

float mouseInput() 
{
    return iMouse.x / iResolution.x;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy * 2.0 - 1.0; // Clamp
    float aspect = iResolution.x / iResolution.y;
    uv.x *= aspect + oscilate(iTime); // Square
    // uv.y *= oscilate(iTime) + 10.0;
    
    fragColor.rg = uv;
    fragColor.b = 0.0;
    
    float thickness = mouseInput();
    float fade = 0.01;
    float distance = 1.0 - length(uv);
    vec3 color = vec3(smoothstep(0.0, fade, distance));
    color *= vec3(smoothstep(thickness + fade, thickness, distance));
    
    // Output to screen
    fragColor = vec4(color, distance);
    fragColor.rgb *= vec3(0.8, 0.5, 0.5);
}