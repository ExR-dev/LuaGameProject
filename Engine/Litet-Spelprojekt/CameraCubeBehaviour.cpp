#include "stdafx.h"
#include "CameraCubeBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "RenderQueuer.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

CameraCubeBehaviour::CameraCubeBehaviour(const CameraPlanes &planes, bool hasCSBuffer, bool invertDepth)
{
	_invertedDepth = invertDepth;
	_hasCSBuffer = hasCSBuffer;
	_cameraPlanes = planes;
}

// Start runs once when the behaviour is created.
bool CameraCubeBehaviour::Start()
{
	if (_name == "")
		_name = "CameraCubeBehaviour"; // For categorization in ImGui.

	ID3D11Device *device = GetScene()->GetDevice();

	const XMMATRIX projMatrix = Load(GetProjectionMatrix());
	const XMFLOAT4A pos = To4(GetTransform()->GetPosition(World));
	for (int i = 0; i < 6; i++)
	{
		const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix(i);
		if (!_cubeSides[i].viewProjBuffer.Initialize(device, sizeof(XMFLOAT4X4A), &viewProjMatrix))
		{
			ErrMsg("Failed to initialize camera VS buffer!");
			return false;
		}

		if (_hasCSBuffer)
		{
			const GeometryBufferData bufferData = { GetViewMatrix(i), pos };
			_cubeSides[i].viewProjPosBuffer = std::make_unique<ConstantBufferD3D11>();
			if (!_cubeSides[i].viewProjPosBuffer->Initialize(device, sizeof(GeometryBufferData), &bufferData))
			{
				ErrMsg("Failed to initialize camera GS buffer!");
				return false;
			}

			_cubeSides[i].posBuffer = std::make_unique<ConstantBufferD3D11>();

			XMFLOAT3A forward = {};
			GetAxes(i, nullptr, nullptr, &forward);

			CameraBufferData camBufferData = { 
				GetViewProjectionMatrix(i),
				GetTransform()->GetPosition(World),
				forward,
				_invertedDepth ? _cameraPlanes.farZ : _cameraPlanes.nearZ,
				_invertedDepth ? _cameraPlanes.nearZ : _cameraPlanes.farZ
			};

			if (!_cubeSides[i].posBuffer->Initialize(device, sizeof(CameraBufferData), &camBufferData))
			{
				ErrMsg("Failed to initialize camera CS buffer!");
				return false;
			}
		}

		// Each camera bounds
		BoundingFrustum::CreateFromMatrix(_cubeSides[i].bounds, projMatrix);

		_cubeSides[i].bounds.Orientation = GetRotation(i);
		_cubeSides[i].transformedBounds = _cubeSides[i].bounds;
	}

	// Entire Cube bounds
	_bounds.Extents = { _cameraPlanes.farZ, _cameraPlanes.farZ, _cameraPlanes.farZ };

	return true;
}

void CameraCubeBehaviour::OnDirty()
{
	_isDirty = true;
	_recalculateBounds = true;
	_recalculateFrustumBounds = true;
}

#ifdef USE_IMGUI
bool CameraCubeBehaviour::RenderUI()
{
	ImGui::Text(std::format("Extents: {}", _cameraPlanes.farZ).c_str());

	return true;
}
#endif

bool CameraCubeBehaviour::Serialize(std::string *code) const
{
	return true;
}
bool CameraCubeBehaviour::Deserialize(const std::string &code)
{
	std::vector<std::string> substrings;
	UINT size = static_cast<UINT>(code.size());
	UINT start = 5;
	while (start < size) {
		UINT blank = static_cast<UINT>(code.find(" ", start + 1));
		std::string number = code.substr(start + 1, blank - start);
		substrings.push_back(number);
		start = blank;
	}
	return true;
}


