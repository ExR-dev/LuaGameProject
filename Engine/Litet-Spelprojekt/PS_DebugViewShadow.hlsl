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
    const float3 norm = normalize(input.normal);
    const float3 viewDir = normalize(cam_position.xyz - pos);
	
    float totalLights = 0.0f;
    float totalShadow = 0.0f;


    uint spotlightCount, spotWidth, spotHeight, _u;
    SpotLights.GetDimensions(spotlightCount, _u);
    SpotShadowMaps.GetDimensions(0, spotWidth, spotHeight, _u, _u);
    totalLights += spotlightCount;

    const float
		spotDX = 1.0f / (float) spotWidth,
		spotDY = 1.0f / (float) spotHeight;

	// Per-spotlight calculations
    for (uint spotlight_i = 0; spotlight_i < spotlightCount; spotlight_i++)
    {
		// Calculate shadow projection
        const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), SpotLights[spotlight_i].vp_matrix);
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
		
        const float shadow = saturate(
			lerp(
				lerp(spotResult00, spotResult10, fracTex.x),
				lerp(spotResult01, spotResult11, fracTex.x),
			fracTex.y)
		);
		
		// Apply shadow
        totalShadow += 1.0f - saturate(dot(-normalize(SpotLights[spotlight_i].position - pos), SpotLights[spotlight_i].direction) >= 0.0f ? (isInsideFrustum ? saturate(shadow) : 1.0f) : 1.0f);
    }
	
	
    uint pointlightCount, pointWidth, pointHeight;
    PointLights.GetDimensions(pointlightCount, _u);
    PointShadowMaps.GetDimensions(0, pointWidth, pointHeight, _u, _u);
    totalLights += pointlightCount;
	
    const float
		pointDX = 1.0f / (float) pointWidth,
		pointDY = 1.0f / (float) pointHeight;

	// Per-pointlight calculations
    for (uint pointlight_i = 0; pointlight_i < pointlightCount; pointlight_i++)
    {
		// Calculate shadow projection
        const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), PointLights[pointlight_i].vp_matrix);
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
		
        const float shadow = saturate(lerp(
			lerp(pointResult00, pointResult10, fracTex.x),
			lerp(pointResult01, pointResult11, fracTex.x),
			fracTex.y)
		);
		
		// Apply shadow
        totalShadow += (1.0f - shadow);
    }
	
	
    const float result = 1.0f - (totalShadow / totalLights);
    return float4(result, result, result, 1.0f);
}