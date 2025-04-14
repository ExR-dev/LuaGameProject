#include "stdafx.h"
#include "Scene.h"

#pragma region General
Scene::Scene(lua_State *L)
{
}

Scene::~Scene()
{
	for (auto it = m_systems.begin(); it != m_systems.end(); it++)
	{
		delete(*it);
		it = m_systems.erase(it);
	}
}
#pragma endregion

#pragma region Entities
int Scene::GetEntityCount()
{
	// TODO: Implement
	return 0;
}

int Scene::CreateEntity()
{
	// TODO: Implement
	return 0;
}

bool Scene::IsEntity(int entity)
{
	// TODO: Implement
	return false;
}

void Scene::RemoveEntity(int entity)
{
	// TODO: Implement
}
#pragma endregion

#pragma region Components
#pragma endregion

#pragma region Systems
void Scene::UpdateSystems(float delta)
{
	for (auto it = m_systems.begin(); it != m_systems.end(); it++)
	{
		if ((*it)->OnUpdate(m_registry, delta))
		{
			delete(*it);
			it = m_systems.erase(it);
		}
	}
}
#pragma endregion

#pragma region Lua
void Scene::lua_openscene(lua_State *L, Scene *scene)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
	//  { "MethodNameInLua",	FunctionNameInCpp	},
		{ "CreateEntity",		lua_CreateEntity	},
		{ "SetComponent",		lua_SetComponent	},
		{ NULL,					NULL				}
	};

	lua_pushlightuserdata(L, scene);
	luaL_setfuncs(L, methods, 1); // 1 : one upvalue (lightuserdata)

	lua_setglobal(L, "scene");
}

int Scene::lua_CreateEntity(lua_State *L)
{
	// TODO: Implement
	return 0;
}
int Scene::lua_SetComponent(lua_State *L)
{
	// TODO: Implement
	return 0;
}
int Scene::lua_GetEntityCount(lua_State *L)
{
	// TODO: Implement
	return 0;
}
int Scene::lua_IsEntity(lua_State *L)
{
	// TODO: Implement
	return 0;
}
int Scene::lua_RemoveEntity(lua_State *L)
{
	// TODO: Implement
	return 0;
}
int Scene::lua_HasComponent(lua_State *L)
{
	// TODO: Implement
	return 0;
}
int Scene::lua_GetComponent(lua_State *L)
{
	// TODO: Implement
	return 0;
}
int Scene::lua_RemoveComponent(lua_State *L)
{
	// TODO: Implement
	return 0;
}
#pragma endregion
