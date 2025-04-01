#pragma once

#include "InteractableBehaviour.h"
#include "Behaviour.h"
//#include "BillboardMeshBehaviour.h"

class EndCutSceneBehaviour : public Behaviour
{
private:
	const std::string _cutSceneName = "endscene5";

	Transform* _objectTransform = nullptr;
	Entity* _playerEntity = nullptr;

	bool _cutSceneFinished = false;
	bool _startCutScene = false;

	//BillboardMeshBehaviour* _flare = nullptr;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time& time, const Input& input) override;

public:

	void CutScene();
	void Hide() = delete;
	bool IsHiding() const = delete;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;
};
