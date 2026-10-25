#include <metal_stdlib>
using namespace metal;

struct VertexOutput
{
    float4 position [[position]];
    float3 color;
};

vertex VertexOutput triangleVertex(uint vertexID [[vertex_id]])
{
    const float2 positions[] = {
        float2( 0.0,  0.7),
        float2(-0.7, -0.7),
        float2( 0.7, -0.7)
    };
    const float3 colors[] = {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 1.0, 1.0),
        float3(0.0, 0.0, 1.0)
    };

    return { float4(positions[vertexID], 0.0, 1.0), colors[vertexID] };
}

fragment float4 triangleFragment(VertexOutput in [[stage_in]])
{
    return float4(in.color, 1.0);
}
