#pragma once

#include "Behaviour.h"
#include "RenderQueuer.h"

class ExampleBehaviour final : public Behaviour
{
private:
	bool _hasCreatedChild = false;
	bool _debugDraw = false;
	bool _overlay = false;
	float _spinSpeed = 1.0f;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

public:
	ExampleBehaviour() = default;
	~ExampleBehaviour() = default;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;
};
