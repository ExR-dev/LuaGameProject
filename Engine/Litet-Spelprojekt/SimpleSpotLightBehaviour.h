#pragma once

#include "Behaviour.h"
#include "CameraBehaviour.h"

struct SimpleSpotLightBufferData
{
	DirectX::XMFLOAT3 position = { };
	DirectX::XMFLOAT3 direction = { };
	DirectX::XMFLOAT3 color = { };
	float angle = 0.0f;
	float falloff = 0.0f;
	int orthographic = -1;
};

class SimpleSpotLightBehaviour final : public Behaviour
{
private:
	DirectX::XMFLOAT3 _color = { };
	float _falloff = 0.0f;
	float _angle = 0.0f;
	bool _isOrtho = false;

	DirectX::BoundingFrustum _bounds = { };
	bool _recalculateBounds = true;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

	// OnEnable runs immediately after the behaviour is enabled.
	void OnEnable() override;

	// OnEnable runs immediately after the behaviour is disabled.
	void OnDisable() override;

	void OnDirty() override;

public:
	SimpleSpotLightBehaviour() = default;
	SimpleSpotLightBehaviour(DirectX::XMFLOAT3 color, float angle, float falloff, bool isOrtho);
	~SimpleSpotLightBehaviour();

	[[nodiscard]] SimpleSpotLightBufferData GetLightBufferData() const;
	void SetLightBufferData(DirectX::XMFLOAT3 color, float angle, float falloff, bool isOrtho);

	[[nodiscard]] bool ContainsPoint(const DirectX::XMFLOAT3A &point);
	[[nodiscard]] bool IntersectsLightTile(const DirectX::BoundingFrustum &tile);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;

};
