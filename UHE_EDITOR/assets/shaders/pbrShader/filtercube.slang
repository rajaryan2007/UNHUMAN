
struct VSInput
{
    float3 Pos;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 UVW;
};

struct PushConstants {
    float4x4 mvp;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    output.UVW = input.Pos;
    output.Pos = mul(pc.mvp, float4(input.Pos, 1.0));
    return output;
}
