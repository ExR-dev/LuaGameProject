#include "stdafx.h"
#include "System.h"
#include <Game/Components/Components.h>

bool BehaviourSystem::OnUpdate(entt::registry &registry, float delta)
{
	ZoneScopedC(RandomUniqueColor());
	// Retrieve all entitites with a behaviour component
	auto view = registry.view<ECS::Behaviour>();

	// & is for capturing L and delta as a reference
	view.each([&](ECS::Behaviour &script) {
		// Retrieve the behaviour table to the top of the stack
		lua_rawgeti(L, LUA_REGISTRYINDEX, script.LuaRef);

		// Retrieve the OnUpdate method from the table
		lua_getfield(L, -1, "OnUpdate");

		// Push the table as the first argument to the method
		lua_pushvalue(L, -2);

		// Push delta as the second argument to the method
		lua_pushnumber(L, delta);

		// Call the method, pops the method and its arguments from the stack
		LuaChk(lua_pcall(L, 2, 0, 0));

		// Pop the behaviour table from the stack
		lua_pop(L, 1);
	});

	// false -> Do not destroy the system
	return false;
}
