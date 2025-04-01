
Texture2D<float> HeightMap : register(t0);
sampler Sampler : register(s0);


cbuffer ViewProjMatrixBuffer : register(b0)
{
	float4x4 viewProjMatrix;
}


struct DomainShaderOutput
{
	float4 position			: SV_POSITION;
	float4 world_position	: POSITION;
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
	float2 tex_coord		: TEXCOORD;
};

struct HullShaderOutput
{
	float4 world_position	: POSITION;
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
	float2 tex_coord		: TEXCOORD;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]	: SV_TessFactor;
	float InsideTessFactor	: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DomainShaderOutput main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HullShaderOutput, NUM_CONTROL_POINTS> patch)
{
	DomainShaderOutput output;

	output.world_position = 
		patch[0].world_position * domain.x +
		patch[1].world_position * domain.y + 
		patch[2].world_position * domain.z;

	output.normal = normalize(
		patch[0].normal * domain.x +
		patch[1].normal * domain.y + 
		patch[2].normal * domain.z
	);

	output.tangent = normalize(
		patch[0].tangent * domain.x +
		patch[1].tangent * domain.y + 
		patch[2].tangent * domain.z
	);

	output.tex_coord = 
		patch[0].tex_coord * domain.x +
		patch[1].tex_coord * domain.y + 
		patch[2].tex_coord * domain.z;

	output.world_position += float4(output.normal * HeightMap.SampleLevel(Sampler, output.tex_coord, 0).x * 0.1f, 0.0f);
	output.position = mul(output.world_position, viewProjMatrix);

	return output;
}
