 #pragma once

#include "Behaviour.h"
#include "CameraBehaviour.h"

struct SpotLightBufferData
{
	DirectX::XMFLOAT4X4 vpMatrix = { };
	DirectX::XMFLOAT3 position = { };
	DirectX::XMFLOAT3 direction = { };
	DirectX::XMFLOAT3 color = { };
	float angle = 0.0f;
	float falloff = 0.0f;
	int orthographic = -1;
};

class SpotLightBehaviour final : public Behaviour
{
private:
	ProjectionInfo _initialProjInfo = { };
	DirectX::XMFLOAT3 _color = { };
	float _falloff = 0.0f;
	bool _ortho = false;

	UINT _updateFrequency = 2;
	int _updateTimer = 1;

	CameraBehaviour *_shadowCamera = nullptr;
	SpotLightBufferData _lastLightBufferData = { };

	DirectX::BoundingFrustum _transformedBounds = { };
	bool _boundsDirty = true;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

	// OnEnable runs immediately after the behaviour is enabled.
	void OnEnable() override;

	// OnEnable runs immediately after the behaviour is disabled.
	void OnDisable() override;


public:
	SpotLightBehaviour() = default;
	SpotLightBehaviour(CameraBehaviour *camera, DirectX::XMFLOAT3 color, float falloff, UINT updateFrequency = 2);
	SpotLightBehaviour(ProjectionInfo projInfo, DirectX::XMFLOAT3 color, float falloff, bool isOrtho = false, UINT updateFrequency = 2);
	~SpotLightBehaviour();

	void SetUpdateTimer(UINT timer);
	[[nodiscard]] bool DoUpdate() const;
	[[nodiscard]] bool UpdateBuffers();

	[[nodiscard]] SpotLightBufferData GetLightBufferData();
	void SetLightBufferData(DirectX::XMFLOAT3 color, float falloff);
	void SetIntensity(float intensity);

	[[nodiscard]] CameraBehaviour *GetShadowCamera() const;
	
	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;

	
	[[nodiscard]] bool ContainsPoint(const DirectX::XMFLOAT3A &point);
	[[nodiscard]] bool IntersectsLightTile(const DirectX::BoundingFrustum &tile);
};
