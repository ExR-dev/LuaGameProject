#pragma once
#include "Behaviour.h"
#include "BillboardMeshBehaviour.h"

class FlashlightBehaviour;

enum class BreadcrumbColor
{
	Red,
	Green,
	Blue
};

class BreadcrumbBehaviour : public Behaviour
{
private:
	BillboardMeshBehaviour *_flare = nullptr;
	BreadcrumbColor _color = BreadcrumbColor::Red;
	FlashlightBehaviour *_flashlight = nullptr;

protected:
	[[nodiscard]] bool Start() override;

	[[nodiscard]] bool Update(Time &time, const Input &input) override;

public:
	BreadcrumbBehaviour() = default;
	~BreadcrumbBehaviour() = default;

	BreadcrumbBehaviour(BreadcrumbColor color);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};