#include "stdafx.h"
#include "SimpleSpotLightBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "RenderQueuer.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

SimpleSpotLightBehaviour::SimpleSpotLightBehaviour(XMFLOAT3 color, float angle, float falloff, bool isOrtho)
{
	_color = color;
	_angle = angle;
	_falloff = falloff;
	_isOrtho = isOrtho;
}
SimpleSpotLightBehaviour::~SimpleSpotLightBehaviour()
{
	if (!IsInitialized())
		return;
	if (!IsEnabled())
		return;

	SpotLightCollection *spotlights = GetScene()->GetSpotlights();
	if (!spotlights)
		return;

	if (!spotlights->UnregisterSimpleLight(this))
		ErrMsg("Failed to unregister simple spotlight!");
}

bool SimpleSpotLightBehaviour::Start()
{
	if (_name == "")
		_name = "SimpleSpotLightBehaviour"; // For categorization in ImGui.

	SpotLightCollection *spotlights = GetScene()->GetSpotlights();

	if (spotlights)
	{
		if (!spotlights->RegisterSimpleLight(this))
		{
			ErrMsg("Failed to register simple spotlight!");
			return false;
		}
	}

	return true;
}

#ifdef USE_IMGUI
bool SimpleSpotLightBehaviour::RenderUI()
{
	float color[3] = { _color.x, _color.y, _color.z };
	float colorStrength = max(color[0], max(color[1], color[2]));

	color[0] /= colorStrength;
	color[1] /= colorStrength;
	color[2] /= colorStrength;

	bool newColor = false;
	if (ImGui::ColorEdit3("Color", color))
		newColor = true;

	bool newStrength = false;
	if (ImGui::InputFloat("Intensity", &colorStrength))
		newStrength = true;

	if (newColor || newStrength)
	{
		float inputStr = max(color[0], max(color[1], color[2]));

		if (inputStr > 0.1f)
		{
			_color.x = color[0] * colorStrength / inputStr;
			_color.y = color[1] * colorStrength / inputStr;
			_color.z = color[2] * colorStrength / inputStr;
		}
	}

	ImGui::DragFloat("Falloff", &_falloff, 0.01f, 0.001f);

	float angle = _angle * RAD_TO_DEG;
	if (ImGui::SliderFloat("Angle", &angle, 0.01f, 359.99f))
	{
		_angle = angle * DEG_TO_RAD;
	}

	return true;
}
#endif

void SimpleSpotLightBehaviour::OnEnable()
{
	SpotLightCollection *spotlights = GetScene()->GetSpotlights();
	if (spotlights)
	{
		if (!spotlights->RegisterSimpleLight(this))
		{
			ErrMsg("Failed to register simple spotlight!");
		}
	}
}
void SimpleSpotLightBehaviour::OnDisable()
{
	SpotLightCollection *spotlights = GetScene()->GetSpotlights();
	if (spotlights)
	{
		if (!spotlights->UnregisterSimpleLight(this))
		{
			ErrMsg("Failed to unregister simple spotlight!");
		}
	}
}

void SimpleSpotLightBehaviour::OnDirty()
{
	_recalculateBounds = true;
}

bool SimpleSpotLightBehaviour::Serialize(std::string *code) const
{
	// Standard code for Serialize
	*code += "SimpleSpotLightBehaviour("
		+ std::to_string(_color.x) + " " + std::to_string(_color.y) + " " + std::to_string(_color.z) + " "
		+ std::to_string(_falloff) + " " + std::to_string(_angle) + 
		" )";

	return true;
}
bool SimpleSpotLightBehaviour::Deserialize(const std::string &code)
{
	return false;
}

SimpleSpotLightBufferData SimpleSpotLightBehaviour::GetLightBufferData() const
{
	Transform *transform = GetTransform();

	SimpleSpotLightBufferData data = { };
	data.position = transform->GetPosition(World);
	data.direction = transform->GetForward(World);
	data.color = _color;
	data.angle = _angle;
	data.falloff = _falloff;
	data.orthographic = _isOrtho ? 1 : -1;

	return data;
}
void SimpleSpotLightBehaviour::SetLightBufferData(XMFLOAT3 color, float angle, float falloff, bool isOrtho)
{
	_color = color;
	_angle = angle;
	_falloff = falloff;
	_isOrtho = isOrtho;

	_recalculateBounds = true;
}

bool SimpleSpotLightBehaviour::ContainsPoint(const XMFLOAT3A &point)
{
	if (_recalculateBounds)
	{
		float reach = CalculateLightReach(_color, _falloff);

		// Get the lights projection matrix and create a frustum from it.
		XMMATRIX projection = XMMatrixPerspectiveFovLH(_angle, 1.0f, 0.01f, reach);
		BoundingFrustum::CreateFromMatrix(_bounds, projection);

		// Transform the frustum to world space.
		XMFLOAT4X4A worldMatrix = GetTransform()->GetWorldMatrix();
		_bounds.Transform(_bounds, Load(worldMatrix));
		_recalculateBounds = false;
	}

	return _bounds.Contains(Load(point));
}
bool SimpleSpotLightBehaviour::IntersectsLightTile(const BoundingFrustum &tile)
{
	if (_recalculateBounds)
	{
		float reach = CalculateLightReach(_color, _falloff);

		// Get the lights projection matrix and create a frustum from it.
		XMMATRIX projection = XMMatrixPerspectiveFovLH(_angle, 1.0f, 0.01f, reach);
		BoundingFrustum::CreateFromMatrix(_bounds, projection);

		// Transform the frustum to world space.
		XMFLOAT4X4A worldMatrix = GetTransform()->GetWorldMatrix();
		_bounds.Transform(_bounds, Load(worldMatrix));
		_recalculateBounds = false;
	}

	return tile.Intersects(_bounds);
}
