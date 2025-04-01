#include "stdafx.h"
#include "CameraBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "RenderQueuer.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

CameraBehaviour::CameraBehaviour(const ProjectionInfo &projectionInfo, bool isOrtho, bool invertDepth)
{
	_invertedDepth = invertDepth;
	_ortho = isOrtho;
	_currProjInfo = _defaultProjInfo = projectionInfo;
}

// Start runs once when the behaviour is created.
bool CameraBehaviour::Start()
{
	if (_name == "")
		_name = "CameraBehaviour"; // For categorization in ImGui.

	ID3D11Device *device = GetScene()->GetDevice();

	const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix();
	if (!_viewProjBuffer.Initialize(device, sizeof(XMFLOAT4X4A), &viewProjMatrix))
	{
		ErrMsg("Failed to initialize camera VS buffer!");
		return false;
	}

	const XMFLOAT4A pos = To4(GetTransform()->GetPosition());
	const GeometryBufferData bufferData = { GetViewMatrix(), pos };
	_viewProjPosBuffer = std::make_unique<ConstantBufferD3D11>();
	if (!_viewProjPosBuffer->Initialize(device, sizeof(GeometryBufferData), &bufferData))
	{
		ErrMsg("Failed to initialize camera GS buffer!");
		return false;
	}

	_invCamBuffer = std::make_unique<ConstantBufferD3D11>();
	XMFLOAT4X4 invCamBufferData[2] = { };
	Store(invCamBufferData[0], XMMatrixInverse(nullptr, Load(GetProjectionMatrix())));
	Store(invCamBufferData[1], XMMatrixInverse(nullptr, Load(GetViewMatrix())));

	if (!_invCamBuffer->Initialize(device, sizeof(invCamBufferData), &invCamBufferData))
	{
		ErrMsg("Failed to initialize inverse camera buffer!");
		return false;
	}

	_posBuffer = std::make_unique<ConstantBufferD3D11>();
	CameraBufferData camBufferData = { 
		GetViewProjectionMatrix(),
		GetTransform()->GetPosition(World),
		GetTransform()->GetForward(World),
		_invertedDepth ? _currProjInfo.planes.farZ : _currProjInfo.planes.nearZ,
		_invertedDepth ? _currProjInfo.planes.nearZ : _currProjInfo.planes.farZ
	};

	if (!_posBuffer->Initialize(device, sizeof(CameraBufferData), &camBufferData))
	{
		ErrMsg("Failed to initialize camera CS buffer!");
		return false;
	}

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.planes.nearZ,
			farZ = _currProjInfo.planes.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3(width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3(width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3(width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	return true;
}

void CameraBehaviour::OnDirty()
{
	_isDirty = true;
	_recalculateBounds = true;
}

// Update runs every frame.
bool CameraBehaviour::Update(Time &time, const Input &input)
{
	if (_debugDraw)
	{
		using namespace DebugDraw;

		XMFLOAT3 corners[8]{};

		if (_ortho)
		{
			BoundingOrientedBox transformedBounds;
			if (!StoreBounds(transformedBounds, false))
			{
				ErrMsg("Failed to store camera bounds!");
				return false;
			}

			transformedBounds.GetCorners(corners);
		}
		else
		{
			BoundingFrustum transformedBounds;
			if (!StoreBounds(transformedBounds, false))
			{
				ErrMsg("Failed to store camera bounds!");
				return false;
			}

			transformedBounds.GetCorners(corners);
		}

		// Draw line segment between 0-1-2-3-0, 4-5-6-7-4, 0-4, 1-5, 2-6, 3-7
		// LineSection takes a point, a size and a color.
		// Line takes two LineSections.
		std::vector<LineSection> nearLineStrip; // size = 0.05f, color = green
		std::vector<LineSection> farLineStrip; // size = 0.05f, color = red
		std::vector<Line> connectingLines; // size = 0.05f, color = yellow

		for (int i = 0; i < 4; i++)
		{
			nearLineStrip.push_back({ corners[i], 0.05f, { 0.0f, 1.0f, 0.0f, 1 } });
			farLineStrip.push_back({ corners[i + 4], 0.05f, { 1.0f, 0.0f, 0.0f, 1 } });
			connectingLines.push_back({ 
				{ corners[i], 0.05f, { 1.0f, 1.0f, 0.0f, 1 } }, 
				{ corners[i + 4], 0.05f, { 1.0f, 1.0f, 0.0f, 1 } } 
				});
		}
		nearLineStrip.push_back({ corners[0], 0.05f, { 0.0f, 1.0f, 0.0f, 1 } });
		farLineStrip.push_back({ corners[4], 0.05f, { 1.0f, 0.0f, 0.0f, 1 } });

		DebugDrawer *debugDraw = GetScene()->GetDebugDrawer();
		debugDraw->DrawLineStrip(nearLineStrip, !_overlayDraw);
		debugDraw->DrawLineStrip(farLineStrip, !_overlayDraw);
		debugDraw->DrawLines(connectingLines, !_overlayDraw);
	}

	return true;
}

#ifdef USE_IMGUI
// RenderUI runs every frame during ImGui rendering if the entity is selected.
bool CameraBehaviour::RenderUI()
{
	if (ImGui::Button("Control"))
		GetScene()->SetViewCamera(this);

	ImGui::Checkbox("Debug Draw", &_debugDraw);
	
	if (_debugDraw)
		ImGui::Checkbox("Overlayed", &_overlayDraw); 
	
	bool valueChanged = false;

	CameraPlanes planes = GetPlanes();
	valueChanged |= ImGui::SliderFloat("NearZ:", &planes.nearZ, 0.01f, planes.farZ - 0.001f);
	valueChanged |= ImGui::SliderFloat("FarZ:", &planes.farZ, planes.nearZ + 0.001f, 750);

	if (valueChanged)
		SetPlanes(planes);

	return true;
}
#endif

bool CameraBehaviour::Serialize(std::string *code) const
{
	*code += _name + "("
		+ std::to_string(_defaultProjInfo.fovAngleY) + " " + std::to_string(_defaultProjInfo.aspectRatio) + " "
		+ std::to_string(_defaultProjInfo.planes.nearZ) + " " + std::to_string(_defaultProjInfo.planes.farZ) + " "
		+ std::to_string(_currProjInfo.fovAngleY) + " " + std::to_string(_currProjInfo.aspectRatio) + " "
		+ std::to_string(_currProjInfo.planes.nearZ) + " " + std::to_string(_currProjInfo.planes.farZ) + " "
		+ std::to_string(_ortho) + " " + std::to_string(_invertedDepth) +
		+ " )";
	return true;
}
bool CameraBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<float> attributes;
	std::istringstream stream(code);

	std::string value;
	while (stream >> value) // Automatically handles spaces correctly
	{
		float attribute = std::stof(value);
		attributes.push_back(attribute);
	}
	
	_defaultProjInfo.fovAngleY = attributes.at(0);
	_defaultProjInfo.aspectRatio = attributes.at(1);
	_defaultProjInfo.planes.nearZ = attributes.at(2);
	_defaultProjInfo.planes.farZ = attributes.at(3);

	_currProjInfo.fovAngleY = attributes.at(4);
	_currProjInfo.aspectRatio = attributes.at(5);
	_currProjInfo.planes.nearZ = attributes.at(6);
	_currProjInfo.planes.farZ = attributes.at(7);

	_ortho = attributes.at(8);
	_invertedDepth = attributes.at(9);

	return true;
}


XMFLOAT4X4A CameraBehaviour::GetViewMatrix()
{
	XMFLOAT3A r, u, f;
	GetTransform()->GetAxes(&r, &u, &f, World);

	XMVECTOR
		posVec = Load(GetTransform()->GetPosition(World)),
		up = Load(u),
		forward = Load(f);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(
		posVec,
		posVec + forward,
		up
	);

	XMFLOAT4X4A result;
	Store(result, viewMatrix);
	return result;
}
XMFLOAT4X4A CameraBehaviour::GetProjectionMatrix() const
{
	XMMATRIX projectionMatrix;

	if (_ortho)
	{
		projectionMatrix = XMMatrixOrthographicLH(
			_currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			_currProjInfo.fovAngleY,
			_currProjInfo.planes.nearZ,
			_currProjInfo.planes.farZ
		);
	}
	else
	{
		projectionMatrix = XMMatrixPerspectiveFovLH(
			_currProjInfo.fovAngleY,
			_currProjInfo.aspectRatio,
			_currProjInfo.planes.nearZ,
			_currProjInfo.planes.farZ
		);
	}

	return *reinterpret_cast<XMFLOAT4X4A *>(&projectionMatrix);
}
XMFLOAT4X4A CameraBehaviour::GetViewProjectionMatrix()
{
	XMFLOAT4X4A vpMatrix = { };

	if (!_invertedDepth)
	{
		Store(vpMatrix, XMMatrixTranspose(Load(GetViewMatrix()) * Load(GetProjectionMatrix())));
		return vpMatrix;
	}

	XMFLOAT3A r, u, f;
	GetTransform()->GetAxes(&r, &u, &f, World);

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
	if (_ortho)
	{
		projectionMatrix = XMMatrixOrthographicLH(
			_currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			_currProjInfo.fovAngleY,
			_currProjInfo.planes.farZ,
			_currProjInfo.planes.nearZ
		);
	}
	else
	{
		projectionMatrix = XMMatrixPerspectiveFovLH(
			_currProjInfo.fovAngleY,
			_currProjInfo.aspectRatio,
			_currProjInfo.planes.farZ,
			_currProjInfo.planes.nearZ
		);
	}

	Store(vpMatrix, XMMatrixTranspose(viewMatrix * projectionMatrix));
	return vpMatrix;
}
const ProjectionInfo &CameraBehaviour::GetCurrProjectionInfo() const
{
	return _currProjInfo;
}

bool CameraBehaviour::ScaleToContents(const std::vector<XMFLOAT4A> &nearBounds, const std::vector<XMFLOAT4A> &innerBounds)
{
	if (!_ortho)
		return false;

	XMFLOAT3A f, r, u;
	GetTransform()->GetAxes(&r, &u, &f);

	const XMVECTOR
		forward = Load(f),
		right = Load(r),
		up = Load(u);

	XMVECTOR mid = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (const XMFLOAT4A &point : innerBounds)
		mid = XMVectorAdd(mid, TO_CONST_VEC(point));
	mid = XMVectorScale(mid, 1.0f / static_cast<float>(innerBounds.size()));

	float
		nearDist = FLT_MAX, farDist = -FLT_MAX,
		leftDist = FLT_MAX, rightDist = -FLT_MAX,
		downDist = FLT_MAX, upDist = -FLT_MAX;

	float
		sceneFarDist = -FLT_MAX,
		sceneLeftDist = FLT_MAX, sceneRightDist = -FLT_MAX,
		sceneDownDist = FLT_MAX, sceneUpDist = -FLT_MAX;

	for (const XMFLOAT4A &point : nearBounds)
	{
		const XMVECTOR pointVec = TO_CONST_VEC(point);
		const XMVECTOR toPoint = pointVec - mid;

		const float
			xDot = XMVectorGetX(XMVector3Dot(toPoint, right)),
			yDot = XMVectorGetX(XMVector3Dot(toPoint, up)),
			zDot = XMVectorGetX(XMVector3Dot(toPoint, forward));

		if (xDot < sceneLeftDist)	sceneLeftDist = xDot;
		if (xDot > sceneRightDist)	sceneRightDist = xDot;

		if (yDot < sceneDownDist)	sceneDownDist = yDot;
		if (yDot > sceneUpDist)		sceneUpDist = yDot;

		if (zDot < nearDist)		nearDist = zDot;
		if (zDot > sceneFarDist)	sceneFarDist = zDot;
	}

	for (const XMFLOAT4A &point : innerBounds)
	{
		const XMVECTOR pointVec = TO_CONST_VEC(point);
		const XMVECTOR toPoint = pointVec - mid;

		const float
			xDot = XMVectorGetX(XMVector3Dot(toPoint, right)),
			yDot = XMVectorGetX(XMVector3Dot(toPoint, up)),
			zDot = XMVectorGetX(XMVector3Dot(toPoint, forward));

		if (xDot < leftDist)	leftDist = xDot;
		if (xDot > rightDist)	rightDist = xDot;

		if (yDot < downDist)	downDist = yDot;
		if (yDot > upDist)		upDist = yDot;

		if (zDot > farDist)		farDist = zDot;
	}

	if (sceneLeftDist > leftDist)	leftDist = sceneLeftDist;
	if (sceneRightDist < rightDist)	rightDist = sceneRightDist;
	if (sceneDownDist > downDist)	downDist = sceneDownDist;
	if (sceneUpDist < upDist)		upDist = sceneUpDist;

	if (farDist - nearDist < 0.001f)
	{
		ErrMsg("Near and far planes are very close, camera can likely be disabled.");
		return false;
	}

	XMFLOAT4A newPos = { 0, 0, 0, 0 };
	TO_VEC(newPos) = XMVectorAdd(mid, XMVectorScale(forward, nearDist - 1.0f));
	TO_VEC(newPos) = XMVectorAdd(TO_VEC(newPos), XMVectorScale(right, (rightDist + leftDist) * 0.5f));
	TO_VEC(newPos) = XMVectorAdd(TO_VEC(newPos), XMVectorScale(up, (upDist + downDist) * 0.5f));

	GetTransform()->SetPosition(newPos);

	const float
		nearZ = 1.0f,
		farZ = (farDist - nearDist) + 1.0f,
		width = (rightDist - leftDist) * 0.5f,
		height = (upDist - downDist) * 0.5f;

	const XMFLOAT3 corners[8] = {
		XMFLOAT3(-width, -height, nearZ),
		XMFLOAT3(width, -height, nearZ),
		XMFLOAT3(-width,  height, nearZ),
		XMFLOAT3(width,  height, nearZ),
		XMFLOAT3(-width, -height, farZ),
		XMFLOAT3(width, -height, farZ),
		XMFLOAT3(-width,  height, farZ),
		XMFLOAT3(width,  height, farZ)
	};

	BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));

	_currProjInfo.planes.nearZ = nearZ;
	_currProjInfo.planes.farZ = farZ;
	_currProjInfo.fovAngleY = height * 2.0f;
	_currProjInfo.aspectRatio = width / height;

	_isDirty = true;
	return true;
}
bool CameraBehaviour::FitPlanesToPoints(const std::vector<XMFLOAT4A> &points)
{
	const float
		currNear = _currProjInfo.planes.nearZ,
		currFar = _currProjInfo.planes.farZ;

	const XMFLOAT3A f = GetTransform()->GetForward();
	const XMVECTOR direction = Load(f);
	const XMVECTOR origin = Load(GetTransform()->GetPosition());

	float minDist = FLT_MAX, maxDist = -FLT_MAX;
	for (const XMFLOAT4A &point : points)
	{
		const XMVECTOR pointVec = TO_CONST_VEC(point);
		const XMVECTOR toPoint = pointVec - origin;

		const float dot = XMVectorGetX(XMVector3Dot(toPoint, direction));

		if (dot < minDist)
			minDist = dot;

		if (dot > maxDist)
			maxDist = dot;
	}

	//_currProjInfo.planes.nearZ = max(minDist, _defaultProjInfo.planes.nearZ);
	_currProjInfo.planes.farZ = min(maxDist, _defaultProjInfo.planes.farZ);

	if (_currProjInfo.planes.farZ - _currProjInfo.planes.nearZ < 0.001f)
	{
		ErrMsg("Near and far planes are very close, camera could be ignored.");
		return false;
	}

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.planes.nearZ,
			farZ = _currProjInfo.planes.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3(width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3(width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3(width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	if (abs(currNear - _currProjInfo.planes.nearZ) + abs(currFar - _currProjInfo.planes.farZ) > 0.01f)
	{
		_isDirty = true;
		_recalculateBounds = true;
		_lightGridFrustums.clear();
	}
	return true;
}

bool CameraBehaviour::UpdateBuffers()
{
	if (!_isDirty)
		return true;

	auto context = GetScene()->GetContext();

	const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix();
	if (!_viewProjBuffer.UpdateBuffer(context, &viewProjMatrix))
	{
		ErrMsg("Failed to update camera view projection buffer!");
		return false;
	}

	if (_invCamBuffer != nullptr)
	{
		XMFLOAT4X4 invCamBufferData[2] = { };
		Store(invCamBufferData[0], XMMatrixInverse(nullptr, Load(GetProjectionMatrix())));
		Store(invCamBufferData[1], XMMatrixInverse(nullptr, Load(GetViewMatrix())));

		if (!_invCamBuffer->UpdateBuffer(context, &invCamBufferData))
		{
			ErrMsg("Failed to update inverse camera buffer!");
			return false;
		}
	}

	if (_viewProjPosBuffer != nullptr)
	{
		XMFLOAT3A pos3 = GetTransform()->GetPosition();
		const GeometryBufferData bufferData = { viewProjMatrix, To4(pos3) };
		if (!_viewProjPosBuffer->UpdateBuffer(context, &bufferData))
		{
			ErrMsg("Failed to update camera view projection positon buffer!");
			return false;
		}
	}

	if (_posBuffer != nullptr)
	{
		CameraBufferData camBufferData = { 
			GetViewProjectionMatrix(),
			GetTransform()->GetPosition(World),
			GetTransform()->GetForward(World),
			_invertedDepth ? _currProjInfo.planes.farZ : _currProjInfo.planes.nearZ,
			_invertedDepth ? _currProjInfo.planes.nearZ : _currProjInfo.planes.farZ
		};

		if (!_posBuffer->UpdateBuffer(context, &camBufferData))
		{
			ErrMsg("Failed to update camera position buffer!");
			return false;
		}
	}

	_isDirty = false;
	return true;
}

bool CameraBehaviour::BindDebugDrawBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const camViewPosBuffer = GetCameraGSBuffer();
	if (camViewPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind geometry buffer, camera does not have that buffer!");
		return false;
	}
	context->GSSetConstantBuffers(0, 1, &camViewPosBuffer);

	return true;
}
bool CameraBehaviour::BindShadowCasterBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->VSSetConstantBuffers(1, 1, &vpmBuffer);

	return true;
}
bool CameraBehaviour::BindGeometryBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const posBuffer = (_posBuffer == nullptr) ? nullptr : _posBuffer->GetBuffer();
	context->HSSetConstantBuffers(3, 1, &posBuffer);

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	return true;
}
bool CameraBehaviour::BindPSLightingBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const camPosBuffer = GetCameraCSBuffer();
	if (camPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind PS lighting buffer, camera does not have that buffer!");
		return false;
	}
	context->PSSetConstantBuffers(3, 1, &camPosBuffer);

	return true;
}
bool CameraBehaviour::BindCSLightingBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const camPosBuffer = GetCameraCSBuffer();
	if (camPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind CS lighting buffer, camera does not have that buffer!");
		return false;
	}
	context->CSSetConstantBuffers(3, 1, &camPosBuffer);

	return true;
}
bool CameraBehaviour::BindTransparentBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	ID3D11Buffer *const camViewPosBuffer = GetCameraGSBuffer();
	if (camViewPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind geometry buffer, camera does not have that buffer!");
		return false;
	}
	context->GSSetConstantBuffers(0, 1, &camViewPosBuffer);

	if (!BindPSLightingBuffers())
	{
		ErrMsg("Failed to bind lighting buffer, camera does not have that buffer!");
		return false;
	}

	return true;
}
bool CameraBehaviour::BindViewBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	ID3D11Buffer *const posBuffer = (_posBuffer == nullptr) ? nullptr : _posBuffer->GetBuffer();
	context->HSSetConstantBuffers(3, 1, &posBuffer);


	context->VSSetConstantBuffers(1, 1, &vpmBuffer);

	return true;
}
bool CameraBehaviour::BindInverseBuffers() const
{
	auto context = GetScene()->GetContext();

	ID3D11Buffer *const invBuffer = _invCamBuffer->GetBuffer();
	context->CSSetConstantBuffers(4, 1, &invBuffer);

	return true;
}

