#include "stdafx.h"
#include "Scene.h"

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

int Scene::GetEntityCount()
{
	return m_registry.view<entt::entity>().size();
}

int Scene::CreateEntity()
{
	return static_cast<int>(m_registry.create());
}

bool Scene::IsEntity(int entity)
{
	return m_registry.valid(static_cast<entt::entity>(entity));
}

void Scene::RemoveEntity(int entity)
{
	m_registry.destroy(static_cast<entt::entity>(entity));
}

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

void Scene::lua_openscene(lua_State *L, Scene *scene)
{
	lua_newtable(L);

	luaL_Reg methods[] = {
	//  { "MethodNameInLua",	FunctionNameInCpp	},
		{ "CreateEntity",		lua_CreateEntity	},
		{ "SetComponent",		lua_SetComponent	},
		{ "GetEntityCount",		lua_GetEntityCount	},
		{ "IsEntity",			lua_IsEntity		},
		{ "RemoveEntity",		lua_RemoveEntity	},
		{ "HasComponent",		lua_HasComponent	},
		{ "GetComponent",		lua_GetComponent	},
		{ "RemoveComponent",	lua_RemoveComponent	},
		{ NULL,					NULL				}
	};

	lua_pushlightuserdata(L, scene);
	luaL_setfuncs(L, methods, 1); // 1 : one upvalue (lightuserdata)

	lua_setglobal(L, "scene");
}

int Scene::lua_CreateEntity(lua_State *L)
{
	Scene *scene = (Scene *)lua_topointer(L, lua_upvalueindex(1));

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