void CameraCubeBehaviour::GetAxes(UINT cameraIndex, XMFLOAT3A *right, XMFLOAT3A *up, XMFLOAT3A *forward)
{
	switch (cameraIndex)
	{
	case 0:
		if (right)	 *right		= { 0, 0, 1 };
		if (up)		 *up		= { 0, -1, 0 };
		if (forward) *forward	= { 1, 0, 0 };
		break;

	case 1:
		if (right)	 *right		= { 0, 0, -1 };
		if (up)		 *up		= { 0, -1, 0 };
		if (forward) *forward	= { -1, 0, 0 };
		break;

	case 2:
		if (right)	 *right		= { 1, 0, 0 };
		if (up)		 *up		= { 0, 0, 1 };
		if (forward) *forward	= { 0, -1, 0 };
		break;

	case 3:
		if (right)	 *right		= { 1, 0, 0 };
		if (up)		 *up		= { 0, 0, -1 };
		if (forward) *forward	= { 0, 1, 0 };
		break;

	case 4:
		if (right)	 *right		= { 1, 0, 0 };
		if (up)		 *up		= { 0, -1, 0 };
		if (forward) *forward	= { 0, 0, -1 };
		break;

	case 5:
		if (right)	 *right		= { -1, 0, 0 };
		if (up)		 *up		= { 0, -1, 0 };
		if (forward) *forward	= { 0, 0, 1 };
		break;

	default:
		ErrMsg("Invalid camera index!");
		break;
	}
}

XMFLOAT4A CameraCubeBehaviour::GetRotation(UINT cameraIndex)
{
	XMFLOAT4A result;

	switch (cameraIndex)
	{
	case 0:
		Store(result, XMQuaternionRotationRollPitchYaw(0, XM_PIDIV2, XM_PI));
		return result;

	case 1:
		Store(result, XMQuaternionRotationRollPitchYaw(0, -XM_PIDIV2, XM_PI));
		return result;

	case 2:
		Store(result, XMQuaternionRotationRollPitchYaw(XM_PIDIV2, 0, 0));
		return result;

	case 3:
		Store(result, XMQuaternionRotationRollPitchYaw(-XM_PIDIV2, 0, 0));
		return result;

	case 4:
		Store(result, XMQuaternionRotationRollPitchYaw(0, XM_PI, XM_PI));
		return result;

	case 5:
		Store(result, XMQuaternionRotationRollPitchYaw(0, 0, XM_PI));
		return result;

	default:
		ErrMsg("Invalid camera index!");
		return { 0, 0, 0, 1 };
	}
}

XMFLOAT4X4A CameraCubeBehaviour::GetViewMatrix(UINT cameraIndex)
{
	XMFLOAT3A r, u, f;
	GetAxes(cameraIndex, &r, &u, &f);

	XMVECTOR 
		posVec = Load(GetTransform()->GetPosition(World)),
		up = Load(u),
		forward = Load(f);

	XMMATRIX projectionMatrix = XMMatrixLookAtLH(
		posVec,
		posVec + forward,
		up
	);

	XMFLOAT4X4A result;
	Store(result, projectionMatrix);
	return result;
}
XMFLOAT4X4A CameraCubeBehaviour::GetProjectionMatrix() const
{
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		1,
		_cameraPlanes.nearZ,
		_cameraPlanes.farZ
	);

	XMFLOAT4X4A pMat;
	Store(pMat, projectionMatrix);
	return pMat;
}
XMFLOAT4X4A CameraCubeBehaviour::GetViewProjectionMatrix(UINT cameraIndex)
{
	XMFLOAT4X4A vpMatrix = { };

	if (!_invertedDepth)
	{
		Store(vpMatrix, XMMatrixTranspose(Load(GetViewMatrix(cameraIndex)) * Load(GetProjectionMatrix())));
		return vpMatrix;
	}

	XMFLOAT3A r, u, f;
	GetAxes(cameraIndex, &r, &u, &f);

	XMVECTOR
		posVec = Load(GetTransform()->GetPosition(World)),
		up = Load(u),
		forward = Load(f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(
		posVec,
		posVec + forward,
		up
	);

	XMMATRIX projectionMatrix;
	projectionMatrix = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,
		1,
		_cameraPlanes.farZ,
		_cameraPlanes.nearZ
	);

	Store(vpMatrix, XMMatrixTranspose(viewMatrix * projectionMatrix));
	return vpMatrix;
}

