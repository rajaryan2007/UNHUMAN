#pragma warning(disable: 41012)
struct VertexInput {
    [[vk::location(0)]] float3 a_Position : POSITION;
    [[vk::location(1)]] float3 a_Normal : NORMAL;
    [[vk::location(2)]] float2 a_TexCoord : TEXCOORD;
    [[vk::location(3)]] int4 a_Joints : JOINTS;
    [[vk::location(4)]] float4 a_Weights : WEIGHTS;
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
    float4 cameraPos;
    int entityID;
    int textureSlot;
    int enableLighting;
    int lightBufferIndex;
    int numLights;
    int mrTextureSlot;
    float metallicFactor;
    float roughnessFactor;
    int boneBufferIndex;
    int boneOffset;
    int padding1;
    int padding2;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

struct LightData {
    float4 type_radius_pad; // x = type, y = radius
    float4 position_or_direction;
    float4 color_intensity;
};

[[vk::binding(0, 0)]] StructuredBuffer<LightData> u_LightBuffers[10000];
[[vk::binding(1, 0)]] Sampler2D u_Textures[10000];
[[vk::binding(0, 0)]] StructuredBuffer<float4x4> u_BoneBuffers[10000];

struct FragmentOutput {
    [[vk::location(0)]] float4 Color : SV_Target0;
    [[vk::location(1)]] int EntityID : SV_Target1;
};


[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    
    float4 localPos = float4(input.a_Position, 1.0);
    float3 localNormal = input.a_Normal;
    
    if (pc.boneBufferIndex >= 0 && pc.boneOffset >= 0)
    {
        StructuredBuffer<float4x4> boneBuffer = u_BoneBuffers[pc.boneBufferIndex];
        
        float4x4 skinMat = 
            boneBuffer[pc.boneOffset + input.a_Joints.x] * input.a_Weights.x +
            boneBuffer[pc.boneOffset + input.a_Joints.y] * input.a_Weights.y +
            boneBuffer[pc.boneOffset + input.a_Joints.z] * input.a_Weights.z +
            boneBuffer[pc.boneOffset + input.a_Joints.w] * input.a_Weights.w;
            
        localPos = mul(skinMat, float4(input.a_Position, 1.0));
        localNormal = mul((float3x3)skinMat, input.a_Normal);
    }
    
    float4 worldPos = mul(pc.model, localPos);
    output.v_WorldPos = worldPos.xyz;
    output.v_Normal = normalize(mul((float3x3)pc.model, localNormal));
    output.v_TexCoord = input.a_TexCoord;
    
    output.location = mul(pc.viewProjection, worldPos);
    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input) {
    FragmentOutput output;
    
    float4 texColor = u_Textures[pc.textureSlot].Sample(input.v_TexCoord);
    
    float3 finalColor = texColor.rgb;
    
    if (pc.enableLighting == 1) {
        float3 litColor = float3(0.0);
        float3 normal = normalize(input.v_Normal);
        float3 V = normalize(pc.cameraPos.xyz - input.v_WorldPos);
        
        // Use material properties and texture
        float metallic = pc.metallicFactor;
        float roughness = pc.roughnessFactor;
        
        if (pc.mrTextureSlot >= 0) {
            float4 mrTex = u_Textures[pc.mrTextureSlot].Sample(input.v_TexCoord);
            // In glTF, B channel is metallic, G channel is roughness
            metallic *= mrTex.b;
            roughness *= mrTex.g;
        }
        
        // Clamp roughness to prevent divide by zero
        roughness = max(roughness, 0.04);
        
        float3 F0 = float3(0.04);
        F0 = lerp(F0, texColor.rgb, metallic);
        
        float PI = 3.14159265359;
        float3 Lo = float3(0.0);
        
        for (int i = 0; i < pc.numLights; ++i) {
            LightData light = u_LightBuffers[pc.lightBufferIndex][i];
            
            int type = int(light.type_radius_pad.x);
            float radius = light.type_radius_pad.y;

            float3 L;
            float attenuation = 1.0;
            
            if (type == 0) { // Directional Light
                L = normalize(-light.position_or_direction.xyz);
            } 
            else if (type == 1) { // Point Light
                L = light.position_or_direction.xyz - input.v_WorldPos;
                float distance = length(L);
                L = normalize(L);
                
                attenuation = max(0.0, 1.0 - (distance / radius));
                attenuation *= attenuation; // quadratic falloff
            }
            
            float3 H = normalize(V + L);
            float3 radiance = light.color_intensity.xyz * light.color_intensity.w * attenuation;
            
            // Cook-Torrance BRDF
            // NDF (Distribution GGX)
            float a = roughness * roughness;
            float a2 = a * a;
            float NdotH = max(dot(normal, H), 0.0);
            float NdotH2 = NdotH * NdotH;
            
            float num = a2;
            float denom = (NdotH2 * (a2 - 1.0) + 1.0);
            denom = PI * denom * denom;
            float NDF = num / max(denom, 0.0000001);
            
            // Geometry (Smith)
            float r = (roughness + 1.0);
            float k = (r * r) / 8.0;
            
            float NdotV = max(dot(normal, V), 0.0);
            float NdotL = max(dot(normal, L), 0.0);
            
            float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
            float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
            float G = ggx1 * ggx2;
            
            // Fresnel (Schlick)
            float cosTheta = max(dot(H, V), 0.0);
            float3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
            
            float3 numerator = NDF * G * F;
            float denominator = 4.0 * NdotV * NdotL + 0.0001;
            float3 specular = numerator / denominator;
            
            float3 kS = F;
            float3 kD = float3(1.0) - kS;
            kD *= 1.0 - metallic;
            
            Lo += (kD * texColor.rgb / PI + specular) * radiance * NdotL;
        }
        
        float3 ambient = float3(0.03) * texColor.rgb; // Base ambient
        litColor = ambient + Lo;
        
        // Simple HDR tonemapping
        litColor = litColor / (litColor + float3(1.0));
        
        finalColor = litColor;
    }
    
    output.Color = float4(finalColor, texColor.a);
    output.EntityID = pc.entityID;
    
    return output;
}
