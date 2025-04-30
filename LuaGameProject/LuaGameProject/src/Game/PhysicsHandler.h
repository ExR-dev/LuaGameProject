#pragma once

#include "box2d/box2D.h"
#include "Components/Components.h"

class PhysicsHandler
{
public:
	PhysicsHandler();
	~PhysicsHandler();

	void Setup();
	void Update(lua_State* L);

	b2WorldId GetWorldId() const;

	b2BodyId CreateRigidBody(const ECS::Collider &collider, const ECS::Transform &transform);

private:
	b2WorldId m_worldId;
};