bool CameraCubeBehaviour::UpdateBuffers()
{
	if (!_isDirty)
		return true;

	auto context = GetScene()->GetContext();

	for (int i = 0; i < 6; i++)
	{
		CubeSide *side = &_cubeSides[i];

		const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix(i);
		if (!side->viewProjBuffer.UpdateBuffer(context, &viewProjMatrix))
		{
			ErrMsg("Failed to update camera view projection buffer!");
			return false;
		}

		if (side->viewProjPosBuffer != nullptr)
		{
			XMFLOAT3A pos3 = GetTransform()->GetPosition(World);
			const GeometryBufferData bufferData = { viewProjMatrix, To4(pos3) };
			if (!side->viewProjPosBuffer->UpdateBuffer(context, &bufferData))
			{
				ErrMsg("Failed to update camera view projection positon buffer!");
				return false;
			}
		}

		if (side->posBuffer != nullptr)
		{
			XMFLOAT3A forward = {};
			GetAxes(i, nullptr, nullptr, &forward);

			CameraBufferData camBufferData = { 
				GetViewProjectionMatrix(i),
				GetTransform()->GetPosition(World),
				forward,
				_invertedDepth ? _cameraPlanes.farZ : _cameraPlanes.nearZ,
				_invertedDepth ? _cameraPlanes.nearZ : _cameraPlanes.farZ
			};

			if (!side->posBuffer->UpdateBuffer(context, &camBufferData))
			{
				ErrMsg("Failed to update camera position buffer!");
				return false;
			}
		}
	}

	_isDirty = false;
	return true;
}

bool CameraCubeBehaviour::BindShadowCasterBuffers(UINT cameraIndex) const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer(cameraIndex);
	context->VSSetConstantBuffers(1, 1, &vpmBuffer);

	return true;
}
bool CameraCubeBehaviour::BindGeometryBuffers(UINT cameraIndex) const
{
	auto context = GetScene()->GetContext();
	const CubeSide *side = &_cubeSides[cameraIndex];

	ID3D11Buffer *const posBuffer = (side->posBuffer == nullptr) ? nullptr : side->posBuffer->GetBuffer();
	context->HSSetConstantBuffers(3, 1, &posBuffer);

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer(cameraIndex);
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	return true;
}
bool CameraCubeBehaviour::BindLightingBuffers(UINT cameraIndex) const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const camPosBuffer = GetCameraCSBuffer(cameraIndex);
	if (camPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind lighting buffer, camera does not have that buffer!");
		return false;
	}
	context->PSSetConstantBuffers(3, 1, &camPosBuffer);

	return true;
}
bool CameraCubeBehaviour::BindTransparentBuffers(UINT cameraIndex) const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer(cameraIndex);
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	ID3D11Buffer *const camViewPosBuffer = GetCameraGSBuffer(cameraIndex);
	if (camViewPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind geometry buffer, camera does not have that buffer!");
		return false;
	}
	context->GSSetConstantBuffers(0, 1, &camViewPosBuffer);

	ID3D11Buffer *const camPosBuffer = GetCameraCSBuffer(cameraIndex);
	if (camPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind lighting buffer, camera does not have that buffer!");
		return false;
	}
	context->PSSetConstantBuffers(3, 1, &camPosBuffer);

	return true;
}
bool CameraCubeBehaviour::BindViewBuffers(UINT cameraIndex) const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer(cameraIndex);
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	return true;
}
bool CameraCubeBehaviour::BindMainBuffers(UINT cameraIndex) const
{
	auto context = GetScene()->GetContext();
	const CubeSide *side = &_cubeSides[cameraIndex];

	ID3D11Buffer *const posBuffer = (side->posBuffer == nullptr) ? nullptr : side->posBuffer->GetBuffer();
	context->HSSetConstantBuffers(3, 1, &posBuffer);

	return true;
}

