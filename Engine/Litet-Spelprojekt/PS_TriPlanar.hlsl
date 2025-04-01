#include "DefaultMaterial.hlsli"
#include "LightSampling.hlsli"

static float SHARPNESS = 1.0;

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
	
	
	// Calculate Tri-Planar UVs
	float2 uvx = pos.yz;
	float2 uvy = pos.xz;
	float2 uvz = pos.xy;
	
	// Flip UVs to correct for normal direction
	uvx.x = input.normal.x > 0 ? 1.0 - uvx.x : uvx.x;
	uvx.y = 1.0 - uvx.y;
	
	uvy = float2(1.0, 1.0) - uvy;
	uvy.x = input.normal.y > 0 ? 1.0 - uvy.x : uvy.x;
	
	uvz.y = input.normal.z < 0 ? 1.0 - uvz.y : uvz.y;
	
	float3 weight = pow(abs(input.normal), SHARPNESS);
	weight /= weight.x + weight.y + weight.z;
	
	float2 uv = (uvx * weight.x) + (uvy * weight.y) + (uvz * weight.z);
	

	const float3 diffuseCol = Texture.Sample(Sampler, uv).xyz;
	
	const float3 bitangent = normalize(cross(input.normal, input.tangent));
	const float3 norm = (sampleNormal > 0)
		? normalize(mul(NormalMap.Sample(Sampler, uv).xyz * 2.0f - float3(1.0f, 1.0f, 1.0f), float3x3(input.tangent, bitangent, input.normal)))
        : normalize(input.normal);
	const float3 viewDir = normalize(cam_position.xyz - pos);

	const float3 specularCol = (sampleSpecular > 0)
		? SpecularMap.Sample(Sampler, uv).xyz
		: float3(0.0f, 0.0f, 0.0f);
	
    const float glossiness = (sampleGlossiness > 0)
		? GlossinessMap.Sample(Sampler, input.tex_coord).x
		: 1.0 - (1.0 / pow(specularExponent, 1.75));

	const float3 ambientCol = (sampleAmbient > 0)
		? AmbientMap.Sample(Sampler, uv).xyz
        : float3(0.0f, 0.0f, 0.0f);

	const float3 light = (sampleLight > 0)
		? LightMap.Sample(Sampler, uv).xyz
		: float3(0.0f, 0.0f, 0.0f);

    const float occlusion = (sampleOcclusion > 0)
		? OcclusionMap.Sample(Sampler, input.tex_coord).x
		: 1.0;
        
	
	float3 totalDiffuseLight, totalSpecularLight;
	
	CalculateLighting(
		pos, viewDir,							// View
		norm, specularCol, glossiness,			// Surface
		totalDiffuseLight, totalSpecularLight	// Output
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