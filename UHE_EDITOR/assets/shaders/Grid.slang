struct VSInput
{
    uint vertexID : SV_VertexID;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 nearPoint;
    float3 farPoint;
};

struct PushConstants
{
    float4x4 viewProj;
    float4x4 inverseViewProj;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PushConstants;

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;
    
    float2 positions[6] = {
        float2(-1.0, -1.0),
        float2( 1.0, -1.0),
        float2(-1.0,  1.0),
        float2( 1.0, -1.0),
        float2( 1.0,  1.0),
        float2(-1.0,  1.0)
    };
    
    float2 pos = positions[input.vertexID];
    
    // Near and far points in clip space
    float4 nearClip = float4(pos.x, pos.y, 0.0, 1.0); 
    float4 farClip = float4(pos.x, pos.y, 1.0, 1.0);
    
    float4 unprojectedNear = mul(u_PushConstants.inverseViewProj, nearClip);
    float4 unprojectedFar = mul(u_PushConstants.inverseViewProj, farClip);
    
    output.nearPoint = unprojectedNear.xyz / unprojectedNear.w;
    output.farPoint = unprojectedFar.xyz / unprojectedFar.w;
    output.position = float4(pos, 0.9999, 1.0);
    
    return output;
}

struct FSOutput
{
    float4 color : SV_Target0;
    int entityID : SV_Target1;
    float depth : SV_Depth;
};

[shader("fragment")]
FSOutput fragmentMain(VSOutput input)
{
    FSOutput output;
    
    float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
    
    if (t < 0.0)
    {
        discard;
    }
    
    float3 fragPos3D = input.nearPoint + t * (input.farPoint - input.nearPoint);
    
    float scale = 1.0;
    float2 coord = fragPos3D.xz * scale;
    float2 derivative = fwidth(coord);
    float2 grid = abs(frac(coord - 0.5) - 0.5) / derivative;
    float lineVal = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1.0);
    float minimumx = min(derivative.x, 1.0);
    
    float4 color = float4(0.4, 0.4, 0.4, 1.0 - min(lineVal, 1.0));
    
    // Primary axes
    if (fragPos3D.x > -0.5 * minimumx && fragPos3D.x < 0.5 * minimumx)
        color.xyz = float3(0.1, 0.1, 0.8); // Z axis (Blue)
    if (fragPos3D.z > -0.5 * minimumz && fragPos3D.z < 0.5 * minimumz)
        color.xyz = float3(0.8, 0.1, 0.1); // X axis (Red)
        
    // Fading
    float linearDepth = length(fragPos3D - input.nearPoint);
    float maxDist = 50.0; // Grid view distance
    float fade = max(0.0, 1.0 - (linearDepth / maxDist));
    fade = fade * fade * fade; // Non-linear fade for better aesthetics
    
    color.a *= fade;
    color.a *= 0.5; // Overall transparency
    
    if (color.a < 0.01) discard;
    
    output.color = color;
    output.entityID = -1;
    
    
    float4 clipPos = mul(u_PushConstants.viewProj, float4(fragPos3D, 1.0));
    output.depth = clipPos.z / clipPos.w;
    
    return output;
}
