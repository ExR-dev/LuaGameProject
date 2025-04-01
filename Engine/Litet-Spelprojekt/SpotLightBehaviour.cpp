#include "stdafx.h"
#include "SpotLightBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "RenderQueuer.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

SpotLightBehaviour::SpotLightBehaviour(CameraBehaviour *camera, XMFLOAT3 color, float falloff, UINT updateFrequency)
{
	if (camera)
	{
		CameraPlanes planes = camera->GetPlanes();

		float reach = CalculateLightReach(color, falloff);

		if (planes.nearZ < planes.farZ)
			planes.farZ = reach;
		else
			planes.nearZ = reach;

		camera->SetPlanes(planes);
	}

	_shadowCamera = camera;
	_color = color;
	_falloff = falloff;
	_updateFrequency = updateFrequency;
}

SpotLightBehaviour::SpotLightBehaviour(ProjectionInfo projInfo, XMFLOAT3 color, float falloff, bool isOrtho, UINT updateFrequency)
{
	float reach = CalculateLightReach(color, falloff);

	if (projInfo.planes.nearZ < projInfo.planes.farZ)
		projInfo.planes.farZ = reach;
	else
		projInfo.planes.nearZ = reach;

	projInfo.planes.nearZ = projInfo.planes.nearZ < 0.05f ? 0.05f : projInfo.planes.nearZ;

	_shadowCamera = nullptr;
	_initialProjInfo = projInfo;
	_color = color;
	_falloff = falloff;
	_ortho = isOrtho;
	_updateFrequency = updateFrequency;
}

SpotLightBehaviour::~SpotLightBehaviour()
{
	if (!IsInitialized())
		return;

	if (!IsEnabled())
		return;

	SpotLightCollection *spotlights = GetScene()->GetSpotlights();
	if (!spotlights)
		return;

	if (!spotlights->UnregisterLight(this))
		ErrMsg("Failed to unregister spotlight!");
}

bool SpotLightBehaviour::Start()
{
	if (_name == "")
		_name = "SpotLightBehaviour"; // For categorization in ImGui.

	if (!_shadowCamera)
	{
		_shadowCamera = new CameraBehaviour(_initialProjInfo, _ortho, true);

		if (!_shadowCamera->Initialize(GetEntity()))
		{
			ErrMsg("Failed to bind shadow camera to spotlight!");
			return false;
		}
		_shadowCamera->SetSerialization(false);
	}

	_shadowCamera->SetRendererInfo({ false, true });

	SpotLightCollection *spotlights = GetScene()->GetSpotlights();

	if (spotlights)
	{
		if (!spotlights->RegisterLight(this))
		{
			ErrMsg("Failed to register spotlight!");
			return false;
		}
	}

	return true;
}

bool SpotLightBehaviour::Update(Time &time, const Input &input)
{
	if (_updateTimer <= 0)
	{
		_updateTimer += _updateFrequency;
		_boundsDirty = true;
	}
	_updateTimer--;

	return true;
}

#ifdef USE_IMGUI
bool SpotLightBehaviour::RenderUI()
{
	if (ImGui::Button("Reset Color"))
		_color = { 1.0f, 1.0f, 1.0f };

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
	if (ImGui::InputFloat("Intensity", &colorStrength))
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

	if (_shadowCamera)
	{
		float angle = _shadowCamera->GetFOV() * RAD_TO_DEG;
		if (ImGui::SliderFloat("Angle", &angle, 0.01f, 179.99f))
			_shadowCamera->SetFOV(angle * DEG_TO_RAD);
	}

	if (recalculateReach)
	{
		if (_shadowCamera)
		{
			CameraPlanes planes = _shadowCamera->GetPlanes();

			float reach = CalculateLightReach(_color, _falloff);

			if (planes.nearZ < planes.farZ)
				planes.farZ = reach;
			else
				planes.nearZ = reach;

			_shadowCamera->SetPlanes(planes);
		}
	}

	return true;
}
#endif

void SpotLightBehaviour::OnEnable()
{
	SpotLightCollection *spotlights = GetScene()->GetSpotlights();
	if (spotlights)
	{
		if (!spotlights->RegisterLight(this))
		{
			ErrMsg("Failed to register spotlight!");
		}
	}

	_updateTimer = 1;
}
void SpotLightBehaviour::OnDisable()
{
	SpotLightCollection *spotlights = GetScene()->GetSpotlights();
	if (spotlights)
	{
		if (!spotlights->UnregisterLight(this))
		{
			ErrMsg("Failed to unregister spotlight!");
		}
	}
}

