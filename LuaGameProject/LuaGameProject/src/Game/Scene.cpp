#include "stdafx.h"
#include "Scene.h"

#define lua_GetSceneUpValue(L) (Scene *)lua_topointer(L, lua_upvalueindex(1))

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
			delete (*it);
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
	Scene *scene = lua_GetSceneUpValue(L);
	int entity = scene->CreateEntity();
	lua_pushinteger(L, entity);
	return 1;
}

int Scene::lua_SetComponent(lua_State *L)
{
	Scene *scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	
	if (type == "Health") 
	{
		// TODO
	}
	else if (type == "Transform") 
	{
		// TODO
	}
	else if (type == "Sprite") 
	{
		// TODO
	}
	else if (type == "Behaviour")
	{
		if (scene->HasComponents<ECS::Behaviour>(entity))
			scene->RemoveComponent<ECS::Behaviour>(entity);
		
		const char *path = lua_tostring(L, 3);
		
		// Returns the behaviour table on top of the stack
		luaL_dofile(L, path);
		
		// luaL_ref pops the value of the stack, so we push the table again before luaL_ref
		lua_pushvalue(L, -1);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		
		// Populate the behaviour table with the information the behaviour should know about
		lua_pushinteger(L, entity);
		lua_setfield(L, -2, "ID");
		
		lua_pushstring(L, path);
		lua_setfield(L, -2, "path");
		
		// Let the behaviour construct itself. It may be good
		// practice to check if the method exists before calling it
		lua_getfield(L, -1, "OnCreate");
		lua_pushvalue(L, -2); // Push the table as argument
		lua_pcall(L, 1, 0, 0);
		
		scene->SetComponent<ECS::Behaviour>(entity, path, ref);
		return 1;
	}
}

int Scene::lua_GetEntityCount(lua_State *L)
{
	Scene *scene = lua_GetSceneUpValue(L);
	int count = scene->GetEntityCount();
	lua_pushinteger(L, count);
	return 1;
}

int Scene::lua_IsEntity(lua_State *L)
{
	Scene *scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	bool alive = scene->IsEntity(entity);
	lua_pushboolean(L, alive);
	return 1;
}

int Scene::lua_RemoveEntity(lua_State *L)
{
	Scene *scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	scene->RemoveEntity(entity);
	return 0;
}

int Scene::lua_HasComponent(lua_State *L)
{
	Scene *scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	
	bool hasComponent = true;
	
	if (type == "Health")
	{
		hasComponent = scene->HasComponents<ECS::Health>(entity);
	}
	else if (type == "Transform")
	{
		hasComponent = scene->HasComponents<ECS::Transform>(entity);
	}
	else if (type == "Sprite")
	{
		hasComponent = scene->HasComponents<ECS::Sprite>(entity);
	}
	// else if...
		
	lua_pushboolean(L, hasComponent);
	return 1;
}

int Scene::lua_GetComponent(lua_State *L)
{
	Scene *scene = lua_GetSceneUpValue(L);
	
	if (!lua_isinteger(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}
	
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	
	// Sanity check that the entity exist
	if (!scene->IsEntity(entity))
	{
		lua_pushnil(L);
		return 1;
	}

	if (type == "Health" && scene->HasComponents<ECS::Health>(entity))
	{
		ECS::Health &health = scene->GetComponent<ECS::Health>(entity);
		lua_pushnumber(L, health.Value); // Maybe push a "component" table?
		return 1;
	}
	else if (type == "Transform" && scene->HasComponents<ECS::Transform>(entity))
	{
		ECS::Transform &transform = scene->GetComponent<ECS::Transform>(entity);
		// TODO: lua_pushtransform(L, transform);
		return 1;
	}
	else if (type == "Sprite" && scene->HasComponents<ECS::Sprite>(entity))
	{
		ECS::Sprite &sprite = scene->GetComponent<ECS::Sprite>(entity);
		// TODO: lua_pushsprite(L, sprite);
		return 1;
	}
	// else if...
	
	// Name or component not found
	lua_pushnil(L);
	return 1;
}

int Scene::lua_RemoveComponent(lua_State *L)
{
	Scene *scene = lua_GetSceneUpValue(L);

	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	if (type == "Health" && scene->HasComponents<ECS::Health>(entity))
	{
		scene->RemoveComponent<ECS::Health>(entity);
	}
	else if (type == "Transform" && scene->HasComponents<ECS::Transform>(entity))
	{
		scene->RemoveComponent<ECS::Transform>(entity);
	}
	else if (type == "Sprite" && scene->HasComponents<ECS::Sprite>(entity))
	{
		scene->RemoveComponent<ECS::Sprite>(entity);
	}
	// else if...

	return 0;
}
