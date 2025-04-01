#pragma once
#include "Behaviour.h"
#include "MeshBehaviour.h"

class CompassBehaviour final : public Behaviour
{
private:
	Transform *_compassNeedle = nullptr;

	Entity* _compassBodyMesh = nullptr;
	Entity* _compassNeedleMesh = nullptr;

protected:
	[[nodiscard]] bool Start() override;
	[[nodiscard]] bool LateUpdate(Time& time, const Input& input) override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;

public:
	CompassBehaviour() = default;
	~CompassBehaviour() = default;
};