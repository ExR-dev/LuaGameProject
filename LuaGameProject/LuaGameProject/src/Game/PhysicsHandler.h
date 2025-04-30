#pragma once

#include "box2d/box2D.h"
#include "Components/Components.h"

class PhysicsHandler
{
public:
	PhysicsHandler();
	~PhysicsHandler();

	void Setup();
	void Update();

	b2WorldId GetWorldId() const;

	b2BodyId CreateRigidBody(const ECS::Transform &transform);

private:
	b2WorldId m_worldId;
};
