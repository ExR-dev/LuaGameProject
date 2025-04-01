
cbuffer WorldMatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix inverseTransposeWorldMatrix;
};

struct Particle
{
	float4 position;
	float4 velocity;
	float4 color;
	float4 startPos;
	float4 endPos;
};

StructuredBuffer<Particle> Particles : register(t0);


struct ParticleOut
{
	float3 position : POSITION;
	float4 color : COLOR;
	float size : SIZE;
};


ParticleOut main(const uint vertexID : SV_VertexID)
{
	ParticleOut output;
	output.position = mul(float4(Particles[vertexID].position.xyz, 1.0f), worldMatrix).xyz;
	output.color = Particles[vertexID].color;
	output.size = Particles[vertexID].position.w;
	return output;
}