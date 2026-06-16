SamplerCube samplerEnv;

sturct PushConstants {
    [[vk::offset(64)]] float deltaPhi;
    [[vk::offset(68)]] float deltaTheta;
};

[[vk::push_constant]] PushConstants pushConstants;

#define PI 3.1415926535897932384626433832795

[shader("fragment")]
float4 fragmentMain(float3 inPos)
{
  float3 N = normalize(inPos.xyz);
  float3 up = float3(0.0, 1.0, 0.0);
  float3 right = normalize(cross(up, N));
  up = normalize(cross(N, right));

  const float TWO_PI  = 2.0 * PI;
   float phi = atan2(N.z, N.x) + pushConstants.deltaPhi;
   float theta = acos(N.y) + pushConstants.deltaTheta;

   float3 color = float3(0.0, 0.0, 0.0);
   uint sampleCount = 0u;
   for(float phi = 0.0; phi < TWO_PI;)

}