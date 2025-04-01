#include "DefaultMaterial.hlsli"
#include "LightSampling.hlsli"


struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 tex_coord : TEXCOORD;
};

struct PixelShaderOutput
{
    float4 color : SV_Target0; // w is emissiveness
    float depth : SV_Target1; // in world units
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
	
    output.depth = length(input.world_position.xyz - cam_position.xyz);
    
    const float3 pos = input.world_position.xyz;

    const float4 diffuseColW = Texture.Sample(Sampler, input.tex_coord);
    const float3 diffuseCol = diffuseColW.xyz;
	
	// TODO: Implement alpha cutout for real
    clip(diffuseColW.w - alphaCutoff);
	
    const float3 bitangent = normalize(cross(input.normal, input.tangent));
    const float3 norm = (sampleNormal > 0)
		? normalize(mul(NormalMap.Sample(Sampler, input.tex_coord).xyz * 2.0 - float3(1.0, 1.0, 1.0), float3x3(input.tangent, bitangent, input.normal)))
        : normalize(input.normal);
    const float3 viewDir = normalize(cam_position.xyz - pos);

    const float3 specularCol = (sampleSpecular > 0)
		? SpecularMap.Sample(Sampler, input.tex_coord).xyz
		: float3(0.0, 0.0, 0.0);
	
    const float glossiness = (sampleGlossiness > 0)
		? GlossinessMap.Sample(Sampler, input.tex_coord).x
		: 1.0 - (1.0 / pow(specularExponent, 1.75));

    const float3 ambientCol = (sampleAmbient > 0)
		? AmbientMap.Sample(Sampler, input.tex_coord).xyz
        : float3(0.0, 0.0, 0.0);

	const float3 light = (sampleLight > 0)
		? LightMap.Sample(Sampler, input.tex_coord).xyz
		: float3(0.0, 0.0, 0.0);

	const float occlusion = (sampleOcclusion > 0)
		? 0.15 + (OcclusionMap.Sample(Sampler, input.tex_coord).x * 0.85)
		: 1.0;
        
	float3 totalDiffuseLight, totalSpecularLight;
	
	CalculateLighting(
		pos, viewDir,								// View
		norm, specularCol, glossiness,				// Surface
		totalDiffuseLight, totalSpecularLight		// Output
	);
	
    float3 totalLight = diffuseCol * (light + occlusion * (ambientCol + totalDiffuseLight)) + occlusion * totalSpecularLight;
	float3 acesLight = ACESFilm(totalLight);
	
	//float emissiveness = max(acesLight.x, max(acesLight.y, acesLight.z));
    float emissiveness = (totalSpecularLight.x + totalSpecularLight.y + totalSpecularLight.z) / 3.0;
	//emissiveness = max(0.0, (1.0 - (1.0 / (emissiveness - 1.2))));
	emissiveness = 1.0 - pow(emissiveness * 0.1 - 0.99, 2.0);
	
    output.color = float4(totalLight, emissiveness);
    return output;
}