bool CameraBehaviour::StoreBounds(BoundingFrustum &bounds, bool includeScale)
{
	if (_ortho)
		return false;

	if (_recalculateBounds || _transformedWithScale != includeScale)
	{
		XMFLOAT4X4A worldMatrix;

		if (includeScale)
			worldMatrix = GetTransform()->GetWorldMatrix();
		else
			worldMatrix = GetTransform()->GetUnscaledWorldMatrix();

		_bounds.perspective.Transform(_transformedBounds.perspective, Load(worldMatrix));

		_recalculateBounds = false;
		_transformedWithScale = includeScale;
	}

	bounds = _transformedBounds.perspective;
	return true;
}
bool CameraBehaviour::StoreBounds(BoundingOrientedBox &bounds, bool includeScale)
{
	if (!_ortho)
		return false;

	if (_recalculateBounds || _transformedWithScale != includeScale)
	{
		XMFLOAT4X4A worldMatrix;

		if (includeScale)
			worldMatrix = GetTransform()->GetWorldMatrix();
		else
			worldMatrix = GetTransform()->GetUnscaledWorldMatrix();

		_bounds.ortho.Transform(_transformedBounds.ortho, Load(worldMatrix));

		_recalculateBounds = false;
		_transformedWithScale = includeScale;
	}

	bounds = _transformedBounds.ortho;
	return true;
}

