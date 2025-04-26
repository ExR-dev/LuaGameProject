#include "stdafx.h"
#include "Scene.h"

#define lua_GetSceneUpValue(L) (Scene *)lua_topointer(L, lua_upvalueindex(1))
#define lua_GetScene(L) lua_GetSceneUpValue(L)

Scene::~Scene()
{
	ZoneScopedC(RandomUniqueColor());

	for (int i = 0; i < m_systems.size(); i++)
	{
		delete m_systems[i];
	}
}

entt::registry &Scene::GetRegistry()
{
	return m_registry;
}

int Scene::GetEntityCount()
{
	return m_registry.view<entt::entity>().size();
}

int Scene::CreateEntity()
{
	ZoneScopedC(RandomUniqueColor());

	return static_cast<int>(m_registry.create());
}

bool Scene::IsEntity(entt::entity entity)
{
	return m_registry.valid(entity);
}
bool Scene::IsEntity(int entity)
{
	return IsEntity(static_cast<entt::entity>(entity));
}

void Scene::RemoveEntity(entt::entity entity)
{
	m_registry.destroy(entity);
}
void Scene::RemoveEntity(int entity)
{
	RemoveEntity(static_cast<entt::entity>(entity));
}

void Scene::SystemsInitialize(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	// Must be done for all systems that are managed automatically by the scene
	CreateSystem<BehaviourSystem>(L);
}

void Scene::SystemsOnUpdate(float delta)
{
	ZoneScopedC(RandomUniqueColor());

	if (m_systems.empty())
		return;

	for (auto it = m_systems.begin(); it != m_systems.end(); it++)
	{
		if ((*it)->OnUpdate(m_registry, delta))
		{
			delete (*it);
			it = m_systems.erase(it);
		}
	}
}

void Scene::SystemsOnRender(float delta)
{
	ZoneScopedC(RandomUniqueColor());

	if (m_systems.empty())
		return;

	for (auto it = m_systems.begin(); it != m_systems.end(); it++)
	{
		if ((*it)->OnRender(m_registry, delta))
		{
			delete (*it);
			it = m_systems.erase(it);
		}
	}
}

