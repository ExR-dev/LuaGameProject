#pragma once
#include "Behaviour.h"
#include "ColliderBehaviour.h"
#include "PlayerMovementBehaviour.h"
#include "InteractorBehaviour.h"
#include "Raycast.h"

class PlayerViewBehaviour : public Behaviour
{
private:
	PlayerMovementBehaviour *_movement = nullptr;
	InteractorBehaviour *_interactor = nullptr;
	CameraBehaviour *_camera = nullptr;
	DirectX::XMFLOAT3A _startPos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3A _targetPos = { 0.0f, 0.0f, 0.0f };
	float _moveInertia = 1.25f;
	float _targetLerpTime = 0.0f;
	float _targetLerpTimeTotalInverse = 1.0f;

	bool RayCastFromCamera(RaycastOut &out, CameraBehaviour *camera);

protected:
	[[nodiscard]] bool Start() override;

	[[nodiscard]] bool Update(Time &time, const Input &input) override;

	[[nodiscard]] bool FixedUpdate(const float &deltaTime, const Input &input) override;

#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI() override;
#endif

public:
	PlayerViewBehaviour() = default;
	~PlayerViewBehaviour() = default;

	PlayerViewBehaviour(PlayerMovementBehaviour *movementBehaviour);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};