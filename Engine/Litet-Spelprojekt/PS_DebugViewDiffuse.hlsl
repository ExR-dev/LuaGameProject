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
    const float3 diffuseCol = Texture.Sample(Sampler, input.tex_coord).xyz;
    return float4(diffuseCol, 1.0f);
}