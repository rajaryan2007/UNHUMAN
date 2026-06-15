struct VertexInput {
    [[vk::location(0)]] float3 a_Position : POSITION;
    [[vk::location(1)]] float4 a_Color : COLOR;
    [[vk::location(2)]] float2 a_TexCoord : TEXCOORD;
    [[vk::location(3)]] float a_TexIndex : TEXINDEX;
    [[vk::location(4)]] float a_TilingFactor : TILINGFACTOR;
    [[vk::location(5)]] int a_EntityID : ENTITYID;
};

struct VertexOutput {
    float4 location : SV_Position;
    [[vk::location(0)]] float4 v_Color;
    [[vk::location(1)]] float2 v_TexCoord;
    [[vk::location(2)]] nointerpolation float v_TexIndex;
    [[vk::location(3)]] float v_TilingFactor;
    [[vk::location(4)]] nointerpolation int v_EntityID;
};

struct PushConstants {
    float4x4 viewProjection;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

[[vk::binding(1, 0)]] Sampler2D u_Textures[10000];

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    output.v_Color = input.a_Color;
    output.v_TexCoord = input.a_TexCoord;
    output.v_TexIndex = input.a_TexIndex;
    output.v_TilingFactor = input.a_TilingFactor;
    output.v_EntityID = input.a_EntityID;
    
    output.location = mul(pc.viewProjection, float4(input.a_Position, 1.0));
    return output;
}

struct FragmentOutput {
    [[vk::location(0)]] float4 color : SV_Target0;
    [[vk::location(1)]] int entityID : SV_Target1;
};

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input) {
    FragmentOutput output;
    
    int index = (int)input.v_TexIndex;
    float4 texColor = u_Textures[index].Sample(input.v_TexCoord * input.v_TilingFactor);
    
    output.color = texColor * input.v_Color;
    output.entityID = input.v_EntityID;
    
    return output;
}
