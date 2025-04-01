#include "stdafx.h"
#include "SimplePointLightBehaviour.h"
#include "Entity.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

SimplePointLightBehaviour::SimplePointLightBehaviour(XMFLOAT3 color, float falloff)
{
	_color = color;
	_falloff = falloff;
}
SimplePointLightBehaviour::~SimplePointLightBehaviour()
{
	if (!IsInitialized())
		return;

	if (!IsEnabled())
		return;

	PointLightCollection *pointlights = GetScene()->GetPointlights();
	if (!pointlights)
		return;

	if (!pointlights->UnregisterSimpleLight(this))
		ErrMsg("Failed to unregister simple pointlight!");
}

bool SimplePointLightBehaviour::Start()
{
	if (_name == "")
		_name = "SimplePointLightBehaviour"; // For categorization in ImGui.

	PointLightCollection *pointlights = GetScene()->GetPointlights();

	if (!pointlights)
	{
		ErrMsg("Failed to get pointlight collection!");
		return false;
	}

	if (!pointlights->RegisterSimpleLight(this))
	{
		ErrMsg("Failed to register simple pointlight!");
		return false;
	}

	return true;
}

#ifdef USE_IMGUI
bool SimplePointLightBehaviour::RenderUI()
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
	if (ImGui::DragFloat("Intensity", &colorStrength, 0.01f, 0.001f))
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

	return true;
}
#endif

void SimplePointLightBehaviour::OnEnable()
{
	PointLightCollection *pointlights = GetScene()->GetPointlights();
	if (pointlights)
	{
		if (!pointlights->RegisterSimpleLight(this))
		{
			ErrMsg("Failed to register simple pointlight!");
		}
	}
}
void SimplePointLightBehaviour::OnDisable()
{
	PointLightCollection *pointlights = GetScene()->GetPointlights();
	if (pointlights)
	{
		if (!pointlights->UnregisterSimpleLight(this))
		{
			ErrMsg("Failed to unregister simple pointlight!");
		}
	}
}

bool SimplePointLightBehaviour::Serialize(std::string *code) const
{
	// Standard code for Serialize
	*code += "SimplePointLightBehaviour("
		+ std::to_string(_color.x) + " " + std::to_string(_color.y) + " " + std::to_string(_color.z) + " " + std::to_string(_falloff) +
		" )";

	return true;
}
bool SimplePointLightBehaviour::Deserialize(const std::string &code)
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

	_color.x = attributes.at(0);
	_color.y = attributes.at(1);
	_color.z = attributes.at(2);
	_falloff = attributes.at(3);

	return true;
	return true;
}

SimplePointLightBufferData SimplePointLightBehaviour::GetLightBufferData() const
{
	Transform *transform = GetTransform();

	SimplePointLightBufferData data = { };
	data.position = transform->GetPosition(World);
	data.color = _color;
	data.falloff = _falloff;

	return data;
}
void SimplePointLightBehaviour::SetLightBufferData(XMFLOAT3 color, float falloff)
{
	_color = color;
	_falloff = falloff;
}

bool SimplePointLightBehaviour::ContainsPoint(const XMFLOAT3A &point) const
{
	float reach = CalculateLightReach(_color, _falloff);

	// Test sphere-point intersection.
	BoundingSphere sphereBounds = BoundingSphere(GetTransform()->GetPosition(World), reach);
	
	return sphereBounds.Contains(Load(point));
}
bool SimplePointLightBehaviour::IntersectsLightTile(const DirectX::BoundingFrustum &tile) const
{
	float reach = CalculateLightReach(_color, _falloff);

	BoundingBox boxBounds = BoundingBox(GetTransform()->GetPosition(World), XMFLOAT3(reach, reach, reach));

	// Test box-frustum intersection.
	if (!tile.Intersects(boxBounds))
		return false;

	// Test sphere-frustum intersection.
	BoundingSphere sphereBounds = BoundingSphere(boxBounds.Center, reach);

	return tile.Intersects(sphereBounds);
}
