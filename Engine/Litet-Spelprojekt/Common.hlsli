static const float EPSILON = 0.00005;
static const float NORMAL_OFFSET = 0.0025;
static const float PI = 3.14159265;

sampler Sampler : register(s0);
Texture2D NoiseTexture : register(t10);

cbuffer GlobalLight : register(b0)
{
	float4 ambient_light; // Use alpha channel for screen fade-out
};

cbuffer CameraData : register(b3)
{
	matrix view_proj_matrix;
	float4 cam_position;
	float4 cam_direction;
	float2 cam_planes;
	float cam_padding[2];
};

cbuffer GeneralData : register(b5)
{
	float time;
	float deltaTime;
	int randInt;
	float randNorm;
};


uint NextRandom(inout uint state)
{
	state = state * 747796405u + 2891336453u;
	uint result = ((state >> ((state >> 28) + 4u)) ^ state) * 277803737u;
	result = (result >> 22) ^ result;
	return result;
}

float RandomValue(inout uint state)
{
	return float(NextRandom(state)) / 4294967295.0;
}

// Generic color-clamping algorithm, not mine but it looks good
float3 ACESFilm(const float3 x)
{
	return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

float DistributionGGX(float3 normal, float3 halfway, float roughness)
{
	float a2 = roughness * roughness;
	float NdotH = max(dot(normal, halfway), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return nom / denom;
}

void BlinnPhong(float3 toLightDir, float3 viewDir, float3 normal, float3 lightCol, float specularity, out float3 diffuse, out float3 specular)
{
	const float3 halfwayDir = normalize(toLightDir + viewDir);
		
	float directionScalar = max(dot(normal, toLightDir), 0.0);
	diffuse = lightCol * directionScalar;
	
	//const float specFactor = pow(max(dot(normal, halfwayDir), 0.0), specularity);
	const float specFactor = DistributionGGX(normal, halfwayDir, 1.0 - specularity);
	specular = lightCol * directionScalar * smoothstep(0.0, 1.0, specFactor);
}

float LinearizeDepth(float depthNDC, float near, float far)
{
    // Transform depth from [0, 1] to [-1, 1] (NDC space)
    float depth = depthNDC * 2.0 - 1.0;

    // Linearize depth (convert to view space)
    float linearDepth = (2.0 * near * far) / (far + near - depth * (far - near));

    // Normalize to [0, 1] for visualization
    return (linearDepth - near) / (far - near);
}

