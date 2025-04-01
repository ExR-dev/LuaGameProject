#include "stdafx.h"
#include "MeshD3D11.h"

#include "ErrMsg.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using Microsoft::WRL::ComPtr;


MeshD3D11::~MeshD3D11()
{
	
}


bool MeshD3D11::Initialize(ID3D11Device *device, const MeshData &meshInfo)
{
	if (!_vertexBuffer.Initialize(device, meshInfo.vertexInfo.sizeOfVertex, meshInfo.vertexInfo.nrOfVerticesInBuffer, meshInfo.vertexInfo.vertexData))
	{
		ErrMsg("Failed to initialize vertex buffer!");
		return false;
	}

	if (!_indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData))
	{
		ErrMsg("Failed to initialize index buffer!");
		return false;
	}

	const size_t subMeshCount = meshInfo.subMeshInfo.size();
	for (size_t i = 0; i < subMeshCount; i++)
	{
		SubMeshD3D11 subMesh;
		if (!subMesh.Initialize(device,
			meshInfo.subMeshInfo.at(i).startIndexValue, 
			meshInfo.subMeshInfo.at(i).nrOfIndicesInSubMesh,
			meshInfo.subMeshInfo.at(i).ambientTexturePath,
			meshInfo.subMeshInfo.at(i).diffuseTexturePath,
			meshInfo.subMeshInfo.at(i).specularTexturePath,
			meshInfo.subMeshInfo.at(i).specularExponent))
		{
			ErrMsg("Failed to initialize sub mesh!");
			return false;
		}

		_subMeshes.push_back(std::move(subMesh));
	}

	_boundingBox = meshInfo.boundingBox;
	//_mtlFile = meshInfo.mtlFile;

	return true;
}


bool MeshD3D11::BindMeshBuffers(ID3D11DeviceContext *context, UINT stride, const UINT offset) const
{
	if (stride == 0)
		stride = static_cast<UINT>(_vertexBuffer.GetVertexSize());

	ID3D11Buffer *const vertxBuffer = _vertexBuffer.GetBuffer();
	context->IASetVertexBuffers(0, 1, &vertxBuffer, &stride, &offset);

	ID3D11Buffer *const indexBuffer = _indexBuffer.GetBuffer();
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	return true;
}

bool MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext *context, const UINT subMeshIndex) const
{
	if (!_subMeshes.at(subMeshIndex).PerformDrawCall(context))
	{
		ErrMsg(std::format("Failed to perform draw call for sub mesh #{}!", subMeshIndex));
		return false;
	}
	return true;
}

const DirectX::BoundingOrientedBox &MeshD3D11::GetBoundingOrientedBox() const
{
	return _boundingBox;
 }

const std::string &MeshD3D11::GetMaterialFile() const
{
	return _mtlFile;
}


UINT MeshD3D11::GetNrOfSubMeshes() const
{
	return static_cast<UINT>(_subMeshes.size());
}


const std::string &MeshD3D11::GetAmbientPath(const UINT subMeshIndex) const
{
	return _subMeshes.at(subMeshIndex).GetAmbientPath();
}

const std::string &MeshD3D11::GetDiffusePath(const UINT subMeshIndex) const
{
	return _subMeshes.at(subMeshIndex).GetDiffusePath();
}

const std::string &MeshD3D11::GetSpecularPath(const UINT subMeshIndex) const
{
	return _subMeshes.at(subMeshIndex).GetSpecularPath();
}

ID3D11Buffer *MeshD3D11::GetSpecularBuffer(const UINT subMeshIndex) const
{
	return _subMeshes.at(subMeshIndex).GetSpecularBuffer();
}
