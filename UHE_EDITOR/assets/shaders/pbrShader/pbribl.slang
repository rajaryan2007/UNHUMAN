struct VSInput 
{
    [[vk::location(0)]] float3 Pos : POSITION;
    [[vk::location(1)]] float3 Normal : NORMAL;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float4 Tangent : TANGENT;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 WorldPos;
    float3 Normal;
    float2 UV;
};

struct UBO
{
    float4x4 projection;
    float4x4 model;
    float4x4 view;
    float3 camPos;
};
ConstantBuffer<UBO> ubo;

struct PushConstants {
    int enableLighting;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

struct UBOParams {
    float4 lights[4];
    float exposure;
    float gamma;
};
ConstantBuffer<UBOParams> uboParams;

SamplerCube samplerIrradiance;
Sampler2D samplerBRDFLUT;
SamplerCube prefilterMapSampler;

struct Material {
    [[vk::offset(12)]] float roughness;
    [[vk::offset(16)]] float metallic;
    [[vk::offset(20)]] float specular;
    [[vk::offset(24)]] float r;
    [[vk::offset(28)]] float g;
    [[vk::offset(32)]] float b;
};

#define PI 3.1415926535897932384626433832795

float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * denom * denom);
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

float3 F_Schlick(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 prefilteredReflections(float3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 9.0;
    float lod = roughness * MAX_REFLECTION_LOD;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    float3 a = prefilterMapSampler.SampleLevel(R, lodf).rgb;
    float3 b = prefilterMapSampler.SampleLevel(R, lodc).rgb;
    return lerp(a, b, lod - lodf);
}

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    float4 locPos = mul(ubo.model, float4(input.Pos, 1.0));
    output.WorldPos = locPos.xyz;
    output.Normal = mul((float3x3)ubo.model, input.Normal);
    output.UV = input.UV;
    output.Pos = mul(ubo.projection, mul(ubo.view, locPos));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
    float3 N = normalize(input.Normal);
    float3 V = normalize(ubo.camPos - input.WorldPos);
    float3 R = reflect(-V, N);

    // Using placeholder material properties since Material buffer isn't bound in this stub
    float roughness = 0.5;
    float metallic = 0.0;
    float3 albedo = float3(1.0, 1.0, 1.0);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    float2 brdf = samplerBRDFLUT.Sample(float2(max(dot(N, V), 0.0), roughness)).rg;
    float3 reflection = prefilteredReflections(R, roughness);
    float3 irradiance = samplerIrradiance.Sample(N).rgb;

    float3 diffuse = irradiance * albedo;
    float F = F_Schlick(max(dot(N, V), 0.0), F0, roughness).r;
    float3 specular = reflection * (F * brdf.x + brdf.y);

    float3 kD = 1.0 - F;
    kD *= 1.0 - metallic;

    float3 ambient = (kD * diffuse + specular);
    
    if (pc.enableLighting == 0) {
        return float4(albedo, 1.0);
    }

    return float4(ambient, 1.0);
}
