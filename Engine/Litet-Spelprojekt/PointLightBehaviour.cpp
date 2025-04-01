#include "stdafx.h"
#include "PointLightBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "RenderQueuer.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

PointLightBehaviour::PointLightBehaviour(CameraCubeBehaviour *cameraCube, XMFLOAT3 color, float falloff, UINT updateFrequency)
{
	if (cameraCube)
		cameraCube->SetFarZ(CalculateLightReach(color, falloff));

	_shadowCameraCube = cameraCube;
	_color = color;
	_falloff = falloff;
	_updateFrequency = updateFrequency;
}
PointLightBehaviour::PointLightBehaviour(CameraPlanes planes, XMFLOAT3 color, float falloff, UINT updateFrequency)
{
	float reach = CalculateLightReach(color, falloff);

	if (planes.nearZ < planes.farZ)
		planes.farZ = reach;
	else
		planes.nearZ = reach;

	planes.nearZ = planes.nearZ < 0.05f ? 0.05f : planes.nearZ;

	_shadowCameraCube = nullptr;
	_initialCameraPlanes = planes;
	_color = color;
	_falloff = falloff;
	_updateFrequency = updateFrequency;
}
PointLightBehaviour::~PointLightBehaviour()
{
	if (!IsInitialized())
		return;

	if (!IsEnabled())
		return;

	PointLightCollection *pointlights = GetScene()->GetPointlights();
	if (!pointlights)
		return;

	if (!pointlights->UnregisterLight(this))
		ErrMsg("Failed to unregister pointlight!");
}

bool PointLightBehaviour::Start()
{
	if (_name == "")
		_name = "PointLightBehaviour"; // For categorization in ImGui.

	if (!_shadowCameraCube)
	{
		_shadowCameraCube = new CameraCubeBehaviour(_initialCameraPlanes, false, true);

		if (!_shadowCameraCube->Initialize(GetEntity()))
		{
			ErrMsg("Failed to bind shadow camera cube to pointlight!");
			return false;
		}
	}

	_shadowCameraCube->SetRendererInfo({ false, true });
	_shadowCameraCube->SetSerialization(false);

	PointLightCollection *pointlights = GetScene()->GetPointlights();

	if (!pointlights)
	{
		ErrMsg("Failed to get pointlight collection!");
		return false;
	}

	if (!pointlights->RegisterLight(this))
	{
		ErrMsg("Failed to register pointlight!");
		return false;
	}

	return true;
}

bool PointLightBehaviour::Update(Time &time, const Input &input)
{
	if (_updateTimer <= 0)
	{
		_updateTimer += _updateFrequency;

		for (int i = 0; i < 6; i++)
			_cameraBoundsDirty[i] = true;
		_boundsDirty = true;
	}
	_updateTimer--;

	return true;
}

#ifdef USE_IMGUI
bool PointLightBehaviour::RenderUI()
{
	float color[3] = { _color.x, _color.y, _color.z };
	float colorStrength = max(color[0], max(color[1], color[2]));

	color[0] /= colorStrength;
	color[1] /= colorStrength;
	color[2] /= colorStrength;

	bool recalculateReach = false;

	bool newColor = false;
	if (ImGui::ColorEdit3("Color", color))
		newColor = true;

	bool newStrength = false;
	if (ImGui::DragFloat("Intensity", &colorStrength, 0.01f, 0.001f))
		newStrength = true;

	if (newColor || newStrength)
	{
		recalculateReach = true;
		float inputStr = max(color[0], max(color[1], color[2]));

		if (inputStr > 0.1f)
		{
			_color.x = color[0] * colorStrength / inputStr;
			_color.y = color[1] * colorStrength / inputStr;
			_color.z = color[2] * colorStrength / inputStr;
		}
	}

	if (ImGui::DragFloat("Falloff", &_falloff, 0.01f, 0.001f))
		recalculateReach = true;

	if (recalculateReach && _shadowCameraCube)
		_shadowCameraCube->SetFarZ(CalculateLightReach(_color, _falloff));

	return true;
}
#endif

void PointLightBehaviour::OnEnable()
{
	PointLightCollection *pointlights = GetScene()->GetPointlights();
	if (pointlights)
	{
		if (!pointlights->RegisterLight(this))
		{
			ErrMsg("Failed to register pointlight!");
		}
	}
}
void PointLightBehaviour::OnDisable()
{
	PointLightCollection *pointlights = GetScene()->GetPointlights();
	if (pointlights)
	{
		if (!pointlights->UnregisterLight(this))
		{
			ErrMsg("Failed to unregister pointlight!");
		}
	}
}

