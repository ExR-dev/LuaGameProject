#pragma once

#include <d3d11_4.h>
#include <wrl/client.h>
#include <DirectXCollision.h>
#include <vector>

#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"


struct SimpleVertex {
	float
		x, y, z, s,
		r, g, b, a;

	SimpleVertex() :
		x(0.0f), y(0.0f), z(0.0f), s(0.0f),
		r(0.0f), g(0.0f), b(0.0f), a(0.0f)
	{ }

	SimpleVertex(
		const float x, const float y, const float z,  const float s,
		const float r, const float g, const float b, const float a) :
		x(x), y(y), z(z), s(s),
		r(r), g(g), b(b), a(a)
	{ }

	bool operator==(const SimpleVertex &other) const
	{
		if (x != other.x) return false;
		if (y != other.y) return false;
		if (z != other.z) return false;
		if (s != other.s) return false;

		if (r != other.r) return false;
		if (g != other.g) return false;
		if (b != other.b) return false;
		if (a != other.a) return false;

		return true;
	}
};

struct SimpleMeshData
{
	struct VertexInfo
	{
		UINT sizeOfVertex = 0;
		UINT nrOfVerticesInBuffer = 0;
		float *vertexData = nullptr;

		~VertexInfo() { delete[] vertexData; }
	} vertexInfo;
};

class SimpleMeshD3D11
{
private:
	VertexBufferD3D11 _vertexBuffer;

public:
	SimpleMeshD3D11() = default;
	~SimpleMeshD3D11() = default;
	SimpleMeshD3D11(const SimpleMeshD3D11 &other) = delete;
	SimpleMeshD3D11 &operator=(const SimpleMeshD3D11 &other) = delete;
	SimpleMeshD3D11(SimpleMeshD3D11 &&other) = delete;
	SimpleMeshD3D11 &operator=(SimpleMeshD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const SimpleMeshData &meshInfo);

	void BindMeshBuffers(ID3D11DeviceContext *context, UINT stride = 0, UINT offset = 0) const;
	void PerformDrawCall(ID3D11DeviceContext *context) const;
};