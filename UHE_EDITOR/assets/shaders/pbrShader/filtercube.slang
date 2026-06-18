
struct VSInput
{
  float3 Pos;
}

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 UVM;
}

[shader["vertex"]]
VSOutput vertexMain(VSInput input,uniform float4x4 mvp)
{
    VSOutput output;
    output.UVW = input.Pos;
    output.Pos = mul(mvp,float4(input.Pos.xyz,1.0));
    return output;
}




