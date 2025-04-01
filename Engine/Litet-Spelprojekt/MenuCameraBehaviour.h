#pragma once
#include "Behaviour.h"
#include "Raycast.h"

class MenuCameraBehaviour : public Behaviour
{
private:
	CameraBehaviour *_mainCamera = nullptr;
	Entity *_spotLight = nullptr;
	Entity *_buttons[5] = { nullptr };

	bool _drawPointer = false;

	[[nodiscard]] bool RayCastFromMouse(RaycastOut &out, DirectX::XMFLOAT3A &pos, const Input &input); // Casts a ray from mouse position on nearplane along the z-axis
	
protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] virtual bool Start();

	// Update runs every frame.
	[[nodiscard]] virtual bool Update(Time &time, const Input &input);

public:
	MenuCameraBehaviour() = default;
	~MenuCameraBehaviour() = default;

	// Serializes the behaviour to a string.
	[[nodiscard]] virtual bool Serialize(std::string *code) const;

	// Deserializes the behaviour from a string.
	[[nodiscard]] virtual bool Deserialize(const std::string &code);
};

