#include "Common.hlsli"


cbuffer ObjectPositionBuffer : register(b0)
{
	float4 objPos;
};


struct VertexShaderOutput
{
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

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
	uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;
	
	/*const float
		d = 0.8f,
		h = 6.0f,
		t = 16.0f;*/
	
	const float
		d = 1.0f,
		h = 1.0f,
		t = 1.0f;

	const float
		distSqr = pow(cam_position.x - objPos.x, 2) + pow(cam_position.y - objPos.y, 2) + pow(cam_position.z - objPos.z, 2),
		tessFactor = clamp((h - d) * t / (distSqr + t) + d, 1, h);

	output.EdgeTessFactor[0] = tessFactor;
	output.EdgeTessFactor[1] = tessFactor;
	output.EdgeTessFactor[2] = tessFactor;
	output.InsideTessFactor = tessFactor;

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HullShaderOutput main( 
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint patchID : SV_PrimitiveID)
{
	HullShaderOutput output;
	
	output.world_position	= ip[i].world_position;
	output.normal			= ip[i].normal;
	output.tangent			= ip[i].tangent;
	output.tex_coord		= ip[i].tex_coord;

	return output;
}
