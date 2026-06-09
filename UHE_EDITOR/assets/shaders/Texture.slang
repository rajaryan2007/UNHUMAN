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

[[vk::binding(1, 0)]] Sampler2D u_Textures[32];

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
    float4 texColor = float4(1.0);
    int index = (int)input.v_TexIndex;

    switch(index) {
        case 0: texColor = u_Textures[0].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 1: texColor = u_Textures[1].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 2: texColor = u_Textures[2].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 3: texColor = u_Textures[3].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 4: texColor = u_Textures[4].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 5: texColor = u_Textures[5].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 6: texColor = u_Textures[6].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 7: texColor = u_Textures[7].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 8: texColor = u_Textures[8].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 9: texColor = u_Textures[9].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 10: texColor = u_Textures[10].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 11: texColor = u_Textures[11].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 12: texColor = u_Textures[12].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 13: texColor = u_Textures[13].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 14: texColor = u_Textures[14].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 15: texColor = u_Textures[15].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 16: texColor = u_Textures[16].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 17: texColor = u_Textures[17].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 18: texColor = u_Textures[18].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 19: texColor = u_Textures[19].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 20: texColor = u_Textures[20].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 21: texColor = u_Textures[21].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 22: texColor = u_Textures[22].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 23: texColor = u_Textures[23].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 24: texColor = u_Textures[24].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 25: texColor = u_Textures[25].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 26: texColor = u_Textures[26].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 27: texColor = u_Textures[27].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 28: texColor = u_Textures[28].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 29: texColor = u_Textures[29].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 30: texColor = u_Textures[30].Sample(input.v_TexCoord * input.v_TilingFactor); break;
        case 31: texColor = u_Textures[31].Sample(input.v_TexCoord * input.v_TilingFactor); break;
    }
    
    output.color = texColor * input.v_Color;
    output.entityID = input.v_EntityID;
    
    return output;
}
