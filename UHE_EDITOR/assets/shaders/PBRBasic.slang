struct UBOParams {
    float4 lights[4];
};

struct Material {
    [[vk::offset(12)]] float roughness;
    [[vk::offset(16)]] float metallic;
    [[vk::offset(20)]] float r;
    [[vk::offset(24)]] float g;
    [[vk::offset(28)]] float b;
};

[[vk::push_constant]] Material material;

static const float PI = 3.14159265359;

// Normal Distribution function 
float DGGX(float dotNH,float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2)/(PI * denom * denom);
}

//Geometric Shadowing function 
float G_SchlicksmithGGX(float donNL,float dotNV,float roughness)
{
    float r = (roughness + 1.0);
    float k = (r* r)/ 8.0;
    float GL = dotNL / (dotNL * (1.0 -k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

float3 F_Schlick(float cosTheta,Material material)
{
    float3 F0 = lerp(float3(0.04,0.04,0.04),float3(material.r,material.g,material.b)material.metallic);
    float3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta,5.0);
    return F;
}

float3 BRDF(float3 L,float3 V,float3 N,Material material)
{
  float3 H = normalize (V + L);
  float dotNV = clamp(dot(N,V),0.0,1.0);
  float dotNL = clamp(dot(N,L),0.0,1.0);
  float dotLH = clamp(dot(L,H),0.0,1.0);
  float dotNH = clamp(dot(N,H),0.0,1.0);

  float3 lightColor = float3(1.0,1.0,1.0);

  float3 color = float3(0.0,0.0,0.0);
  
  if(dotNL > 0.0)
  {
    float rroughness = max(0.05,material.roughness);
    // D = Nommal distritution (distritution of the mircofacets)
    float D = DGGX(dotNH,material.roughness);
    // G = Geometric shadwoing term (mircofacets shadwoing)
    float G = G_SchlicksmithGGX(dotNL,dotNV,rroughness);
    //F = fresnel factor (feflectancec depending on angle of incidence )
    float3 F = F_Schlick(dotNV,material);
    
    float3 spec = D * F * G / (4.0 * dotNL * dotNV);
    
    color += spec * dotNL * lightColor;

  }
  return color;

    
}
