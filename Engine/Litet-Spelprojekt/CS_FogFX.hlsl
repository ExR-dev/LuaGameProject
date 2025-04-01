#include "Common.hlsli"
#include "LightData.hlsli"


cbuffer DistortionSettings : register(b2)
{
	float3 distortion_source;
	float distortion_distance;
};

cbuffer InverseCameraMatrixBuffer : register(b4)
{
    matrix inverseProjectionMatrix;
    matrix inverseViewMatrix;
};

cbuffer FogSettings : register(b6)
{
	float thickness;
	float stepSize;
	uint minSteps;
	uint maxSteps;
};


Texture2D<float> Input : register(t0);

RWTexture2D<float4> Output : register(u0);


float3 SampleLight(float3 pos, LightTile lightTile)
{
    //float3 totalDiffuseLight = ambient_light.xyz;
    float3 totalDiffuseLight = float3(0,0,0);
	
	// Get spotlight data
	uint spotlightCount = lightTile.spotlightCount;
    
    uint spotWidth, spotHeight, _;
    SpotShadowMaps.GetDimensions(0, spotWidth, spotHeight, _, _);

    const float
		spotDX = 1.0f / (float)spotWidth,
		spotDY = 1.0f / (float)spotHeight;
	
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
		
        const float offsetAngle = saturate(1.0f - (
			light.orthographic > 0 ?
			length(cross(light.direction, toLight)) :
			acos(dot(-toLightDir, light.direction))
			) / (light.angle * 0.5f));
		
		// Calculate shadow projection
        const float4 fragPosLightClip = mul(float4(pos, 1.0f), light.vp_matrix);
        const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
        const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0 && fragPosLightNDC.x < 1.0 &&
			fragPosLightNDC.y > -1.0 && fragPosLightNDC.y < 1.0
		);

		const float3 spotUV = float3((fragPosLightNDC.x * 0.5) + 0.5, (fragPosLightNDC.y * -0.5) + 0.5, currentSpotlight);
        const float spotDepth = SpotShadowMaps.SampleLevel(ShadowSampler, spotUV, 0).x;
        const float spotResult = spotDepth - EPSILON < fragPosLightNDC.z ? 1.0 : 0.0;
		
        const float shadow = isInsideFrustum * saturate(offsetAngle * spotResult);
		
		// Apply lighting
        totalDiffuseLight += light.color.xyz * shadow * inverseLightDistSqr;
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
			(projectedDist > 0.0 ? 1.0 : 0.0) * (1.0 / (1.0 + pow(projectedDist, 2.0))) : // If orthographic
			1.0 / (1.0 + (pow(toLight.x * light.falloff, 2.0) + pow(toLight.y * light.falloff, 2.0) + pow(toLight.z * light.falloff, 2.0))); // If perspective
		
		CutoffLight(light.color.xyz, inverseLightDistSqr);
		
        const float offsetAngle = saturate(1.0f - (
			light.orthographic > 0 ?
			length(cross(light.direction, toLight)) :
			acos(dot(-toLightDir, light.direction))
			) / (light.angle * 0.5f));
		
		// Apply lighting
		totalDiffuseLight += light.color.xyz * saturate(offsetAngle) * inverseLightDistSqr;
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
		
        float inverseLightDistSqr = 1.0 / (1.0 + (
			pow(toLight.x * light.falloff, 2) +
			pow(toLight.y * light.falloff, 2) +
			pow(toLight.z * light.falloff, 2)
		));
		
		CutoffLight(light.color.xyz, inverseLightDistSqr);
		
		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos, 1.0), light.vp_matrix);
        const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
        const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0 && fragPosLightNDC.x < 1.0 &&
			fragPosLightNDC.y > -1.0 && fragPosLightNDC.y < 1.0 &&
			fragPosLightNDC.z > 0.0
		);

		const float3 pointUV = float3((fragPosLightNDC.x * 0.5) + 0.5, (fragPosLightNDC.y * -0.5) + 0.5, currentPointlight);
        const float pointDepth = PointShadowMaps.SampleLevel(ShadowSampler, pointUV, 0).x;
        const float pointResult = pointDepth - EPSILON < fragPosLightNDC.z ? 1.0 : 0.0;
        
        const float shadow = isInsideFrustum * saturate(pointResult);
        
		// Apply lighting
		totalDiffuseLight += light.color.xyz * shadow * inverseLightDistSqr;
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
		
		// Apply lighting
		totalDiffuseLight += light.color.xyz * inverseLightDistSqr;
	}
    
    return totalDiffuseLight;
}

