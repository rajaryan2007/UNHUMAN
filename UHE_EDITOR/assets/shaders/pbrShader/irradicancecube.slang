SamplerCube samplerEnv;

struct PushConstants {
    [[vk::offset(64)]] float deltaPhi;
    [[vk::offset(68)]] float deltaTheta;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants;

#define PI 3.1415926535897932384626433832795

[shader("fragment")]
float4 fragmentMain(float3 inPos)
{
    float3 N = normalize(inPos);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    const float TWO_PI = 2.0 * PI;
    const float HALF_PI = PI / 2.0;

    float3 color = float3(0.0, 0.0, 0.0);
    uint sampleCount = 0u;

    for (float phi = 0.0; phi < TWO_PI; phi += pushConstants.deltaPhi) {
        for (float theta = 0.0; theta < HALF_PI; theta += pushConstants.deltaTheta) {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            color += samplerEnv.Sample(sampleVec).rgb * cos(theta) * sin(theta);
            sampleCount++;
        }
    }

    return float4(PI * color / float(sampleCount), 1.0);
}