#pragma once

#include "box2d/box2D.h"
#include "Components/Components.h"

class Scene;

class PhysicsHandler
{
public:
	PhysicsHandler();
	~PhysicsHandler();

	void Setup();
	void Update(lua_State* L, Scene* scene) const;

	b2WorldId GetWorldId() const;

	b2BodyId CreateRigidBody(int entity, const ECS::Collider &collider, const ECS::Transform &transform) const;

private:
	b2WorldId m_worldId{};
};