void Scene::lua_openscene(lua_State *L, Scene *scene)
{
	ZoneScopedC(RandomUniqueColor());

	lua_newtable(L);

	luaL_Reg methods[] = {
	//  { "NameInLua",			NameInCpp			},
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

	tracy::LuaRegister(L);
}

int Scene::lua_CreateEntity(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = scene->CreateEntity();
	lua_pushinteger(L, entity);
	return 1;
}

int Scene::lua_SetComponent(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	
	if (type == "Transform") 
	{
		scene->TryRemoveComponent<ECS::Transform>(entity);

		ECS::Transform transform{};
		transform.LuaPull(L, 3);

		scene->SetComponent<ECS::Transform>(entity, transform);
		return 1;
	}
	else if (type == "Behaviour")
	{
		if (scene->HasComponents<ECS::Behaviour>(entity))
			scene->RemoveComponent<ECS::Behaviour>(entity);
		
		const char *path = lua_tostring(L, 3);
		
		scene->SetComponent<ECS::Behaviour>(entity, path, entity, L);
		return 1;
	}
	else if (type == "Health") 
	{
		scene->TryRemoveComponent<ECS::Health>(entity);

		ECS::Health health{};
		health.LuaPull(L, 3);

		scene->SetComponent<ECS::Health>(entity, health);
		return 1;
	}
	else if (type == "Sprite") 
	{
		if (scene->HasComponents<ECS::Sprite>(entity))
			scene->RemoveComponent<ECS::Sprite>(entity);

		ECS::Sprite sprite{};
		sprite.LuaPull(L, 3);

		scene->SetComponent<ECS::Sprite>(entity, sprite);

		// Update the sprite sort order
		scene->m_registry.sort<ECS::Sprite>(ECS::Sprite::Compare);
		return 1;
	}
	else if (type == "CameraData") 
	{
		if (scene->HasComponents<ECS::CameraData>(entity))
			scene->RemoveComponent<ECS::CameraData>(entity);

		ECS::CameraData cameraData{};
		cameraData.LuaPull(L, 3);

		scene->SetComponent<ECS::CameraData>(entity, cameraData);
		return 1;
	}
}

int Scene::lua_GetEntityCount(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int count = scene->GetEntityCount();
	lua_pushinteger(L, count);
	return 1;
}

int Scene::lua_IsEntity(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = lua_tointeger(L, 1);
	bool alive = scene->IsEntity(entity);
	lua_pushboolean(L, alive);
	return 1;
}

int Scene::lua_RemoveEntity(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = lua_tointeger(L, 1);
	scene->RemoveEntity(entity);
	return 0;
}

int Scene::lua_HasComponent(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	
	bool hasComponent = true;
	
	if (type == "Transform")
	{
		hasComponent = scene->HasComponents<ECS::Transform>(entity);
	}
	else if (type == "Health")
	{
		hasComponent = scene->HasComponents<ECS::Health>(entity);
	}
	else if (type == "Behaviour")
	{
		hasComponent = scene->HasComponents<ECS::Behaviour>(entity);
	}
	else if (type == "Sprite")
	{
		hasComponent = scene->HasComponents<ECS::Sprite>(entity);
	}
	else if (type == "CameraData")
	{
		hasComponent = scene->HasComponents<ECS::CameraData>(entity);
	}
		
	lua_pushboolean(L, hasComponent);
	return 1;
}

int Scene::lua_GetComponent(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	
	if (!lua_isinteger(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}
	
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	
	if (!scene->IsEntity(entity))
	{
		lua_pushnil(L);
		return 1;
	}
	
	if (type == "Transform" && scene->HasComponents<ECS::Transform>(entity))
	{
		ECS::Transform &transform = scene->GetComponent<ECS::Transform>(entity);
		transform.LuaPush(L);
		return 1;
	}
	else if (type == "Behaviour" && scene->HasComponents<ECS::Behaviour>(entity))
	{
		ECS::Behaviour &behaviour = scene->GetComponent<ECS::Behaviour>(entity);
		behaviour.LuaPush(L);
		return 1;
	}
	else if (type == "Health" && scene->HasComponents<ECS::Health>(entity))
	{
		ECS::Health &health = scene->GetComponent<ECS::Health>(entity);
		health.LuaPush(L);
		return 1;
	}
	else if (type == "CameraData" && scene->HasComponents<ECS::CameraData>(entity))
	{
		ECS::CameraData &cameraData = scene->GetComponent<ECS::CameraData>(entity);
		cameraData.LuaPush(L);
		return 1;
	}
	else if (type == "Sprite" && scene->HasComponents<ECS::Sprite>(entity))
	{
		ECS::Sprite &sprite = scene->GetComponent<ECS::Sprite>(entity);
		sprite.LuaPush(L);
		return 1;
	}
	
	// Name or component not found
	lua_pushnil(L);
	return 1;
}

int Scene::lua_RemoveComponent(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);

	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	if (type == "Behaviour" && scene->HasComponents<ECS::Behaviour>(entity))
	{
		scene->RemoveComponent<ECS::Behaviour>(entity);
	}
	else if (type == "Transform" && scene->HasComponents<ECS::Transform>(entity))
	{
		scene->RemoveComponent<ECS::Transform>(entity);
	}
	else if (type == "Health" && scene->HasComponents<ECS::Health>(entity))
	{
		scene->RemoveComponent<ECS::Health>(entity);
	}
	else if (type == "Sprite" && scene->HasComponents<ECS::Sprite>(entity))
	{
		scene->RemoveComponent<ECS::Sprite>(entity);
	}
	else if (type == "CameraData" && scene->HasComponents<ECS::CameraData>(entity))
	{
		scene->RemoveComponent<ECS::CameraData>(entity);
	}
	// else if...

	return 0;
}