bool SpotLightBehaviour::Serialize(std::string *code) const
{
	DirectX::XMFLOAT4X4 projectionMatrix = _shadowCamera->GetProjectionMatrix();
	float nearPlane = projectionMatrix._43 / projectionMatrix._33;
	float farPlane = projectionMatrix._43 / (projectionMatrix._33 - 1);

	float fovAngleY = 2.0f * std::atan(1.0f / projectionMatrix._22);
	float aspectRatio = projectionMatrix._22 / projectionMatrix._11;

	// Standard code for Serialize
	*code += "SpotLightBehaviour("
		+ std::to_string(_color.x) + " " + std::to_string(_color.y) + " " + std::to_string(_color.z) + " "
		+ std::to_string(_falloff) + " " + std::to_string(fovAngleY) + " " + std::to_string(aspectRatio)+  " " 
		+ std::to_string(nearPlane) + " " + std::to_string(farPlane) +
		" )";

	return true;
}
bool SpotLightBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<std::string> substrings;
	
	std::istringstream stream(code);
	std::string value;

	while (stream >> value) {  // Automatically handles spaces correctly
		substrings.push_back(value);
	}

	float numbers[8];
	for (int i = 0; i < 8; i++) {
		numbers[i] = stof(substrings[i]);
	}
	// color , falloff
	SetLightBufferData({ numbers[0], numbers[1],numbers[2] }, numbers[3]);
	// fovA, aspectRatio and CameraPlanes
	_initialProjInfo = { numbers[4], numbers[5], numbers[6] * -1 , numbers[7] * -1 };

	return true;
}


void SpotLightBehaviour::SetUpdateTimer(UINT timer)
{
	_updateTimer = timer;
}
bool SpotLightBehaviour::DoUpdate() const
{
	return _updateTimer <= 0;
}
bool SpotLightBehaviour::UpdateBuffers()
{
	if (!DoUpdate())
		return true;

	if (!_shadowCamera->UpdateBuffers())
	{
		ErrMsg("Failed to update shadow camera buffers!");
		return false;
	}

	return true;
}

SpotLightBufferData SpotLightBehaviour::GetLightBufferData()
{
	if (!DoUpdate())
		return _lastLightBufferData;

	Transform *transform = GetTransform();
	CameraBehaviour *cam = _shadowCamera;

	SpotLightBufferData &data = _lastLightBufferData;
	data.vpMatrix = cam->GetViewProjectionMatrix();
	data.position = transform->GetPosition(World);
	data.direction = transform->GetForward(World);
	data.color = _color;
	data.angle = cam->GetFOV();
	data.falloff = _falloff;
	data.orthographic = _shadowCamera->GetOrtho() ? 1 : -1;

	return data;
}
void SpotLightBehaviour::SetLightBufferData(XMFLOAT3 color, float falloff)
{
	_color = color;
	_falloff = falloff;

	if (_shadowCamera)
	{
		CameraPlanes planes = _shadowCamera->GetPlanes();

		float reach = CalculateLightReach(color, falloff);

		if (planes.nearZ < planes.farZ)
			planes.farZ = reach;
		else
			planes.nearZ = reach;

		planes.nearZ = planes.nearZ < 0.05f ? 0.05f : planes.nearZ;

		_shadowCamera->SetPlanes(planes);
	}
}
void SpotLightBehaviour::SetIntensity(float intensity)
{
	float maxChannel = max(_color.x, max(_color.y, _color.z));
	_color.x = (_color.x / maxChannel) * intensity;
	_color.y = (_color.y / maxChannel) * intensity;
	_color.z = (_color.z / maxChannel) * intensity;

	if (_shadowCamera)
	{
		CameraPlanes planes = _shadowCamera->GetPlanes();

		float reach = CalculateLightReach(_color, _falloff);

		if (planes.nearZ < planes.farZ)
			planes.farZ = reach;
		else
			planes.nearZ = reach;

		_shadowCamera->SetPlanes(planes);
	}
}

CameraBehaviour *SpotLightBehaviour::GetShadowCamera() const
{
	return _shadowCamera;
}

bool SpotLightBehaviour::ContainsPoint(const XMFLOAT3A &point)
{
	if (DoUpdate())
	{
		bool failed = false;
#pragma omp critical
		{
			if (_boundsDirty)
			{
				_boundsDirty = false;
				if (!_shadowCamera->StoreBounds(_transformedBounds, false))
				{
					ErrMsg("Failed to store spotlight camera frustum!");
					failed = true;
				}
			}
		}

		if (failed)
			return false;
	}

	return _transformedBounds.Contains(Load(point));
}
bool SpotLightBehaviour::IntersectsLightTile(const BoundingFrustum &tile)
{
	if (DoUpdate())
	{
		bool failed = false;
#pragma omp critical
		{
			if (_boundsDirty)
			{
				_boundsDirty = false;
				if (!_shadowCamera->StoreBounds(_transformedBounds, false))
				{
					ErrMsg("Failed to store spotlight camera frustum!");
					failed = true;
				}
			}
		}

		if (failed)
			return false;
	}

	return tile.Intersects(_transformedBounds);
}