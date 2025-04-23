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

	int GetEntityCount();

	int CreateEntity();

	bool IsEntity(int entity);

	void RemoveEntity(int entity);

	template<typename...Args>
	bool HasComponents(int entity);

	template<typename T>
	T &GetComponent(int entity);

	template<typename T>
	void SetComponent(int entity, const T&);

	template<typename T, typename...Args>
	void SetComponent(int entity, Args...args);

	template<typename T>
	void RemoveComponent(int entity);

	template<typename T>
	void TryRemoveComponent(int entity);

	template<typename T, typename...Args>
	void CreateSystem(Args...args);

	void RunSystem(std::function<void(entt::registry &registry)> system);

	template<typename...Args>
	void RunSystem(std::function<void(entt::registry &registry, Args...)> system, Args&&... args);

	void InitializeSystems(lua_State *L);

	void UpdateSystems(float delta);

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
	// Returns: your choice :) for transform you can use
	// lua_totransform that you made in module M2
	static int lua_GetComponent(lua_State *L);
	
	// Arguments: entity (int), component type (string)
	// Returns: none
	static int lua_RemoveComponent(lua_State *L);
};


template<typename...Args>
bool Scene::HasComponents(int entity)
{
	return m_registry.all_of<Args...>((entt::entity)entity);
}

template<typename T>
T &Scene::GetComponent(int entity)
{
	return m_registry.get<T>((entt::entity)entity);
}

template<typename T>
void Scene::SetComponent(int entity, const T &component)
{
	m_registry.emplace_or_replace<T>((entt::entity)entity, component);
}

template<typename T, typename...Args>
void Scene::SetComponent(int entity, Args...args)
{
	m_registry.emplace_or_replace<T>((entt::entity)entity, args...);
}

template<typename T>
void Scene::RemoveComponent(int entity)
{
	m_registry.remove<T>((entt::entity)entity);
}

template<typename T>
inline void Scene::TryRemoveComponent(int entity)
{
	if (HasComponents<T>(entity))
		m_registry.remove<T>((entt::entity)entity);
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
