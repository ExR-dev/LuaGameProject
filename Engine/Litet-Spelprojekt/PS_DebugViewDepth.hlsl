#include "Common.hlsli"
#include "DefaultMaterial.hlsli"
#include "LightData.hlsli"


struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 tex_coord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    // Get the depth value in NDC space
	float depthNDC = input.position.z / input.position.w;

    // Linearize the depth
	float depth = LinearizeDepth(depthNDC, cam_planes.x, cam_planes.y);
    
    return float4(depth, depth, depth, 1.0f);
}