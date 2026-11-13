#include <metal_stdlib>
using namespace metal;

struct VertexOutput
{
    float4 position [[position]];
    float3 color;
};

vertex VertexOutput vertexMain(uint vertexID [[vertex_id]],
                               constant float4x4& viewProjection [[buffer(0)]])
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

    return { viewProjection * float4(positions[vertexID], 0.0, 1.0), colors[vertexID] };
}

fragment float4 fragmentMain(VertexOutput in [[stage_in]], constant float& t [[buffer(0)]])
{
    return float4(in.color * float3(0.5f + 0.5f * cos(t), sin(t), -sin(t) * cos(t)), 1.0);
}