void RayStepping(
	float Length, float3 Start, float3 Dir, LightTile lightTile, // View params
	float thickness, float Step, float MinSteps, float MaxSteps, // Setting params
	out float3 Color, out float Density)
{
    Color = float3(0.0, 0.0, 0.0);
    Density = 0.0;

    Step = max(0.1, min(Step, Length / MinSteps - 0.075));
    if (Length / Step > MaxSteps)
    {
        Step = (Length / MaxSteps) + 0.075;
    }
	
    float3 samplePos = Start;
    float3 thisStepColor;

    float3 prevStepColor = SampleLight(samplePos, lightTile);
	
	uint seed = abs(randInt + (Length * 642616.742947315) + (Dir.x * -4657647.1751857) + (Dir.y * 133451.6456564) + (Dir.z * 5685732.4565677));
	
	float distortionStaticRadius = pow(RandomValue(seed), 6.0);
	
    int i = 0;
    for (float dist = Step; dist < Length; dist += Step)
    {
		float randOffset = (RandomValue(seed) - 0.5) * Step;
		
		samplePos = Start + (dist + randOffset) * Dir;
		thisStepColor = SampleLight(samplePos, lightTile);
		
        Color += (prevStepColor + thisStepColor) / 2.0 * Step;
        prevStepColor = thisStepColor;
		
		Density += thickness * Step;
		
		// Sample Distortion
		float distToSource = length(samplePos - distortion_source);
		float distortion = Step * pow(1.0 - saturate(distToSource / (distortion_distance * 0.2)), 1.25);
		float dS = Step * pow(1.0 - saturate(distToSource / (distortion_distance * 0.5 * distortionStaticRadius)), 0.75);
		
		Color -= 0.1 * ((0.1 * distortion) + (0.1 * dS));
		Density += 0.1 * ((6.0 * distortion) + (12.0 * dS));

		if (++i > MaxSteps)
		{
			Color /= 20.0;
			Density /= 20.0;
			//Color /= i;
			//Density /= i;
			return;
		}
    }

    samplePos = Start + Length * Dir;
	thisStepColor = SampleLight(samplePos, lightTile);
	
    Color += (prevStepColor + thisStepColor) / 2.0 * (frac(Length / Step) * Step);
	Color /= 20.0;
    
	Density += thickness * (frac(Length / Step) * Step);
	Density /= 20.0;
}


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int2 inDim, outDim;
    Input.GetDimensions(inDim.x, inDim.y);
    Output.GetDimensions(outDim.x, outDim.y);
    
    float2 inPixelSize = float2(outDim.xy) / float2(inDim.xy);
    
	float2 uv = (float2(DTid.xy) + float2(0.5, 0.5)) / float2(outDim);
    float2 uvPixelSize = inPixelSize / float2(outDim);
    
    float 
        depth00 = Input.SampleLevel(Sampler, uv + float2(-uvPixelSize.x, -uvPixelSize.y), 0),
        depth01 = Input.SampleLevel(Sampler, uv + float2(uvPixelSize.x, -uvPixelSize.y), 0),
        depth10 = Input.SampleLevel(Sampler, uv + float2(-uvPixelSize.x, uvPixelSize.y), 0),
        depth11 = Input.SampleLevel(Sampler, uv + float2(uvPixelSize.x, uvPixelSize.y), 0);
    
	float depth = (depth00 + depth01 + depth10 + depth11) / 4.0;
    
    float2 clipSpace = float2(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0);
    float4 viewRay = float4(clipSpace, 1.0, 1.0);
    viewRay = mul(inverseProjectionMatrix, viewRay);
    viewRay /= viewRay.w;

    float3 rayWorldDir = mul(inverseViewMatrix, float4(viewRay.xyz, 0.0)).xyz;
	rayWorldDir = normalize(rayWorldDir);
    
    
	// Calculate light tile position
	const float4 screenPosClip = mul(float4(cam_position.xyz + rayWorldDir, 1.0f), view_proj_matrix);
	const float3 screenPosNDC = screenPosClip.xyz / screenPosClip.w;
	const float2 screenUV = float2((screenPosNDC.x * 0.5f) + 0.5f, (screenPosNDC.y * 0.5f) + 0.5f);
		
    uint lightTiles, lightTileDim, _;
    LightTileBuffer.GetDimensions(lightTiles, _);
    lightTileDim = sqrt(lightTiles);
	
    uint2 lightTileCoord = uint2(screenUV * float(lightTileDim));
    int lightTileIndex = lightTileCoord.x + (lightTileCoord.y * lightTileDim);
	
	LightTile lightTile = LightTileBuffer[lightTileIndex];
	
    
    float3 currStep = cam_position;
	float3 stepDir = rayWorldDir;
    
	float3 color = float3(0.0, 0.0, 0.0);
    float density = 0.0;
    
	RayStepping(
        depth, currStep, stepDir, lightTile, // View
        thickness, stepSize, minSteps, maxSteps, // Setting
        color, density // Output
    );
	
	color = max(0.0, color);
	
	float transmittance = 1.0 - exp(-density);
	Output[DTid.xy] = float4(color, transmittance);
}