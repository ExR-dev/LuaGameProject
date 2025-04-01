sampler ShadowSampler : register(s1);

struct SpotLight
{
	float4x4 vp_matrix;
	float3 position;
	float3 direction;
	float3 color;
	float angle;
	float falloff;
	int orthographic;
};
StructuredBuffer<SpotLight> SpotLights : register(t11);
Texture2DArray<float> SpotShadowMaps : register(t5);

struct SimpleSpotLight
{
	float3 position;
	float3 direction;
	float3 color;
	float angle;
	float falloff;
	int orthographic;
};
StructuredBuffer<SimpleSpotLight> SimpleSpotLights : register(t12);

struct PointLight
{
	float4x4 vp_matrix;
	float3 position;
	float3 color;
	float falloff;
	
	float padding;
};
StructuredBuffer<PointLight> PointLights : register(t6);
Texture2DArray<float> PointShadowMaps : register(t7);

struct SimplePointLight
{
	float3 position;
	float3 color;
	float falloff;

	float padding;
};
StructuredBuffer<SimplePointLight> SimplePointLights : register(t13);

static const uint MAX_LIGHTS = 15;
struct LightTile
{
	uint spotlights[MAX_LIGHTS];
	uint pointlights[MAX_LIGHTS*6];
	uint simpleSpotlights[MAX_LIGHTS];
	uint simplePointlights[MAX_LIGHTS];

	uint spotlightCount;
	uint pointlightCount;
	uint simpleSpotlightCount;
	uint simplePointlightCount;
};

StructuredBuffer<LightTile> LightTileBuffer : register(t14);

/// Adds a smoothed cutoff point to lights, beyond which their intensity becomes zero.
/// This is done to prevent harsh edges between light tiles.
/// This cutoff is matched by lightBehaviours using CalculateLightReach() in GameMath.h.
void CutoffLight(float3 color, inout float invSquare)
{
	// The intensity at which the light is considered to have no effect.
	// Must match the value of the same name in CalculateLightReach().
	float intensityCutoff = 0.01; // previously 0.005
	
	float strength = max(color.x, max(color.y, color.z));
	float correction = strength / (strength - intensityCutoff);
	
	float intensity = strength * invSquare;
	
	intensity = max(0.0, correction * (intensity - intensityCutoff));
	
	invSquare = intensity / strength;
}