const BoundingFrustum *CameraBehaviour::GetLightGridFrustums()
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(942655105, "Get Light Tiles");
#endif

	if (_ortho)
	{
		ErrMsg("Cannot get light tiles as frustums from an orthographic camera!");
		return nullptr;
	}

	if (_lightGridFrustums.empty())
	{
		Graphics *graphics = GetScene()->GetGraphics();
		const UINT lightTileCount = LIGHT_GRID_RES * LIGHT_GRID_RES;

		_lightGridFrustums.resize(lightTileCount);
		_transformedLightGridFrustums.resize(lightTileCount);

		XMMATRIX projMatrix = Load(GetProjectionMatrix());

		float nearZ = _currProjInfo.planes.nearZ;
		float farZ = _currProjInfo.planes.farZ;

		// Extract frustum planes at near plane (assuming symmetric perspective projection)
		float m11 = XMVectorGetX(projMatrix.r[0]);
		float m22 = XMVectorGetY(projMatrix.r[1]);

		float r = nearZ / m11;	// Right plane at nearZ
		float t = nearZ / m22;	// Top plane
		float l = -r;			// Left plane
		float b = -t;			// Bottom plane

#pragma warning(disable: 6993)
#pragma omp parallel for num_threads(PARALLEL_THREADS)
#pragma warning(default: 6993)
		for (int tileY = 0; tileY < LIGHT_GRID_RES; ++tileY)
		{
			for (int tileX = 0; tileX < LIGHT_GRID_RES; ++tileX)
			{
				// Calculate NDC boundaries for this tile
				float xMinNDC = -1.0f + (static_cast<float>(tileX) / LIGHT_GRID_RES) * 2.0f;
				float xMaxNDC = -1.0f + (static_cast<float>(tileX + 1) / LIGHT_GRID_RES) * 2.0f;
				float yMinNDC = -1.0f + (static_cast<float>(tileY) / LIGHT_GRID_RES) * 2.0f;
				float yMaxNDC = -1.0f + (static_cast<float>(tileY + 1) / LIGHT_GRID_RES) * 2.0f;

				// Convert NDC to parametric space [0, 1]
				float txMin = (xMinNDC + 1.0f) * 0.5f;
				float txMax = (xMaxNDC + 1.0f) * 0.5f;
				float tyMin = (yMinNDC + 1.0f) * 0.5f;
				float tyMax = (yMaxNDC + 1.0f) * 0.5f;

				// Calculate tile frustum planes in view space
				float tileL = l + (r - l) * txMin;
				float tileR = l + (r - l) * txMax;
				float tileB = b + (t - b) * tyMin;
				float tileT = b + (t - b) * tyMax;

				// Create tile projection matrix
				XMMATRIX projTile = XMMatrixPerspectiveOffCenterLH(tileL, tileR, tileB, tileT, nearZ, farZ);

				// Store bounding frustum
				BoundingFrustum::CreateFromMatrix(_lightGridFrustums.at(tileX + tileY * LIGHT_GRID_RES), projTile);
			}
		}
	}

	XMMATRIX worldMatrix = Load(GetTransform()->GetUnscaledWorldMatrix());

	UINT tileCount = static_cast<UINT>(_lightGridFrustums.size());
	for (UINT i = 0; i < tileCount; i++)
	{
		BoundingFrustum transformedTile; 
		_lightGridFrustums[i].Transform(_transformedLightGridFrustums[i], worldMatrix);
	}

	return _transformedLightGridFrustums.data();
}

