#pragma once

#include <array>
#include "Behaviour.h"
#include "CameraCubeBehaviour.h"

class PointLightCollection;

struct SimplePointLightBufferData
{
	DirectX::XMFLOAT3 position = { };
	DirectX::XMFLOAT3 color = { };
	float falloff = 0.0f;
	float padding = 0.0f;
};

class SimplePointLightBehaviour final : public Behaviour
{
private:
	DirectX::XMFLOAT3 _color = { };
	float _falloff = 0.0f;

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

public:
	SimplePointLightBehaviour() = default;
	SimplePointLightBehaviour(DirectX::XMFLOAT3 color, float falloff);
	~SimplePointLightBehaviour();

	[[nodiscard]] SimplePointLightBufferData GetLightBufferData() const;
	void SetLightBufferData(DirectX::XMFLOAT3 color, float falloff);

	[[nodiscard]] bool ContainsPoint(const DirectX::XMFLOAT3A &point) const;
	[[nodiscard]] bool IntersectsLightTile(const DirectX::BoundingFrustum &tile) const;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;

};
