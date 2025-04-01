#pragma once

#include <array>
#include "Behaviour.h"
#include "CameraCubeBehaviour.h"

class PointLightCollection;

struct PointLightBufferData
{
	DirectX::XMFLOAT4X4 vpMatrix = { };
	DirectX::XMFLOAT3 position = { };
	DirectX::XMFLOAT3 color = { };
	float falloff = 0.0f;
	float padding = 0.0f;
};

class PointLightBehaviour final : public Behaviour
{
private:
	CameraPlanes _initialCameraPlanes = { };
	DirectX::XMFLOAT3 _color = { };
	float _falloff = 0.0f;

	UINT _updateFrequency = 2;
	int _updateTimer = 1;

	CameraCubeBehaviour *_shadowCameraCube = nullptr;
	PointLightBufferData _lastLightBufferData[6] = { {}, {}, {}, {}, {}, {} };

	DirectX::BoundingBox _transformedBounds = { };
	DirectX::BoundingFrustum _cameraTransformedBounds[6] = { {}, {}, {}, {}, {}, {} };

	bool _cameraBoundsDirty[6] = { true, true, true, true, true, true };
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
	PointLightBehaviour() = default;
	PointLightBehaviour(CameraCubeBehaviour *cameraCube, DirectX::XMFLOAT3 color, float falloff, UINT updateFrequency = 3);
	PointLightBehaviour(CameraPlanes planes, DirectX::XMFLOAT3 color, float falloff, UINT updateFrequency = 3);
	~PointLightBehaviour();

	void SetUpdateTimer(UINT timer);
	[[nodiscard]] bool DoUpdate() const;
	[[nodiscard]] bool UpdateBuffers();

	[[nodiscard]] PointLightBufferData GetLightBufferData(UINT cameraIndex);
	void SetLightBufferData(DirectX::XMFLOAT3 color, float falloff);

	[[nodiscard]] CameraCubeBehaviour *GetShadowCameraCube() const;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;

	[[nodiscard]] bool ContainsPoint(const DirectX::XMFLOAT3A &point);
	[[nodiscard]] bool IntersectsLightTile(const DirectX::BoundingFrustum &tile);
	[[nodiscard]] bool IntersectsLightTile(UINT cameraIndex, const DirectX::BoundingFrustum &tile);
};
