#pragma once
#include "dep/EnTT/entt.hpp"
#include "Components/Components.h"
#include "Systems/System.h"
#include <functional>

class Scene
{
public:
	Scene() = default;
	~Scene();

	entt::registry &GetRegistry();

	int GetEntityCount();

	int CreateEntity();

	bool IsEntity(entt::entity entity);
	bool IsEntity(int entity);

	void RemoveEntity(entt::entity entity);
	void RemoveEntity(int entity);

	bool IsActive(entt::entity entity);
	bool IsActive(int entity);

	void SetActive(entt::entity entity, bool state);
	void SetActive(int entity, bool state);

	template<typename...Args>
	bool HasComponents(entt::entity entity);
	template<typename...Args>
	bool HasComponents(int entity);

	template<typename T>
	T &GetComponent(entt::entity entity);
	template<typename T>
	T &GetComponent(int entity);

	template<typename T>
	void SetComponent(entt::entity entity, const T&);
	template<typename T>
	void SetComponent(int entity, const T&);

	template<typename T, typename...Args>
	void SetComponent(entt::entity entity, Args...args);
	template<typename T, typename...Args>
	void SetComponent(int entity, Args...args);

	template<typename T>
	void RemoveComponent(entt::entity entity);
	template<typename T>
	void RemoveComponent(int entity);

	template<typename T>
	void TryRemoveComponent(entt::entity entity);
	template<typename T>
	void TryRemoveComponent(int entity);

	template<typename T, typename...Args>
	void CreateSystem(Args...args);

	void RunSystem(std::function<void(entt::registry &registry)> system);

	template<typename...Args>
	void RunSystem(std::function<void(entt::registry &registry, Args...)> system, Args&&... args);

	void SystemsInitialize(lua_State *L);

	void SystemsOnUpdate(float delta);
	void SystemsOnRender(float delta);

	void CleanUp();

	static void lua_openscene(lua_State *L, Scene *scene);


private:
	entt::registry m_registry;
	std::vector<System *> m_systems;

	// Aguments: none
	// Returns: entity (int)
	static int lua_CreateEntity(lua_State *L);

	static int lua_SetComponent(lua_State *L);

	// Aguments: none
	// Returns: number of entities in the scene (int)
	static int lua_GetEntityCount(lua_State *L);

	// Arguments: entity (int)
	// Returns: (bool)
	static int lua_IsEntity(lua_State *L);
	
	// Arguments: entity (int)
	// Returns: none
	static int lua_RemoveEntity(lua_State *L);
	
	// Arguments: entity (int), component type (string)
	// Returns: (bool)
	static int lua_HasComponent(lua_State *L);
	
	// Arguments: entity (int), component type (string)
	// Returns: your choice :)
	static int lua_GetComponent(lua_State *L);
	
	// Arguments: entity (int), component type (string)
	// Returns: none
	static int lua_RemoveComponent(lua_State *L);

	// Arguments: entity (int)
	// Returns: (bool)
	static int lua_IsActive(lua_State *L);
	
	// Arguments: entity (int), state (bool)
	// Returns: none
	static int lua_SetActive(lua_State *L);
};


template<typename...Args>
bool Scene::HasComponents(entt::entity entity)
{
	return m_registry.all_of<Args...>(entity);
}
template<typename...Args>
bool Scene::HasComponents(int entity)
{
	return HasComponents<Args...>((entt::entity)entity);
}

template<typename T>
T &Scene::GetComponent(entt::entity entity)
{
	return m_registry.get<T>(entity);
}
template<typename T>
T &Scene::GetComponent(int entity)
{
	return GetComponent<T>((entt::entity)entity);
}

template<typename T>
void Scene::SetComponent(entt::entity entity, const T &component)
{
	m_registry.emplace_or_replace<T>(entity, component);
}
template<typename T>
void Scene::SetComponent(int entity, const T &component)
{
	SetComponent<T>((entt::entity)entity, component);
}

template<typename T, typename...Args>
void Scene::SetComponent(entt::entity entity, Args...args)
{
	m_registry.emplace_or_replace<T>(entity, args...);
}
template<typename T, typename...Args>
void Scene::SetComponent(int entity, Args...args)
{
	SetComponent<T>((entt::entity)entity, args...);
}

template<typename T>
void Scene::RemoveComponent(entt::entity entity)
{
	m_registry.remove<T>(entity);
}
template<typename T>
void Scene::RemoveComponent(int entity)
{
	RemoveComponent<T>((entt::entity)entity);
}

template<typename T>
inline void Scene::TryRemoveComponent(entt::entity entity)
{
	if (HasComponents<T>(entity))
		RemoveComponent<T>(entity);
}
template<typename T>
inline void Scene::TryRemoveComponent(int entity)
{
	TryRemoveComponent<T>((entt::entity)entity);
}

template<typename T, typename...Args>
void Scene::CreateSystem(Args...args)
{
	m_systems.emplace_back(new T(args...));
}

inline void Scene::RunSystem(std::function<void(entt::registry &registry)> system)
{
	system(m_registry);
}

template<typename...Args>
inline void Scene::RunSystem(std::function<void(entt::registry &registry, Args...)> system, Args&&... args)
{
	system(m_registry, std::forward<Args>(args)...);
}
