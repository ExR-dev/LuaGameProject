#pragma once

#include "Behaviour.h"

class ExampleCollisionBehaviour : public Behaviour
{
protected:
	[[nodiscard]] bool Start() override;

	[[nodiscard]] bool Update(Time &time, const Input &input) override;

public:
	ExampleCollisionBehaviour() = default;
	~ExampleCollisionBehaviour() = default;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};
