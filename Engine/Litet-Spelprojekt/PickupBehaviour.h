#pragma once

#include "InteractableBehaviour.h"
#include "Behaviour.h"

class PickupBehaviour : public Behaviour
{
private:
	bool _isHolding = false;

	Transform *_objectTransform = nullptr;
	Transform *_playerTransform = nullptr;

	DirectX::XMFLOAT3A _offset = { -0.9f, -0.6f, 0.8f }; // Local offset from player camera

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

public:
	PickupBehaviour() = default;
	~PickupBehaviour() = default;

	void Pickup();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};

