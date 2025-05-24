#include "stdafx.h"
#include "System.h"
#include <Game/Components/Components.h>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool BehaviourSystem::OnUpdate(entt::registry &registry, float delta)
{
	ZoneScopedC(RandomUniqueColor());

	std::function bindDeltaFunc = [&](lua_State *L) {
		lua_pushnumber(L, delta);
		};

	return OnNamedMethod(registry, "OnUpdate", bindDeltaFunc);
}

bool BehaviourSystem::OnRender(entt::registry &registry, float delta)
{
	ZoneScopedC(RandomUniqueColor());

	std::function bindDeltaFunc = [&](lua_State *L) {
		lua_pushnumber(L, delta);
	};

	return OnNamedMethod(registry, "OnRender", bindDeltaFunc);
}

bool BehaviourSystem::OnNamedMethod(entt::registry &registry, const char *name, std::function<void(lua_State *)> &paramFunc)
{
	// Retrieve all entitites with a behaviour component
	auto view = registry.view<ECS::Behaviour>();

	// & is for capturing L and delta as a reference
	view.each([&](entt::entity entity, ECS::Behaviour &script) {

		// If the entity has an active component, check if it is active
		if (registry.all_of<ECS::Active>(entity))
		{
			ECS::Active &active = registry.get<ECS::Active>(entity);
			if (!active.IsActive)
				return;
		}

		if (script.IsUnownedMethod(name))
			return;
		
		// Retrieve the behaviour table to the top of the stack
		lua_rawgeti(L, LUA_REGISTRYINDEX, script.LuaRef);

		// Retrieve the requested method from the table
		lua_getfield(L, -1, name);

		// Check if the method exists before calling it
		if (lua_isnil(L, -1)) // TODO: Game sometimes crashes here suddenly while shooting
		{
			lua_pop(L, 1); // Pop nil
			script.AddUnownedMethod(name);
		}
		else
		{
			// Push the table as the first argument to the method
			lua_pushvalue(L, -2);

			// Push parameters using the given function if given
			if (paramFunc)
				paramFunc(L);

			// Call the method, pops the method and its arguments from the stack
			LuaChk(L, lua_pcall(L, 2, 0, 0));
		}

		// Pop the behaviour table from the stack
		lua_pop(L, 1);
	});

	// false -> Do not destroy the system
	return false;
}

bool BehaviourSystem::RunDeltaMethod(entt::registry &registry, float delta, const char *name)
{
	// Retrieve all entitites with a behaviour component
	auto view = registry.view<ECS::Behaviour>();

	// & is for capturing L and delta as a reference
	view.each([&](entt::entity entity, ECS::Behaviour &script) {

		// If the entity has an active component, check if it is active
		if (registry.all_of<ECS::Active>(entity))
		{
			ECS::Active &active = registry.get<ECS::Active>(entity);
			if (!active.IsActive)
				return;
		}

		// Retrieve the behaviour table to the top of the stack
		lua_rawgeti(L, LUA_REGISTRYINDEX, script.LuaRef);

		// Retrieve the requested method from the table
		lua_getfield(L, -1, name);

		// Check if the method exists before calling it
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1); // Pop nil
		}
		else
		{
			// Push the table as the first argument to the method
			lua_pushvalue(L, -2);

			// Push delta as the second argument to the method
			lua_pushnumber(L, delta);

			// Call the method, pops the method and its arguments from the stack
			LuaChk(L, lua_pcall(L, 2, 0, 0));
		}

		// Pop the behaviour table from the stack
		lua_pop(L, 1);
	});

	// false -> Do not destroy the system
	return false;
}