bool CameraCubeBehaviour::StoreBounds(BoundingFrustum &bounds, UINT cameraIndex)
{
	if (_recalculateFrustumBounds)
	{
		XMFLOAT3A pos = GetTransform()->GetPosition(World);

		for (int i = 0; i < 6; i++)
		{
			CubeSide *side = &_cubeSides[i];

			side->transformedBounds.Near = _cameraPlanes.nearZ;
			side->transformedBounds.Far = _cameraPlanes.farZ;

			float *closest = &side->transformedBounds.Near;
			if ((*closest) > side->transformedBounds.Far)
				closest = &side->transformedBounds.Far;
			(*closest) = 0.0001f;

			side->transformedBounds.Origin = pos;
		}

		_recalculateFrustumBounds = false;
	}

	bounds = _cubeSides[cameraIndex].transformedBounds;
	return true;
}
bool CameraCubeBehaviour::StoreBounds(BoundingBox &bounds)
{
	if (_recalculateBounds)
	{
		_transformedBounds.Center = GetTransform()->GetPosition(World);
		_transformedBounds.Extents = { _cameraPlanes.farZ, _cameraPlanes.farZ, _cameraPlanes.farZ };
		_recalculateBounds = false;
	}

	bounds = _transformedBounds;
	return true;
}

void CameraCubeBehaviour::QueueGeometry(UINT sideIndex, const ResourceGroup &resource, const RenderInstance &instance)
{
	CubeSide *side = &_cubeSides[sideIndex];
	side->geometryRenderQueue.insert({ resource, instance });
}
void CameraCubeBehaviour::QueueTransparent(UINT sideIndex, const ResourceGroup &resource, const RenderInstance &instance)
{
	CubeSide *side = &_cubeSides[sideIndex];
	side->transparentRenderQueue.insert({ resource, instance });
}
void CameraCubeBehaviour::ResetRenderQueue()
{
	UINT highestDraws = 0;
	for (int i = 0; i < 6; i++)
	{
		UINT drawCount = 
			static_cast<UINT>(_cubeSides[i].geometryRenderQueue.size() +
			_cubeSides[i].transparentRenderQueue.size());

		if (drawCount > highestDraws)
			highestDraws = drawCount;

		_cubeSides[i].geometryRenderQueue.clear();
		_cubeSides[i].transparentRenderQueue.clear();
	}

	if (highestDraws == 0)
		_lastCullCount = 0;
	else if (_lastCullCount == 0)
		_lastCullCount = highestDraws;
}

UINT CameraCubeBehaviour::GetCullCount() const
{
	return _lastCullCount;
}

void CameraCubeBehaviour::SetCullCount(UINT cullCount) 
{
	_lastCullCount = cullCount;
}

std::multimap<ResourceGroup, RenderInstance> &CameraCubeBehaviour::GetGeometryQueue(UINT cameraIndex)
{
	return _cubeSides[cameraIndex].geometryRenderQueue;
}
std::multimap<ResourceGroup, RenderInstance> &CameraCubeBehaviour::GetTransparentQueue(UINT cameraIndex)
{
	return _cubeSides[cameraIndex].transparentRenderQueue;
}

void CameraCubeBehaviour::SetRendererInfo(const RendererInfo &rendererInfo)
{
	_rendererInfo = rendererInfo;
}
void CameraCubeBehaviour::SetFarZ(float farZ)
{
	_cameraPlanes.farZ = farZ;

	_isDirty = true;
	_recalculateBounds = true;
	_recalculateFrustumBounds = true;
}
RendererInfo CameraCubeBehaviour::GetRendererInfo() const
{
	return _rendererInfo;
}
float CameraCubeBehaviour::GetFarZ() const
{
	return _cameraPlanes.farZ;
}

ID3D11Buffer *CameraCubeBehaviour::GetCameraVSBuffer(UINT cameraIndex) const
{
	return _cubeSides[cameraIndex].viewProjBuffer.GetBuffer();
}
ID3D11Buffer *CameraCubeBehaviour::GetCameraGSBuffer(UINT cameraIndex) const
{
	if (_cubeSides[cameraIndex].viewProjPosBuffer == nullptr)
		return nullptr;

	return _cubeSides[cameraIndex].viewProjPosBuffer->GetBuffer();
}
ID3D11Buffer *CameraCubeBehaviour::GetCameraCSBuffer(UINT cameraIndex) const
{
	if (_cubeSides[cameraIndex].posBuffer == nullptr)
		return nullptr;

	return _cubeSides[cameraIndex].posBuffer->GetBuffer();
}