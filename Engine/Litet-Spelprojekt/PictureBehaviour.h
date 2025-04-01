#pragma once
#include "Behaviour.h"
#include "InteractableBehaviour.h"
#include "SoundBehaviour.h"

class PictureBehaviour : public Behaviour
{
private:
	Entity *_ent = nullptr;
	Transform *_t = nullptr;
	bool _isGeneric = false;

	DirectX::XMFLOAT3A _offset = { 0.0f, 0.0f, 1.0f }; // Local offset from player camera

	SoundBehaviour *_sound = nullptr;
	
protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] virtual bool Start();

	// OnEnable runs immediately after the behaviour is enabled.
	virtual void OnEnable();

	// OnEnable runs immediately after the behaviour is disabled.
	virtual void OnDisable();

public:
	PictureBehaviour(bool isGeneric);
	PictureBehaviour(DirectX::XMFLOAT3A offset);
	~PictureBehaviour() = default;

	void Pickup();

	// Serializes the behaviour to a string.
	[[nodiscard]] virtual bool Serialize(std::string *code) const;

	// Deserializes the behaviour from a string.
	[[nodiscard]] virtual bool Deserialize(const std::string &code);
};