void CameraBehaviour::QueueGeometry(const ResourceGroup &resource, const RenderInstance &instance)
{
	if (resource.overlay)
		_overlayRenderQueue.insert({ resource, instance });
	else
		_geometryRenderQueue.insert({ resource, instance });
}
void CameraBehaviour::QueueTransparent(const ResourceGroup &resource, const RenderInstance &instance)
{
	_transparentRenderQueue.insert({ resource, instance });
}
void CameraBehaviour::ResetRenderQueue()
{
	_lastCullCount = static_cast<UINT>(
		_geometryRenderQueue.size() +
		_transparentRenderQueue.size() +
		_overlayRenderQueue.size()
		);

	_geometryRenderQueue.clear();
	_transparentRenderQueue.clear();
	_overlayRenderQueue.clear();
}

UINT CameraBehaviour::GetCullCount() const
{
	return _lastCullCount;
}

std::multimap<ResourceGroup, RenderInstance> &CameraBehaviour::GetGeometryQueue()
{
	return _geometryRenderQueue;
}
std::multimap<ResourceGroup, RenderInstance> &CameraBehaviour::GetTransparentQueue()
{
	return _transparentRenderQueue;
}
std::multimap<ResourceGroup, RenderInstance> &CameraBehaviour::GetOverlayQueue()
{
	return _overlayRenderQueue;
}

