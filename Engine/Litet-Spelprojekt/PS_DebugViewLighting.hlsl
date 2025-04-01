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
    const float3 pos = input.world_position.xyz;

    const float3 diffuseCol = Texture.Sample(Sampler, input.tex_coord).xyz;
	
    const float3 bitangent = normalize(cross(input.normal, input.tangent));
    const float3 norm = (sampleNormal > 0)
		? normalize(mul(NormalMap.Sample(Sampler, input.tex_coord).xyz * 2.0f - float3(1.0f, 1.0f, 1.0f), float3x3(input.tangent, bitangent, input.normal)))
        : normalize(input.normal);
    const float3 viewDir = normalize(cam_position.xyz - pos);

    const float3 specularCol = (sampleSpecular > 0)
		? SpecularMap.Sample(Sampler, input.tex_coord).xyz
		: float3(0.0f, 0.0f, 0.0f);

    const float3 ambientCol = (sampleAmbient > 0)
		? AmbientMap.Sample(Sampler, input.tex_coord).xyz
        : float3(0.0f, 0.0f, 0.0f);

	const float3 light = (sampleLight > 0)
		? LightMap.Sample(Sampler, input.tex_coord).xyz
		: float3(0.0f, 0.0f, 0.0f);
        
    float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
    float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);
    uint spotlightCount, spotWidth, spotHeight, _u;
    SpotLights.GetDimensions(spotlightCount, _u);
    SpotShadowMaps.GetDimensions(0, spotWidth, spotHeight, _u, _u);

    const float
		spotDX = 1.0f / (float) spotWidth,
		spotDY = 1.0f / (float) spotHeight;

	// Per-spotlight calculations
    for (uint spotlight_i = 0; spotlight_i < spotlightCount; spotlight_i++)
    {
		// Prerequisite variables
        const SpotLight light = SpotLights[spotlight_i];

        const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
        const float
			projectedDist = dot(light.direction, -toLight),
			inverseLightDistSqr = light.orthographic > 0 ?
			(projectedDist > 0.0f ? 1.0f : 0.0f) * (1.0f / (1.0f + pow(projectedDist, 2))) :
			1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2)));

        const float offsetAngle = saturate(1.0f - (
			light.orthographic > 0 ?
			length(cross(light.direction, toLight)) :
			acos(dot(-toLightDir, light.direction))
			) / (light.angle * 0.5f));


        float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, norm, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
        specularLightCol *= specularCol;


		// Calculate shadow projection
        const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
        const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
        const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

        const float3
			spotUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, spotlight_i),
			spotUV01 = spotUV00 + float3(0.0f, spotDY, 0.0f),
			spotUV10 = spotUV00 + float3(spotDX, 0.0f, 0.0f),
			spotUV11 = spotUV00 + float3(spotDX, spotDY, 0.0f);

        const float
			spotDepth00 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV00, 0).x,
			spotDepth01 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV01, 0).x,
			spotDepth10 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV10, 0).x,
			spotDepth11 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV11, 0).x;

        const float
			spotResult00 = spotDepth00 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult01 = spotDepth01 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult10 = spotDepth10 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult11 = spotDepth11 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;

        const float2
			texelPos = spotUV00.xy * (float) spotWidth,
			fracTex = frac(texelPos);
		
        const float shadow = isInsideFrustum * saturate(
			offsetAngle * lerp(
				lerp(spotResult00, spotResult10, fracTex.x),
				lerp(spotResult01, spotResult11, fracTex.x),
				fracTex.y)
		);
        //const float shadow = isInsideFrustum * saturate(offsetAngle * spotResult00);


		// Apply lighting
        totalDiffuseLight += diffuseLightCol * shadow * inverseLightDistSqr;
        totalSpecularLight += specularLightCol * shadow * inverseLightDistSqr;
    }
	
	
	uint simpleSpotlightCount;
	SimpleSpotLights.GetDimensions(simpleSpotlightCount, _u);

	// Per-simple spotlight calculations
	for (uint simpleSpotlight_i = 0; simpleSpotlight_i < simpleSpotlightCount; simpleSpotlight_i++)
	{
		// Prerequisite variables
		const SimpleSpotLight light = SimpleSpotLights[simpleSpotlight_i];

		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
		const float
			projectedDist = dot(light.direction, -toLight),
			inverseLightDistSqr = light.orthographic > 0 ?
			(projectedDist > 0.0f ? 1.0f : 0.0f) * (1.0f / (1.0f + pow(projectedDist, 2))) :
			1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2)));

		const float offsetAngle = saturate(1.0f - (
			light.orthographic > 0 ?
			length(cross(light.direction, toLight)) :
			acos(dot(-toLightDir, light.direction))
			) / (light.angle * 0.5f));


		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, norm, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
		specularLightCol *= specularCol;
		
		// Apply lighting
		totalDiffuseLight += diffuseLightCol * saturate(offsetAngle) * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * saturate(offsetAngle) * inverseLightDistSqr;
	}
	
	
    uint pointlightCount, pointWidth, pointHeight;
    PointLights.GetDimensions(pointlightCount, _u);
    PointShadowMaps.GetDimensions(0, pointWidth, pointHeight, _u, _u);

    const float
		pointDX = 1.0f / (float) pointWidth,
		pointDY = 1.0f / (float) pointHeight;

	// Per-pointlight calculations
    for (uint pointlight_i = 0; pointlight_i < pointlightCount; pointlight_i++)
    {
		// Prerequisite variables
        const PointLight light = PointLights[pointlight_i];

        const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
        const float inverseLightDistSqr = 1.0f / (1.0f + (
			pow(toLight.x * light.falloff, 2) +
			pow(toLight.y * light.falloff, 2) +
			pow(toLight.z * light.falloff, 2)
		));
		

        float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, norm, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
        specularLightCol *= specularCol;


		// Calculate shadow projection
        const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
        const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
        const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

        const float3
			pointUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, pointlight_i),
			pointUV01 = pointUV00 + float3(0.0f, pointDY, 0.0f),
			pointUV10 = pointUV00 + float3(pointDX, 0.0f, 0.0f),
			pointUV11 = pointUV00 + float3(pointDX, pointDY, 0.0f);

        const float
			pointDepth00 = PointShadowMaps.SampleLevel(ShadowSampler, pointUV00, 0).x,
			pointDepth01 = PointShadowMaps.SampleLevel(ShadowSampler, pointUV01, 0).x,
			pointDepth10 = PointShadowMaps.SampleLevel(ShadowSampler, pointUV10, 0).x,
			pointDepth11 = PointShadowMaps.SampleLevel(ShadowSampler, pointUV11, 0).x;

        const float
			pointResult00 = pointDepth00 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult01 = pointDepth01 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult10 = pointDepth10 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult11 = pointDepth11 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;

        const float2
			texelPos = pointUV00.xy * (float) pointWidth,
			fracTex = frac(texelPos);
		
        const float shadow = isInsideFrustum * saturate(lerp(
			lerp(pointResult00, pointResult10, fracTex.x),
			lerp(pointResult01, pointResult11, fracTex.x),
			fracTex.y)
		);
        //const float shadow = isInsideFrustum * saturate(pointResult00);


		// Apply lighting
        totalDiffuseLight += diffuseLightCol * shadow * inverseLightDistSqr;
        totalSpecularLight += specularLightCol * shadow * inverseLightDistSqr;
    }
	
	
	uint simplePointlightCount;
	SimplePointLights.GetDimensions(simplePointlightCount, _u);

	// Per-simple pointlight calculations
	for (uint simplePointlight_i = 0; simplePointlight_i < simplePointlightCount; simplePointlight_i++)
	{
		// Prerequisite variables
		const SimplePointLight light = SimplePointLights[simplePointlight_i];

		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
		const float inverseLightDistSqr = 1.0f / (1.0f + (
			pow(toLight.x * light.falloff, 2) +
			pow(toLight.y * light.falloff, 2) +
			pow(toLight.z * light.falloff, 2)
		));
		
		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, norm, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
		specularLightCol *= specularCol;
		
		// Apply lighting
		totalDiffuseLight += diffuseLightCol * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * inverseLightDistSqr;
	}
	
	
    const float3 result = ACESFilm(ambientCol + light + totalDiffuseLight + totalSpecularLight);
	return float4(result, 1.0f);
}