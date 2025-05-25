#include "stdafx.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

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

	int id = static_cast<int>(m_registry.create());
	return id;
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

bool Scene::IsActive(entt::entity entity)
{
	if (m_registry.all_of<ECS::Active>(entity))
	{
		ECS::Active &active = m_registry.get<ECS::Active>(entity);
		return active.IsActive;
	}
	return true;
}
bool Scene::IsActive(int entity)
{
	return IsActive(static_cast<entt::entity>(entity));
}

void Scene::SetActive(entt::entity entity, bool state)
{
	ZoneScopedC(RandomUniqueColor());

	// Check if entity has an active component.
	if (HasComponents<ECS::Active>(entity))
	{
		// If it does, set the active state
		ECS::Active &active = GetComponent<ECS::Active>(entity);

		// Check if the active component is already in the desired state
		if (active.IsActive == state)
			return; // No need to change the state

		active.IsActive = state;
		SetComponent<ECS::Active>(entity, active);
	}
	else
	{
		// If not, create one and set the state
		ECS::Active active{};
		active.IsActive = state;
		SetComponent<ECS::Active>(entity, active);
	}
}
void Scene::SetActive(int entity, bool state)
{
	SetActive(static_cast<entt::entity>(entity), state);
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

void Scene::Clear(lua_State *L)
{
	std::function<void(entt::registry &registry)> clear = [&](entt::registry &registry) {
		ZoneNamedNC(createPhysicsBodiesZone, "Lambda Remove All Entities", RandomUniqueColor(), true);

		auto view = registry.view<entt::entity>(entt::exclude<ECS::Debug>);

		view.each([&](entt::entity entity) {
			ZoneNamedNC(drawSpriteZone, "Lambda Remove All Entities", RandomUniqueColor(), true);
			SetComponent<ECS::Remove>(entity);
		});

		CleanUp(L);
	};

	RunSystem(clear);
}

void Scene::CleanUp(lua_State* L)
{	
	std::function<void(entt::registry& registry)> cleanup = [&](entt::registry& registry) {
		ZoneNamedNC(createPhysicsBodiesZone, "Lambda Remove Entities", RandomUniqueColor(), true);

		auto view = registry.view<ECS::Remove>();
		std::vector<entt::entity> entitiesToDestroy;

		view.each([&](entt::entity entity, ECS::Remove& remove) {
			ZoneNamedNC(drawSpriteZone, "Lambda Remove Entities", RandomUniqueColor(), true);
			entitiesToDestroy.push_back(entity);
		});

		// Destroy entities after iteration
		for (int i = entitiesToDestroy.size() - 1; i >= 0; i--) 
		{
			entt::entity ent = entitiesToDestroy[i];

			if (HasComponents<ECS::Collider>(ent))
				GetComponent<ECS::Collider>(ent).Destroy(L);

			if (HasComponents<ECS::Behaviour>(ent))
				GetComponent<ECS::Behaviour>(ent).Destroy(L);

			RemoveEntity(ent);
		}
	};

	RunSystem(cleanup);
}


void Scene::lua_openscene(lua_State *L, Scene *scene)
{
	ZoneScopedC(RandomUniqueColor());

	lua_newtable(L);

	luaL_Reg methods[] = {
	//  { "NameInLua",			NameInCpp			},
		{ "SetScene",			lua_SetScene	},
		{ "CreateEntity",		lua_CreateEntity	},
		{ "SetComponent",		lua_SetComponent	},
		{ "GetEntityCount",		lua_GetEntityCount	},
		{ "GetEntities",		lua_GetEntities		},
		{ "IsEntity",			lua_IsEntity		},
		{ "RemoveEntity",		lua_RemoveEntity	},
		{ "HasComponent",		lua_HasComponent	},
		{ "GetComponent",		lua_GetComponent	},
		{ "RemoveComponent",	lua_RemoveComponent	},
		{ "IsActive",			lua_IsActive		},
		{ "SetActive",			lua_SetActive		},
		{ "Clear",				lua_Clear			},
		{ NULL,					NULL				}
	};

	lua_pushlightuserdata(L, scene);
	luaL_setfuncs(L, methods, 1); // 1 : one upvalue (lightuserdata)

	lua_createtable(L, 0, 7);
		lua_pushnumber(L, Game::SceneState::None);
		lua_setfield(L, -2, "None");

		lua_pushnumber(L, Game::SceneState::InMenu);
		lua_setfield(L, -2, "InMenu");

		lua_pushnumber(L, Game::SceneState::InGame);
		lua_setfield(L, -2, "InGame");

		lua_pushnumber(L, Game::SceneState::InEditor);
		lua_setfield(L, -2, "InEditor");

		lua_pushnumber(L, Game::SceneState::ReloadGame);
		lua_setfield(L, -2, "ReloadGame");

		lua_pushnumber(L, Game::SceneState::ReloadEditor);
		lua_setfield(L, -2, "ReloadEditor");

		lua_pushnumber(L, Game::SceneState::Quitting);
		lua_setfield(L, -2, "Quitting");
	lua_setfield(L, -2, "SceneState");

	lua_setglobal(L, "scene");

#ifdef LUA_DEBUG
	LuaDoFileCleaned(L, LuaFilePath("PrintTable"));
#endif
}

void Scene::ResetSceneState()
{
	m_sceneState = Game::SceneState::None;
}

Game::SceneState Scene::GetSceneState() const
{
	return m_sceneState;
}

int Scene::lua_SetScene(lua_State* L)
{
	Scene* scene = lua_GetScene(L);
	int sceneState = lua_tointeger(L, 1);

	if (sceneState < Game::SceneState::Count)
		scene->m_sceneState = (Game::SceneState)sceneState;

	return 0;
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
		//scene->TryRemoveComponent<ECS::Transform>(entity);

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
	else if (type == "Active")
	{
		if (scene->HasComponents<ECS::Active>(entity))
			scene->RemoveComponent<ECS::Active>(entity);
		
		ECS::Active active{};
		active.LuaPull(L, 3);
		
		scene->SetComponent<ECS::Active>(entity, active);
		return 1;
	}
	else if (type == "Collider")
	{	
		bool hasCollider = scene->HasComponents<ECS::Collider>(entity);
		b2BodyId id;
		if (hasCollider)
		{
			id = scene->GetComponent<ECS::Collider>(entity).bodyId;
			scene->RemoveComponent<ECS::Collider>(entity);
		}

		ECS::Collider collider {};
		collider.createBody = !hasCollider;
		if (hasCollider)
			collider.bodyId = id;
		collider.LuaPull(L, 3);

		scene->SetComponent<ECS::Collider>(entity, collider);
		return 1;
	}
	else if (type == "Hardness")
	{
		if (scene->HasComponents<ECS::Hardness>(entity))
			scene->RemoveComponent<ECS::Hardness>(entity);

		ECS::Hardness hardness{};
		hardness.LuaPull(L, 3);

		scene->SetComponent<ECS::Hardness>(entity, hardness);
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
	else if (type == "TextRender") 
	{
		if (scene->HasComponents<ECS::TextRender>(entity))
			scene->RemoveComponent<ECS::TextRender>(entity);

		ECS::TextRender textRender{};
		textRender.LuaPull(L, 3);

		scene->SetComponent<ECS::TextRender>(entity, textRender);

		return 1;
	}
	else if (type == "UIElement") 
	{
		if (scene->HasComponents<ECS::UIElement>(entity))
			scene->RemoveComponent<ECS::UIElement>(entity);

		ECS::UIElement uiElement{};
		uiElement.LuaPull(L, 3);

		scene->SetComponent<ECS::UIElement>(entity, uiElement);

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
	else if (type == "Debug")
	{
		if (scene->HasComponents<ECS::Debug>(entity))
			scene->RemoveComponent<ECS::Debug>(entity);

		scene->SetComponent<ECS::Debug>(entity);
		return 1;
	}

	return 0;
}

int Scene::lua_GetEntityCount(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int count = scene->GetEntityCount();
	lua_pushinteger(L, count);
	return 1;
}

int Scene::lua_GetEntities(lua_State* L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene* scene = lua_GetScene(L);
	
	lua_createtable(L, 0, scene->GetEntityCount());

	std::function<void(entt::registry& registry)> pushEntities = [&](entt::registry& registry) {
		ZoneNamedNC(createPhysicsBodiesZone, "Lambda Push All Entities", RandomUniqueColor(), true);

		unsigned int index = 1;
		auto view = registry.view<entt::entity>();

		view.each([&](entt::entity entity) {
			ZoneNamedNC(drawSpriteZone, "Lambda Push Entity", RandomUniqueColor(), true);
			lua_pushinteger(L, index++);
			lua_pushinteger(L, static_cast<int>(entity));
			lua_settable(L, -3);
		});
	};

	scene->RunSystem(pushEntities);

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
	//scene->RemoveEntity(entity);
	if (scene->IsEntity(entity))
		scene->SetComponent<ECS::Remove>(entity);
	return 0;
}

int Scene::lua_HasComponent(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	
	bool hasComponent = true;
		
	if		(type == "Active")
	{
		hasComponent = scene->HasComponents<ECS::Active>(entity);
	}
	else if (type == "Behaviour")
	{
		hasComponent = scene->HasComponents<ECS::Behaviour>(entity);
	}
	else if (type == "Collider")
	{
		hasComponent = scene->HasComponents<ECS::Collider>(entity);
	}
	else if (type == "Health")
	{
		hasComponent = scene->HasComponents<ECS::Health>(entity);
	}
	else if (type == "Hardness")
	{
		hasComponent = scene->HasComponents<ECS::Hardness>(entity);
	}
	else if (type == "Sprite")
	{
		hasComponent = scene->HasComponents<ECS::Sprite>(entity);
	}
	else if (type == "TextRender")
	{
		hasComponent = scene->HasComponents<ECS::TextRender>(entity);
	}
	else if (type == "UIElement")
	{
		hasComponent = scene->HasComponents<ECS::UIElement>(entity);
	}
	else if (type == "CameraData")
	{
		hasComponent = scene->HasComponents<ECS::CameraData>(entity);
	}
	else if (type == "Transform")
	{
		hasComponent = scene->HasComponents<ECS::Transform>(entity);
	}
	else if (type == "Debug")
	{
		hasComponent = scene->HasComponents<ECS::Debug>(entity);
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

	
	if	(type == "Active" && scene->HasComponents<ECS::Active>(entity))
	{
		ECS::Active &active = scene->GetComponent<ECS::Active>(entity);
		active.LuaPush(L);
		return 1;
	}
	else if (type == "Transform" && scene->HasComponents<ECS::Transform>(entity))
	{
		ECS::Transform &transform = scene->GetComponent<ECS::Transform>(entity);
		transform.LuaPush(L);
		return 1;
	}
	else if (type == "Collider" && scene->HasComponents<ECS::Collider>(entity))
	{
		ECS::Collider &collider = scene->GetComponent<ECS::Collider>(entity);
		collider.LuaPush(L);
		return 1;
	}
	else if (type == "Behaviour" && scene->HasComponents<ECS::Behaviour>(entity))
	{
		ECS::Behaviour &behaviour = scene->GetComponent<ECS::Behaviour>(entity);
		behaviour.LuaPush(L);
		return 1;
	}
	else if (type == "Hardness" && scene->HasComponents<ECS::Hardness>(entity))
	{
		ECS::Hardness &hardness = scene->GetComponent<ECS::Hardness>(entity);
		hardness.LuaPush(L);
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
	else if (type == "TextRender" && scene->HasComponents<ECS::TextRender>(entity))
	{
		ECS::TextRender &textRender = scene->GetComponent<ECS::TextRender>(entity);
		textRender.LuaPush(L);
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

	if		(type == "Active" && scene->HasComponents<ECS::Active>(entity))
	{
		scene->RemoveComponent<ECS::Active>(entity);
	}
	else if (type == "Behaviour" && scene->HasComponents<ECS::Behaviour>(entity))
	{
		ECS::Behaviour behaviour = scene->GetComponent<ECS::Behaviour>(entity);
		behaviour.Destroy(L);

		scene->RemoveComponent<ECS::Behaviour>(entity);
	}
	else if (type == "Transform" && scene->HasComponents<ECS::Transform>(entity))
	{
		scene->RemoveComponent<ECS::Transform>(entity);
	}
	else if (type == "Collider" && scene->HasComponents<ECS::Collider>(entity))
	{
		// TODO: Find a better solution
		ECS::Collider collider = scene->GetComponent<ECS::Collider>(entity);
		collider.Destroy(L);

		scene->RemoveComponent<ECS::Collider>(entity);
	}
	else if (type == "Health" && scene->HasComponents<ECS::Health>(entity))
	{
		scene->RemoveComponent<ECS::Health>(entity);
	}
	else if (type == "Sprite" && scene->HasComponents<ECS::Sprite>(entity))
	{
		scene->RemoveComponent<ECS::Sprite>(entity);
	}
	else if (type == "TextRender" && scene->HasComponents<ECS::TextRender>(entity))
	{
		scene->RemoveComponent<ECS::TextRender>(entity);
	}
	else if (type == "CameraData" && scene->HasComponents<ECS::CameraData>(entity))
	{
		scene->RemoveComponent<ECS::CameraData>(entity);
	}
	else if (type == "Debug" && scene->HasComponents<ECS::Debug>(entity))
	{
		scene->RemoveComponent<ECS::Debug>(entity);
	}
	// else if...

	return 1;
}

int Scene::lua_IsActive(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = lua_tointeger(L, 1);

	lua_pushboolean(L, scene->IsActive(entity) ? 1 : 0);
	return 1;
}

int Scene::lua_SetActive(lua_State *L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene *scene = lua_GetScene(L);
	int entity = lua_tointeger(L, 1);
	bool state = lua_toboolean(L, 2);

	scene->SetActive(entity, state);
	return 1;
}

int Scene::lua_Clear(lua_State* L)
{
	ZoneScopedC(RandomUniqueColor());

	Scene* scene = lua_GetScene(L);
	scene->Clear(L);

	return 0;
}