void CameraBehaviour::SetFOV(const float fov)
{
	_currProjInfo.fovAngleY = fov;

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.planes.nearZ,
			farZ = _currProjInfo.planes.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3(width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3(width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3(width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	_isDirty = true;
	_recalculateBounds = true;
	_lightGridFrustums.clear();
}
void CameraBehaviour::SetOrtho(bool state)
{
	_ortho = state;

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.planes.nearZ,
			farZ = _currProjInfo.planes.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3(width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3(width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3(width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	_isDirty = true;
	_recalculateBounds = true;
	_lightGridFrustums.clear();
}
void CameraBehaviour::SetPlanes(CameraPlanes planes)
{
	_currProjInfo.planes = planes;

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.planes.nearZ,
			farZ = _currProjInfo.planes.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3(width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3(width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3(width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	_isDirty = true;
	_recalculateBounds = true;
	_lightGridFrustums.clear();
}

float CameraBehaviour::GetFOV() const
{
	return _currProjInfo.fovAngleY;
}
bool CameraBehaviour::GetOrtho() const
{
	return _ortho;
}
CameraPlanes CameraBehaviour::GetPlanes() const
{
	return _currProjInfo.planes;
}

void CameraBehaviour::SetRendererInfo(const RendererInfo &rendererInfo)
{
	_rendererInfo = rendererInfo;
}
RendererInfo CameraBehaviour::GetRendererInfo() const
{
	return _rendererInfo;
}

ID3D11Buffer *CameraBehaviour::GetCameraVSBuffer() const
{
	return _viewProjBuffer.GetBuffer();
}
ID3D11Buffer *CameraBehaviour::GetCameraGSBuffer() const
{
	if (_viewProjPosBuffer == nullptr)
		return nullptr;

	return _viewProjPosBuffer->GetBuffer();
}
ID3D11Buffer *CameraBehaviour::GetCameraCSBuffer() const
{
	if (_posBuffer == nullptr)
		return nullptr;

	return _posBuffer->GetBuffer();
}


void CameraBehaviour::GetViewRay(const XMFLOAT2A &screenPos, const XMFLOAT2A &screenSize, XMFLOAT3A &origin, XMFLOAT3A &direction)
{
	// Wiewport to NDC coordinates
	float xNDC = ((2.0f * screenPos.x) / screenSize.x) - 1.0f;
	float yNDC = 1.0f - ((2.0f * screenPos.y) / screenSize.y); // can also be - 1 depending on coord-system
	float zNDC = 1.0f;	// not really needed yet (specified anyways)
	XMFLOAT3A ray_clip = XMFLOAT3A(xNDC, yNDC, zNDC);

	// Wiew space -> clip space
	XMVECTOR rayClipVec = Load(ray_clip);
	XMMATRIX projMatrix = Load(GetProjectionMatrix());
	XMVECTOR rayEyeVec = XMVector4Transform(rayClipVec, XMMatrixInverse(nullptr, projMatrix));

	// Set z and w to mean forwards and not a point
	rayEyeVec = XMVectorSet(XMVectorGetX(rayEyeVec), XMVectorGetY(rayEyeVec), 1, 0.0f);

	// Clip space -> world space
	XMMATRIX viewMatrix = Load(GetViewMatrix());
	XMVECTOR rayWorldVec = XMVector4Transform(rayEyeVec, XMMatrixInverse(nullptr, viewMatrix));

	rayWorldVec = XMVector4Normalize(rayWorldVec);	// Normalize
	XMFLOAT3A dir; Store(dir, rayWorldVec);

	// Camera 
	XMFLOAT3A camPos = GetTransform()->GetPosition(World);

	// Perform raycast
	origin = { camPos.x, camPos.y, camPos.z };
	direction = { dir.x, dir.y, dir.z };

}
