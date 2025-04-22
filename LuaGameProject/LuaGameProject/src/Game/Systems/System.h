#pragma once
#include "dep/EnTT/entt.hpp"

class System
{
public:
	virtual bool OnUpdate(entt::registry &registry, float delta) = 0;
};


class BehaviourSystem : public System
{
public:
	BehaviourSystem(lua_State *L) : L(L) {}

	bool OnUpdate(entt::registry &registry, float delta) final;

private:
	lua_State *L;
};

class DrawSpriteSystem : public System
{
public:
	DrawSpriteSystem() {}

	bool OnUpdate(entt::registry &registry, float delta) final;

private:
};
