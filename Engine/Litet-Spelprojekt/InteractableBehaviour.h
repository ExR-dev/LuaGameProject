#pragma once

#include "Behaviour.h"
#include <vector>
#include <functional>
#include "InteractorBehaviour.h"

// Behaviour for iteractable objects
class InteractableBehaviour : public Behaviour
{
private:
	std::vector<std::function<void()>> _interactionCallbacks;
	bool _isHovered = false;
	int _defaultAmbientId = CONTENT_NULL;
	float _interactionRange = 5.0f;

	InteractorBehaviour *_interactor = nullptr;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;
	[[nodiscard]] bool Update(Time &time, const Input &input) override;
	[[nodiscard]] bool OnHover() override;
	[[nodiscard]] bool OffHover() override;

public:
	InteractableBehaviour() = default;
	~InteractableBehaviour() = default;

	void AddInteractionCallback(std::function<void(void)> callback);

	void OnInteraction();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};