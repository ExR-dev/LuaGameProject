#pragma once
#include "dep/EnTT/entt.hpp"

class System
{
public:
	virtual bool OnUpdate(entt::registry &registry, float delta) = 0;
	virtual bool OnRender(entt::registry &registry, float delta) = 0;
	virtual bool OnNamedMethod(entt::registry &registry, const char *name, std::function<void(lua_State *)> &paramFunc) = 0;
};


class BehaviourSystem : public System
{
public:
	BehaviourSystem(lua_State *L) : L(L) {}

	bool OnUpdate(entt::registry &registry, float delta) final;
	bool OnRender(entt::registry &registry, float delta) final;
	bool OnNamedMethod(entt::registry &registry, const char *name, std::function<void(lua_State *)> &paramFunc) final;

private:
	lua_State *L;

	bool RunDeltaMethod(entt::registry &registry, float delta, const char *name);
};
