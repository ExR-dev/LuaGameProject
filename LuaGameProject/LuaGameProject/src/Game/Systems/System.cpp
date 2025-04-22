#include "stdafx.h"
#include "System.h"
#include <Game/Components/Components.h>

bool BehaviourSystem::OnUpdate(entt::registry &registry, float delta)
{
	// Retrieve all entitites with a behaviour component
	auto view = registry.view<ECS::Behaviour>();

	// & is for capturing L and delta as a reference
	view.each([&](ECS::Behaviour &script) {
		// Retrieve the behaviour table to the top of the stack
		lua_rawgeti(L, LUA_REGISTRYINDEX, script.LuaRef); // TODO: What is LuaTableRef?

		// Retrieve the OnUpdate method from the table
		lua_getfield(L, -1, "OnUpdate");

		// Push the table as the first argument to the method
		lua_pushvalue(L, -2);

		// Push delta as the second argument to the method
		lua_pushnumber(L, delta);

		// Call the method, pops the method and its arguments from the stack
		LuaChk(lua_pcall(L, 1, 0, 0));

		// Pop the behaviour table from the stack
		lua_pop(L, 1);
	});

	// false -> Do not destroy the system
	return true;
}

bool DrawSpriteSystem::OnUpdate(entt::registry &registry, float delta)
{
	auto spriteView = registry.view<ECS::Sprite, ECS::Transform>();
	std::cout << "Drawing " << spriteView.size_hint() << " sprites..." << std::endl;
	
	spriteView.each([&](const ECS::Sprite &sprite, const ECS::Transform &transform) {
		
		// Draw the sprite at the location defined by the transform.

		const float posX = transform.Position[0];
		const float posY = transform.Position[1];

		const float sclX = transform.Scale[0];
		const float sclY = transform.Scale[1];

		raylib::Color color(*(raylib::Vector4 *)(&(sprite.Color)));

		DrawRectangle((int)posX, (int)posX, (int)posX, (int)posX, color);
	});

	return false;
}
