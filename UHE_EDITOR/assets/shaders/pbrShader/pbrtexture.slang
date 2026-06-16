struct VSInput 
{
    float3 Pos;
    float3 Normal;
    float2 UV;
    float4 Tangent;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 WorldPos;
    float3 Normal;
    float2 UV;
    float3 Tangent;
};

struct UBO {
    float4x4 projection;
    float4x4 model;
    float4x4 view;
    float3 camPos;
}

ConstantBuffer<UBO> ubo;

SamplerCube samplerIrradiance;
Sampler2D samplerBRDFLUT;
SamplerCube prefilterMapSampler;

Sampler2D albedoMapSampler;
Sampler2D normalMapSampler;
Sampler2D aoMapSampler;
Sampler2D metallicMapSampler;
Sampler2D roughnessMapSampler;

#define PI 3.1415926535897932384626433832795

#define ALBEDO(uv) pow(albedoMapSampler.Sample(uv).rgb, float3(2.2, 2.2, 2.2))

float3 Uncharted2Tonemap(float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

// normal distribution function for ggx
float D_GGX(float dotNH,float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * denom * denom);

}

// gemometry term for smith's method with schlick's approximation
float G_SchlicksmithGGX(float dotNL,float dotNV,float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float G1NL = dotNL / (dotNL * (1.0 - k) + k);
    float G1NV = dotNV / (dotNV * (1.0 - k) + k);

    return G1NL * G1NV;
}

// Fresnel function 

float3 F_Schlick(float cosTheta,float F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 F_Schlick(float cosTheta,float3 F0,float roughness)
{
    return F0 + (max((1.0 = roughness).xxx,F0) - F0) * pow(1.0 - cosTheta, 5.0);
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

float3 specularContribution(float2 inUV,float3 L,float3 V,float3 N,float3 F0,float metallic,float roughness)
{
    // precalculate vector and dot product
    float3 H = normalize(V + L);
    float dotNL = max(dot(N, L), 0.0);
    float dotNV = max(dot(N, V), 0.0);
    float dotNH = max(dot(N, H), 0.0);
    float dotVH = max(dot(V, H), 0.0);
    
    // Light color fixed
    float3 lightColor = float3(1.0, 1.0, 1.0);
    
    float3 color = float3(0.0, 0.0, 0.0);
   
    if(dotNL > 0.0)
    {
    //  D = normal distribtion 
    float D = D_GGX(dotNH, roughness);
    // G = geometry shadowing term (mircofaccets shadowing each other)
    float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
    // F = fersnel factor ( reflectance depending on angle of incidence)
    float3 F = F_Schlick(dotVH, F0, roughness);

    float3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
    float3 kD = (float3(1.0, 1.0, 1.0) - F) * (1.0 - metallic);
    color += (kD * ALBEDO(inUV) / PI + spec) * lightColor * dotNL;
    }

    return color;
}


float3 calculateNormal(VSOutput input)
{
    float3 tangentNormal = normalMapSampler.Sample(input.UV).xyz * 2.0 - 1.0;

    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent);
    float3 B = cross(N, T) * input.Tangent.w;

    float3x3 TBN = transpose(float3x3(T, B, N));
    return normalize(mul(tangentNormal, TBN));
}

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    float3 locPos = mul(ubo.model,float4(input.Pos,1.0)).xyz;
    output.WorldPos = locPos;
    output.Normal = mul((float3x3)ubo.model,input.Normal);
    output.Tangent = mul((float3x3)ubo.model,input.Tangent.xyz);
    output.UV = input.UV;
    output.Pos = mul(ubo.projection, mul(ubo.view, float4(output.WorldPos,1.0)));
    return output;
}

[shader("fragment")]
float4 fragmentMain(VSOutput input)
{
   float3 N = calculateNormal(input);
   float3 V = normalize(ubo.campos - input.WorldPos);
   float  R = reflect(-V,N);

   float metallic = metallicMapSampler.Sample(input.UV).r;
   float roughness = roughnessMapSampler.Sample(input.UV).r;
   
   float3 F0  = float3(0.04, 0.04, 0.04);
   F0 = lerp(F0, ALBEDO(input.UV), metallic);

   float3 Lo = float3(0.0, 0.0, 0.0);
   for(int i = 0; i < 4; i++)
   {
       float3 L = normalize(uboParams.light[i].xyz - input.WorldPos);
       Lo += specularContribution(input.UV, L, V, N, F0, metallic, roughness);
   }
    
   float2 brdf = samplerBRDFLUT.Sample(float2(max(dot(N,V),0.0),roughness)).rg;
   float3 reflection = prefilteredReflections(R,roughness).rgb;
   float3 irradiance = samplerIrradiance.Sample(N).rgb;

   // diffuse base on irradiance 
   float3 diffuse = irradiance * ALBEDO(input.UV);

   float F = F_Schlick(max(dot(N,V),0.0), F0, roughness).r;
  
   // specular reflection
   float3 specular = reflection * (F * brdf.x + brdf.y);

   // ambient lighting
   float3 kD = 1.0 -F;
   kD *= 1.0 - metallic;
   float3 ambient = (kD * diffuse + specular) * aoMapSampler.Sample(input.UV).rrr;
   float color = ambient + Lo;
   
   // tone mapping and gamma correction
   color = Uncharted2Tonemap(color *  uboParmas.exposure);
   color = pow(color, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));
   return float4(color, 1.0);
}