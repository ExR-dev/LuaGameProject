#include "Common.hlsli"
#include "LightData.hlsli"


void CalculateLighting(
	float3 pos, float3 viewDir,										// View
	float3 normal, float3 specularColor, float specularExponent,	// Surface
	out float3 totalDiffuseLight, out float3 totalSpecularLight)	// Output
{
    totalDiffuseLight = ambient_light.xyz;
	totalSpecularLight = float3(0.0, 0.0, 0.0);
	
	// Calculate light tile position
	const float4 screenPosClip = mul(float4(pos, 1.0f), view_proj_matrix);
	const float3 screenPosNDC = screenPosClip.xyz / screenPosClip.w;
	const float2 screenUV = float2((screenPosNDC.x * 0.5f) + 0.5f, (screenPosNDC.y * 0.5f) + 0.5f);
	
    uint lightTiles, lightTileDim, _;
    LightTileBuffer.GetDimensions(lightTiles, _);
    lightTileDim = sqrt(lightTiles);
	
    uint2 lightTileCoord = uint2(screenUV * float(lightTileDim));
    int lightTileIndex = lightTileCoord.x + (lightTileCoord.y * lightTileDim);
	
	LightTile lightTile = LightTileBuffer[lightTileIndex];
	
	
	// Get spotlight data
	uint spotlightCount = lightTile.spotlightCount;
	uint spotWidth, spotHeight;
	SpotShadowMaps.GetDimensions(0, spotWidth, spotHeight, _, _);
	
	const float
		spotDX = 1.0 / float(spotWidth),
		spotDY = 1.0 / float(spotHeight);
	
	// Per-spotlight calculations
	for (uint spotlight_i = 0; spotlight_i < spotlightCount; spotlight_i++)
	{
		// Prerequisite variables
		uint currentSpotlight = lightTile.spotlights[spotlight_i];
		const SpotLight light = SpotLights[currentSpotlight];
		
		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
		const float projectedDist = dot(light.direction, -toLight);
		float inverseLightDistSqr = light.orthographic > 0 ?
			(projectedDist > 0.0 ? 1.0 : 0.0) * (1.0 / (1.0 + pow(projectedDist, 2))) : // If orthographic
			1.0 / (1.0 + (pow(toLight.x * light.falloff, 2.0) + pow(toLight.y * light.falloff, 2.0) + pow(toLight.z * light.falloff, 2.0))); // If perspective
		
		CutoffLight(light.color.xyz, inverseLightDistSqr);
		
		const float offsetAngle = saturate(1.0 - (
			light.orthographic > 0 ?
			length(cross(light.direction, toLight)) :
			acos(dot(-toLightDir, light.direction))
			) / (light.angle * 0.5));


		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, normal, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
		specularLightCol *= specularColor;


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + normal * NORMAL_OFFSET, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3
			spotUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, currentSpotlight),
			spotUV01 = spotUV00 + float3(0.0f, spotDY, 0.0f),
			spotUV10 = spotUV00 + float3(spotDX, 0.0f, 0.0f),
			spotUV11 = spotUV00 + float3(spotDX, spotDY, 0.0f);

		float
			spotDepth00 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV00, 0).x,
			spotDepth01 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV01, 0).x,
			spotDepth10 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV10, 0).x,
			spotDepth11 = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV11, 0).x;
		
		spotDepth00 = spotDepth00 > 0.0 ? spotDepth00 : -100.0;
		spotDepth01 = spotDepth01 > 0.0 ? spotDepth01 : -100.0;
		spotDepth10 = spotDepth10 > 0.0 ? spotDepth10 : -100.0;
		spotDepth11 = spotDepth11 > 0.0 ? spotDepth11 : -100.0;

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
	
	
	// Get simple spotlight data
	uint simpleSpotlightCount = lightTile.simpleSpotlightCount;
	
	// Per-simple spotlight calculations
	for (uint simpleSpotlight_i = 0; simpleSpotlight_i < simpleSpotlightCount; simpleSpotlight_i++)
	{
		// Prerequisite variables
		uint currentSimpleSpotlight = lightTile.simpleSpotlights[simpleSpotlight_i];
		const SimpleSpotLight light = SimpleSpotLights[currentSimpleSpotlight];

		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
		const float projectedDist = dot(light.direction, -toLight);
		float inverseLightDistSqr = light.orthographic > 0 ?
			(projectedDist > 0.0f ? 1.0f : 0.0f) * (1.0f / (1.0f + pow(projectedDist, 2))) : // If orthographic
			1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2))); // If perspective
		
		CutoffLight(light.color.xyz, inverseLightDistSqr);
		
		const float offsetAngle = saturate(1.0f - (
			light.orthographic > 0 ?
			length(cross(light.direction, toLight)) :
			acos(dot(-toLightDir, light.direction))
			) / (light.angle * 0.5f));


		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, normal, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
		specularLightCol *= specularColor;
		
		// Apply lighting
		totalDiffuseLight += diffuseLightCol * saturate(offsetAngle) * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * saturate(offsetAngle) * inverseLightDistSqr;
	}
	
	
	// Get pointlight data
    uint pointlightCount = lightTile.pointlightCount;
    uint pointWidth, pointHeight;
    PointShadowMaps.GetDimensions(0, pointWidth, pointHeight, _, _);

    const float
		pointDX = 1.0f / (float)pointWidth,
		pointDY = 1.0f / (float)pointHeight;
	
	// Per-pointlight calculations
    for (uint pointlight_i = 0; pointlight_i < pointlightCount; pointlight_i++)
    {
		// Prerequisite variables
        uint currentPointlight = lightTile.pointlights[pointlight_i];
        const PointLight light = PointLights[currentPointlight];
		
        const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
        const float inverseLightDistSqr = 1.0f / (1.0f + (
			pow(toLight.x * light.falloff, 2) +
			pow(toLight.y * light.falloff, 2) +
			pow(toLight.z * light.falloff, 2)
		));
		
        float3 diffuseLightCol, specularLightCol;
        BlinnPhong(toLightDir, viewDir, normal, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
        specularLightCol *= specularColor;
		
		
		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + normal * NORMAL_OFFSET, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const bool isInsideFrustum = (
				fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
				fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f &&
				fragPosLightNDC.z > 0.0f
			);

		const float3
			pointUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, currentPointlight),
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


		// Apply lighting
		totalDiffuseLight += diffuseLightCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * shadow * inverseLightDistSqr;
	}
	
	
	// Get simple pointlight data
	uint simplePointlightCount = lightTile.simplePointlightCount;
	
	// Per-simple pointlight calculations
	for (uint simplePointlight_i = 0; simplePointlight_i < simplePointlightCount; simplePointlight_i++)
	{
		// Prerequisite variables
		uint currentSimplePointlight = lightTile.simplePointlights[simplePointlight_i];
		const SimplePointLight light = SimplePointLights[currentSimplePointlight];
		
		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);
		
		float inverseLightDistSqr = 1.0 / (1.0 + (
			pow(toLight.x * light.falloff, 2.0) +
			pow(toLight.y * light.falloff, 2.0) +
			pow(toLight.z * light.falloff, 2.0)
		));
		
		CutoffLight(light.color.xyz, inverseLightDistSqr);
		
		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, normal, light.color.xyz, specularExponent, diffuseLightCol, specularLightCol);
		specularLightCol *= specularColor;
				
		// Apply lighting
		totalDiffuseLight += diffuseLightCol * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * inverseLightDistSqr;
	}
}