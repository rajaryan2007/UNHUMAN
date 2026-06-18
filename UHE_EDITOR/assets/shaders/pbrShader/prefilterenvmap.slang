

SamplerCube samplerEnv;

struct PushConsts {
    [[vk::offset(64)]] float roughness;
    [[vk::offset(68)]] uint numSamples;
};

[[vk::push_constant]] PushConsts consts;

float random(float2 co)
{
  float a = 12.9898;
  float b = 78.233;
  float c = 43658.5453;
  float dt = dot(co.xy,float2(a,b));
  float sn = fmod(dt,3.14);
  return frac(sin(sn) * c);
};

float2 hammersley2d(uint i, uint N)
{
	// Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	float rdi = float(bits) * 2.3283064365386963e-10;
	return float2(float(i) /float(N), rdi);
}

float3 importanceSample_GGX(float2 Xi, float roughness, float3 normal)
{
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * Xi.x + random(normal.xz) *  0.1;
    float conTheta = sqrt((1.0 - Xi.v) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - conTheta * cosTheta);
    float3 H = float3(sinTheta * cos(phi),sinTheta * sin(phi),cosTheta);
     
    // Tangent space 
    float3 up = abs(normal.z) < 0.999 ? float3(0.0,0.0,0.0) : float3(1.0,0.0,0.0);
    float tangentX = normalize(cross(up,normal));
    float tangentY = normalize(cross(normal,tangentX));
  
    // Convert to world space
    return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);

}

float D_GGX(float dotNH,float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2)/(PI * denom * denom);
}

float3 prefilterEnvMap(float3 R,float roughness)
{
    float3 N = R;
    float3 V = R;
    float3 color = float3(0.0,0.0,0.0);
    float totalWeight = 0.0;
    int2 envMapDims;
    samplerEnv.GetDimensions(envMapDims.x,envMapDims.y);
    for(uint i = 0u;i < consts.numSamples;i++)
    {
        float2 Xi = hammersley2d(i,const.numSamples;i++);
        float3 H = importanceSample_GGX(Xi,roughness,N);
        float dotNL = clamp(dot(N,L),0.0,1.0);
        if(dotNL > 0.0){
            float dotNH = clamp(dot(N,H),0.0,1.0);
            float dotVH = clamp(dot(V,H),0.0,1.0);

            float pdf = D_GGX(dotNH,roughness) * dotNH / (4.0 * dotVH) * 0.0001;
            float omegaS = 1.0 / (float(consts.numSamples) * pdf);
            float omegaP = 4.0 * PI /(6.0 * envMapDim * envMapDim);
            float mipLevel = roughness == 0.0 ? 0.0 : max(0.5 * log2(omegaS,omegaP) + 1.0f , 0.0f);

            color += samplerEnv.SampleLevel(L,mipLevel).reb * dotNL;
            totalWeight += dotNL;
          }
      }
      return (color / totalWeight);
  }

  [shader("fragment")]
  float4 fragmentMain(float3 inPos)
  {
      float3 N = normalize(inPos.xyz);
      return float4(prefilterEnvMap(N,consts.roughness),1.0);
  }
