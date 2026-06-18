#pragma warning(disable: 41012)
struct VertexInput {
    [[vk::location(0)]] float3 a_Position : POSITION;
    [[vk::location(1)]] float4 a_Color : COLOR;
    [[vk::location(2)]] float2 a_TexCoord : TEXCOORD;
    [[vk::location(3)]] float a_TexIndex : TEXINDEX;
    [[vk::location(4)]] float a_TilingFactor : TILINGFACTOR;
    [[vk::location(5)]] float a_EntityID : ENTITYID;
};

struct VertexOutput {
    float4 location : SV_Position;
    [[vk::location(0)]] float4 v_Color;
    [[vk::location(1)]] float2 v_TexCoord;
    [[vk::location(2)]] nointerpolation float v_TexIndex;
    [[vk::location(3)]] float v_TilingFactor;
    [[vk::location(4)]] nointerpolation float v_EntityID;
};

struct PushConstants {
    float4x4 viewProjection;
    int textureIndices[32];
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
    
    int localIndex = (int)round(input.v_TexIndex);
    float2 uv = input.v_TexCoord * input.v_TilingFactor;
    float4 texColor = float4(1.0);

    switch(localIndex) {
        case 0: texColor = u_Textures[pc.textureIndices[0]].Sample(uv); break;
        case 1: texColor = u_Textures[pc.textureIndices[1]].Sample(uv); break;
        case 2: texColor = u_Textures[pc.textureIndices[2]].Sample(uv); break;
        case 3: texColor = u_Textures[pc.textureIndices[3]].Sample(uv); break;
        case 4: texColor = u_Textures[pc.textureIndices[4]].Sample(uv); break;
        case 5: texColor = u_Textures[pc.textureIndices[5]].Sample(uv); break;
        case 6: texColor = u_Textures[pc.textureIndices[6]].Sample(uv); break;
        case 7: texColor = u_Textures[pc.textureIndices[7]].Sample(uv); break;
        case 8: texColor = u_Textures[pc.textureIndices[8]].Sample(uv); break;
        case 9: texColor = u_Textures[pc.textureIndices[9]].Sample(uv); break;
        case 10: texColor = u_Textures[pc.textureIndices[10]].Sample(uv); break;
        case 11: texColor = u_Textures[pc.textureIndices[11]].Sample(uv); break;
        case 12: texColor = u_Textures[pc.textureIndices[12]].Sample(uv); break;
        case 13: texColor = u_Textures[pc.textureIndices[13]].Sample(uv); break;
        case 14: texColor = u_Textures[pc.textureIndices[14]].Sample(uv); break;
        case 15: texColor = u_Textures[pc.textureIndices[15]].Sample(uv); break;
        case 16: texColor = u_Textures[pc.textureIndices[16]].Sample(uv); break;
        case 17: texColor = u_Textures[pc.textureIndices[17]].Sample(uv); break;
        case 18: texColor = u_Textures[pc.textureIndices[18]].Sample(uv); break;
        case 19: texColor = u_Textures[pc.textureIndices[19]].Sample(uv); break;
        case 20: texColor = u_Textures[pc.textureIndices[20]].Sample(uv); break;
        case 21: texColor = u_Textures[pc.textureIndices[21]].Sample(uv); break;
        case 22: texColor = u_Textures[pc.textureIndices[22]].Sample(uv); break;
        case 23: texColor = u_Textures[pc.textureIndices[23]].Sample(uv); break;
        case 24: texColor = u_Textures[pc.textureIndices[24]].Sample(uv); break;
        case 25: texColor = u_Textures[pc.textureIndices[25]].Sample(uv); break;
        case 26: texColor = u_Textures[pc.textureIndices[26]].Sample(uv); break;
        case 27: texColor = u_Textures[pc.textureIndices[27]].Sample(uv); break;
        case 28: texColor = u_Textures[pc.textureIndices[28]].Sample(uv); break;
        case 29: texColor = u_Textures[pc.textureIndices[29]].Sample(uv); break;
        case 30: texColor = u_Textures[pc.textureIndices[30]].Sample(uv); break;
        case 31: texColor = u_Textures[pc.textureIndices[31]].Sample(uv); break;
    }

    float4 finalColor = texColor * input.v_Color;

    if (finalColor.a < 0.01) {
        discard;
    }
    output.color = finalColor;
    output.entityID = (int)round(input.v_EntityID);
    
    return output;
}
