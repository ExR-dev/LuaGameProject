#pragma once

#include "InteractableBehaviour.h"
#include "Behaviour.h"
#include "BillboardMeshBehaviour.h"
#include "SoundBehaviour.h"

class HideBehaviour : public Behaviour
{
private:

	Transform *_objectTransform = nullptr;
	Entity *_playerEntity = nullptr;

	SoundBehaviour *_sfx = nullptr;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

public:
	void Hide();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};

