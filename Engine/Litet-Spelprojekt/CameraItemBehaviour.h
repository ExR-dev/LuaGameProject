#pragma once

#include <d3d11_4.h>
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include "Behaviour.h"
#include "SpotLightBehaviour.h"
#include "SoundBehaviour.h"

class CameraItemBehaviour final : public Behaviour
{
private:
	SpotLightBehaviour *_flashLightBehaviour = nullptr;
	SoundBehaviour *_flashSoundBehaviour = nullptr;
	float _flashTimer = 0.0f;
	float _cooldownTimer = 0.0f;
	bool _firstFrameOfFlash = false;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

	[[nodiscard]] bool OnSelect() override;

public:
	CameraItemBehaviour() = default;
	~CameraItemBehaviour() = default;

	void TakePicture(float flashTime);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;

};
