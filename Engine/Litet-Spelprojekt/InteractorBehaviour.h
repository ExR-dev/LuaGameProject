#pragma once
#include "Behaviour.h"

// Behaviour for the player
class InteractorBehaviour : public Behaviour
{
private:
	float _interactionRange = 2.0f;
	
	Entity *_holdingEnt = nullptr;
	Entity *_hidingEnt = nullptr;
	bool _isHolding = false;
	bool _isHiding = false;

	bool _showingPic = false;
	UINT _totalCollected = 0;
	UINT _totalPieces = 5;
	std::vector<Entity *> _picPieces;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

public:
	InteractorBehaviour() = default;
	~InteractorBehaviour() = default;

	float GetInteractionRange() const;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
	void PostDeserialize();

	void ShowPicture();
	void HidePicture();
};

