#pragma once

#include "InteractableBehaviour.h"
#include "BillboardMeshBehaviour.h"
#include "Behaviour.h"

class BreadcrumbPileBehaviour : public Behaviour
{
private:
	BillboardMeshBehaviour *_flare = nullptr;
	Entity *_flashlightEntity = nullptr;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	[[nodiscard]] bool Update(Time &time, const Input &input) override;

public:
	BreadcrumbPileBehaviour() = default;
	~BreadcrumbPileBehaviour() = default;

	void Pickup();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};

