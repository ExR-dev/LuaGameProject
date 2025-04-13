#include "stdafx.h"
#include "Scene.h"

#pragma region General
Scene::Scene()
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
#pragma endregion
