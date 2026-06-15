struct VertexInput {
    [[vk::location(0)]] float3 a_Position : POSITION;
    [[vk::location(1)]] float3 a_Normal : NORMAL;
    [[vk::location(2)]] float2 a_TexCoord : TEXCOORD;
};

struct VertexOutput {
    float4 location : SV_Position;
    [[vk::location(0)]] float3 v_Normal;
    [[vk::location(1)]] float2 v_TexCoord;
    [[vk::location(2)]] float3 v_WorldPos;
};

struct PushConstants {
    float4x4 viewProjection;
    float4x4 model;
    int entityID;
    int textureSlot;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

[[vk::binding(1, 0)]] Sampler2D u_Textures[10000];

struct FragmentOutput {
    [[vk::location(0)]] float4 Color : SV_Target0;
    [[vk::location(1)]] int EntityID : SV_Target1;
};


[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    
    float4 worldPos = mul(pc.model, float4(input.a_Position, 1.0));
    output.v_WorldPos = worldPos.xyz;
    output.v_Normal = normalize(mul((float3x3)pc.model, input.a_Normal));
    output.v_TexCoord = input.a_TexCoord;
    
    output.location = mul(pc.viewProjection, worldPos);
    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input) {
    FragmentOutput output;
    
    float4 texColor = u_Textures[pc.textureSlot].Sample(input.v_TexCoord);
    
    float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
    float ndotl = max(dot(normalize(input.v_Normal), lightDir), 0.1); // 0.1 ambient
    
    float3 finalColor = texColor.rgb * ndotl;
    
    output.Color = float4(finalColor, texColor.a);
    output.EntityID = pc.entityID;
    
    return output;
}