bool PointLightBehaviour::Serialize(std::string *code) const
{
	// Save near and far plane for easy initialization
	DirectX::XMFLOAT4X4 projectionMatrix = _shadowCameraCube->GetProjectionMatrix();
	float nearPlane = projectionMatrix._43 / projectionMatrix._33;
	float farPlane = projectionMatrix._43 / (projectionMatrix._33 - 1);
	
	// Standard code for Serialize
	*code += "PointLightBehaviour("
		+ std::to_string(_color.x) + " " + std::to_string(_color.y) + " " + std::to_string(_color.z) + " "
		+ std::to_string(_falloff) + " " + std::to_string(nearPlane) + " " + std::to_string(farPlane) +
		" )";

	return true;
}
bool PointLightBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<std::string> substrings;

	std::istringstream stream(code);
	std::string value;

	while (stream >> value) 
	{  // Automatically handles spaces correctly
		substrings.push_back(value);
	}

	float numbers[6];
	for (int i = 0; i < 6; i++) 
	{
		numbers[i] = stof(substrings[i]);
	}
	SetLightBufferData({ numbers[0], numbers[1] , numbers[2] }, numbers[3]);

	_initialCameraPlanes = { numbers[4] * -1, numbers[5] * -1};

	return true;
}


void PointLightBehaviour::SetUpdateTimer(UINT timer)
{
	_updateTimer = timer;
}
bool PointLightBehaviour::DoUpdate() const
{
	return _updateTimer <= 0;
}
bool PointLightBehaviour::UpdateBuffers()
{
	if (!DoUpdate())
		return true;

	if (!_shadowCameraCube->UpdateBuffers())
	{
		ErrMsg("Failed to update shadow camera cube buffers!");
		return false;
	}

	return true;
}

PointLightBufferData PointLightBehaviour::GetLightBufferData(UINT cameraIndex)
{
	if (!DoUpdate())
		return _lastLightBufferData[cameraIndex];

	Transform *transform = GetTransform();
	CameraCubeBehaviour *cam = _shadowCameraCube;

	PointLightBufferData &data = _lastLightBufferData[cameraIndex];
	data.vpMatrix = cam->GetViewProjectionMatrix(cameraIndex);
	data.position = transform->GetPosition(World);
	data.color = _color;
	data.falloff = _falloff;

	return data;
}
void PointLightBehaviour::SetLightBufferData(XMFLOAT3 color, float falloff)
{
	_color = color;
	_falloff = falloff;

	if (_shadowCameraCube)
		_shadowCameraCube->SetFarZ(CalculateLightReach(color, falloff));
}

CameraCubeBehaviour *PointLightBehaviour::GetShadowCameraCube() const
{
	return _shadowCameraCube;
}

bool PointLightBehaviour::ContainsPoint(const XMFLOAT3A &point)
{
	XMFLOAT3 lightPos = _transformedBounds.Center;
	if (DoUpdate())
	{
#pragma omp critical
		{
			if (_boundsDirty)
				lightPos = GetTransform()->GetPosition(World);
		}
	}

	// Test sphere-point intersection.
	BoundingSphere lightSphere(GetTransform()->GetPosition(World), _shadowCameraCube->GetFarZ());

	return lightSphere.Contains(Load(point));
}
bool PointLightBehaviour::IntersectsLightTile(const BoundingFrustum &tile)
{
	if (DoUpdate())
	{
		bool failed = false;
#pragma omp critical
		{
			if (_boundsDirty)
			{
				_boundsDirty = false;
				if (!_shadowCameraCube->StoreBounds(_transformedBounds))
				{
					ErrMsg("Failed to store spotlight camera frustum!");
					failed = true;
				}
			}
		}

		if (failed)
			return false;
	}

	// Test box-frustum intersection.
	if (!tile.Intersects(_transformedBounds))
		return false;

	// Test sphere-frustum intersection.
	BoundingSphere lightSphere(_transformedBounds.Center, _transformedBounds.Extents.x);

	return tile.Intersects(lightSphere);
}
bool PointLightBehaviour::IntersectsLightTile(UINT cameraIndex, const DirectX::BoundingFrustum &tile)
{
	BoundingFrustum &lightBounds = _cameraTransformedBounds[cameraIndex];
	if (DoUpdate())
	{
		bool failed = false;
#pragma omp critical
		{
			if (_cameraBoundsDirty[cameraIndex])
			{
				_cameraBoundsDirty[cameraIndex] = false;
				if (!_shadowCameraCube->StoreBounds(lightBounds, cameraIndex))
				{
					ErrMsg("Failed to store spotlight camera frustum!");
					failed = true;
				}
			}
		}

		if (failed)
			return false;
	}

	return tile.Intersects(lightBounds);
}
