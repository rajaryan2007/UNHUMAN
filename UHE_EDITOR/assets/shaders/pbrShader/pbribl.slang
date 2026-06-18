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
    float4x4 camPos;
};

struct UBOPramas {
    float4 lights[4];
    float exposure;
    float gamma;
}

ConstantBuffer<UBOParams> uboParams;

SamplerCube samplerIrradiance;
Sampler2D saplerBRDFFLUT;
SamplerCube PrefiltredMapSampler;

struct Material {
    [[vk::offset(12)]] float roughness;
    [[vk::offset(16)]] float metallic;
    [[vk::offset(20)]] float specular;
    [[vk::offset(24)]] float r;
    [[vk::offset(28)]] float g;
    [[vk::offset(32)]] float b;
}

float D_GGX(float dotNH,float roughness)
{
    float aplha = roughness * roughness;
    float alpha2 = aplha * aplha;
    float denom =  dotNH * dotNH * (alpha -1.0) + 1.0;
    return (aplha2)/(PI * denom * denom);

  }

float G_SchicksmithGGX(float dotNL,float dotNV,float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) /8.0;
    float GL = dotNL /(dotNL * (1.0 - k) + k);
    float GV = dotNV /(dotNV * (1.0 - k) + k);
    return GL * GV;
}


