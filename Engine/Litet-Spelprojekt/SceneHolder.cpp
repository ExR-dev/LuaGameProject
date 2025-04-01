#include "stdafx.h"
#include "SceneHolder.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

struct SceneContents::SceneEntity
{
	bool includeInTree = true;
	Entity *entity = nullptr;

	explicit SceneEntity(const UINT id, const DirectX::BoundingOrientedBox &bounds, bool includeInTree)
	{
		this->includeInTree = includeInTree;
		entity = new Entity(id, bounds);
	}

	~SceneEntity()
	{
		delete entity;
	}

	SceneEntity(const SceneEntity &other) = delete;
	SceneEntity &operator=(const SceneEntity &other) = delete;
	SceneEntity(SceneEntity &&other) = delete;
	SceneEntity &operator=(SceneEntity &&other) = delete;

	Entity *GetEntity() const
	{
		return entity;
	}
};


SceneHolder::~SceneHolder()
{
	for (const SceneContents::SceneEntity *ent : _entities)
		delete ent;
}
bool SceneHolder::Initialize(const BoundingBox &sceneBounds)
{
	_bounds = sceneBounds;
	if (!_volumeTree.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize volume tree!");
		return false;
	}

	return true;
}

bool SceneHolder::Update()
{
	for (const UINT id : _treeInsertionQueue)
	{
		Entity *entity = GetEntityByID(id);

		if (entity == nullptr)
		{
			ErrMsg(std::format("Failed to update scene holder, entity ID #{} not found!", id));
			return false;
		}
		
		DirectX::BoundingOrientedBox entityBounds;
		entity->StoreEntityBounds(entityBounds);

		_volumeTree.Insert(entity, entityBounds);
	}
	_treeInsertionQueue.clear();

	for (Entity *entity : _entityRemovalQueue)
	{
		if (entity == nullptr)
			continue;

		if (!RemoveEntityImmediate(entity))
		{
			ErrMsg("Failed to flush removal of entity!");
			return false;
		}
	}
	_entityRemovalQueue.clear();

	_recalculateColliders = false;
	return true;
}

Entity *SceneHolder::AddEntity(const BoundingOrientedBox &bounds, bool addToTree)
{
	if (_entities.size() == 0)
		_entityCounter = 0;

	SceneContents::SceneEntity *newEntity = new SceneContents::SceneEntity(_entityCounter, bounds, addToTree);
	_entities.push_back(newEntity);
	_entityCounter++;
	_recalculateColliders = true;

	if (addToTree)
		_treeInsertionQueue.push_back(newEntity->GetEntity()->GetID());

	return _entities.back()->GetEntity();
}

bool SceneHolder::RemoveEntityImmediate(Entity *entity)
{
	entity->MarkAsRemoved();

	for (auto &child : *entity->GetChildren())
	{
		if (!RemoveEntityImmediate(child))
		{
			ErrMsg("Failed to remove child entity!");
			return false;
		}
	}

	DirectX::BoundingOrientedBox entityBounds;
	entity->StoreEntityBounds(entityBounds);

	if (!_volumeTree.Remove(entity, entityBounds))
	{
		ErrMsg("Failed to remove entity from volume tree!");
		delete entity;
		return false;
	}

	for (int i = 0; i < _entities.size(); i++)
	{
		if ((_entities[i]->GetEntity() == entity) || (_entities[i]->GetEntity()->GetID() == entity->GetID()))
		{
			delete _entities[i];
			_entities.erase(_entities.begin() + i);
			_recalculateColliders = true;
			break;
		}
	}

	return true;
}

bool SceneHolder::RemoveEntity(Entity *entity)
{
	if (entity == nullptr)
		return false;

	for (Entity *queuedEntity : _entityRemovalQueue)
	{
		if (entity == queuedEntity)
			return true;
	}

	_entityRemovalQueue.push_back(entity);
	entity->MarkAsRemoved();
	_recalculateColliders = true;
	return true;
}
bool SceneHolder::RemoveEntity(const UINT index)
{
	if (index >= _entities.size())
	{
		ErrMsg(std::format("Failed to remove entity, ID:{} out of range!", index));
		return false;
	}

	_recalculateColliders = true;
	return RemoveEntity(_entities.at(index)->GetEntity());
}

bool SceneHolder::IncludeEntityInTree(Entity *entity)
{
	if (!entity)
	{
		ErrMsg("Failed to include entity in tree, entity is null!");
		return false;
	}

	return IncludeEntityInTree(GetEntityIndex(entity));
}
bool SceneHolder::IncludeEntityInTree(UINT index)
{
	if (index >= _entities.size())
	{
		ErrMsg(std::format("Failed to include entity in tree, ID:{} out of range!", index));
		return false;
	}

	SceneContents::SceneEntity *sceneEntity = _entities.at(index);

	if (sceneEntity->includeInTree)
		return true; // Already included

	Entity *entity = sceneEntity->GetEntity();

	auto insertionQueueIt = std::find(_treeInsertionQueue.begin(), _treeInsertionQueue.end(), entity->GetID());
	while (insertionQueueIt != _treeInsertionQueue.end())
	{
		_treeInsertionQueue.erase(insertionQueueIt);
		insertionQueueIt = std::find(_treeInsertionQueue.begin(), _treeInsertionQueue.end(), entity->GetID());
	}

	BoundingOrientedBox entityBounds;
	entity->StoreEntityBounds(entityBounds);

	_volumeTree.Insert(entity, entityBounds);
	sceneEntity->includeInTree = true;

	return true;
}

bool SceneHolder::ExcludeEntityFromTree(Entity *entity)
{
	if (!entity)
	{
		ErrMsg("Failed to exclude entity from tree, entity is null!");
		return false;
	}

	return ExcludeEntityFromTree(GetEntityIndex(entity));
}
bool SceneHolder::ExcludeEntityFromTree(UINT index)
{
	if (index >= _entities.size())
	{
		ErrMsg(std::format("Failed to exclude entity from tree, ID:{} out of range!", index));
		return false;
	}

	SceneContents::SceneEntity *sceneEntity = _entities.at(index);

	if (!sceneEntity->includeInTree)
		return true; // Already excluded

	Entity *entity = sceneEntity->GetEntity();

	auto insertionQueueIt = std::find(_treeInsertionQueue.begin(), _treeInsertionQueue.end(), entity->GetID());
	while (insertionQueueIt != _treeInsertionQueue.end())
	{
		_treeInsertionQueue.erase(insertionQueueIt);
		insertionQueueIt = std::find(_treeInsertionQueue.begin(), _treeInsertionQueue.end(), entity->GetID());
	}

	BoundingOrientedBox entityBounds;
	entity->StoreEntityBounds(entityBounds);

	if (!_volumeTree.Remove(entity, entityBounds))
	{
		ErrMsg("Failed to remove entity from volume tree!");
		return false;
	}

	sceneEntity->includeInTree = false;

	return true;
}

bool SceneHolder::IsEntityIncludedInTree(const Entity *entity) const
{
	if (!entity)
		return false;

	return IsEntityIncludedInTree(GetEntityIndex(entity));
}
bool SceneHolder::IsEntityIncludedInTree(UINT index) const
{
	if (index >= _entities.size())
		return false;

	SceneContents::SceneEntity *sceneEntity = _entities.at(index);
	return sceneEntity->includeInTree;
}

bool SceneHolder::UpdateEntityPosition(Entity *entity)
{
	// Check if entity is already in tree insertion queue
	if (std::find(_treeInsertionQueue.begin(), _treeInsertionQueue.end(), entity->GetID()) != _treeInsertionQueue.end())
	{
		return true;
	}

	for (auto &child : *entity->GetChildren())
	{
		if (!UpdateEntityPosition(child))
		{
			ErrMsg("Failed to update child entity position!");
			return false;
		}
	}

	UINT entityIndex = GetEntityIndex(entity);

	if (entityIndex >= 0)
	{
		if (_entities.at(entityIndex)->includeInTree)
		{
			DirectX::BoundingOrientedBox entityBounds;
			entity->StoreEntityBounds(entityBounds);

			if (!_volumeTree.Remove(entity))
			{
				ErrMsg("Failed to remove entity from volume tree!");
				return false;
			}

			_volumeTree.Insert(entity, entityBounds);
		}
	}

	entity->GetTransform()->CleanScenePos();
	return true;
}

const DirectX::BoundingBox& SceneHolder::GetBounds() const
{
	return _bounds;
}

Entity *SceneHolder::GetEntity(const UINT i) const
{
	if (i < 0)
		return nullptr;

	if (i >= _entities.size())
		return nullptr;

	return _entities[i]->GetEntity();
}
Entity *SceneHolder::GetEntityByID(const UINT id) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		Entity *ent = _entities[i]->GetEntity();

		if (!ent)
			continue;

		if (ent->IsRemoved())
			continue;

		if (ent->GetID() == id)
			return ent;
	}

	return nullptr;
}
Entity *SceneHolder::GetEntityByName(const std::string &name) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		Entity *ent = _entities[i]->GetEntity();

		if (!ent)
			continue;

		if (ent->IsRemoved())
			continue;

		if (ent->GetName() == name)
			return ent;
	}

	return nullptr;
}
Entity *SceneHolder::GetEntityByDeserializedID(UINT id) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		Entity *ent = _entities[i]->GetEntity();

		if (!ent)
			continue;

		if (ent->IsRemoved())
			continue;

		if (ent->GetDeserializedID() == id)
			return ent;
	}

	return nullptr;
}
void SceneHolder::GetEntities(std::vector<Entity *> &entities) const
{
	entities.reserve(_entities.size());
	for (const SceneContents::SceneEntity *ent : _entities)
	{
		if (!ent->GetEntity())
			continue;

		if (ent->GetEntity()->IsRemoved())
			continue;

		entities.push_back(ent->GetEntity());
	}
}

UINT SceneHolder::GetEntityIndex(const Entity *entity) const
{
	if (!entity)
		return -1;

	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		if (entity == _entities[i]->GetEntity())
			return i;
	}

	return -1;
}
UINT SceneHolder::GetEntityIndex(UINT id) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		if (id == _entities[i]->GetEntity()->GetID())
			return i;
	}

	return -1;
}
UINT SceneHolder::GetEntityCount() const
{
	return static_cast<UINT>(_entities.size());
}

bool SceneHolder::FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const
{
	std::vector<Entity *> containingInterfaces;
	containingInterfaces.reserve(_entities.capacity());

	if (!_volumeTree.FrustumCull(frustum, containingInterfaces))
	{
		ErrMsg("Failed to frustum cull volume tree!");
		return false;
	}

	for (Entity *iEnt : containingInterfaces)
		containingItems.push_back(iEnt);

	return true;
}
bool SceneHolder::BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems) const
{
	std::vector<Entity *> containingInterfaces;
	containingInterfaces.reserve(_entities.capacity());

	if (!_volumeTree.BoxCull(box, containingInterfaces))
	{
		ErrMsg("Failed to box cull volume tree!");
		return false;
	}

	for (Entity *iEnt : containingInterfaces)
		containingItems.push_back(iEnt);

	return true;
}
bool SceneHolder::BoxCull(const DirectX::BoundingBox &box, std::vector<Entity *> &containingItems) const
{
	std::vector<Entity *> containingInterfaces;
	containingInterfaces.reserve(_entities.capacity());

	if (!_volumeTree.BoxCull(box, containingInterfaces))
	{
		ErrMsg("Failed to box cull volume tree!");
		return false;
	}

	for (Entity *iEnt : containingInterfaces)
		containingItems.push_back(iEnt);

	return true;
}
bool SceneHolder::RaycastScene(const DirectX::XMFLOAT3A &origin, const DirectX::XMFLOAT3A &direction, RaycastOut &result) const
{
	return _volumeTree.RaycastTree(origin, direction, result.distance, result.entity);
}

void SceneHolder::DebugGetTreeStructure(std::vector<DirectX::BoundingBox> &boxCollection) const
{
	_volumeTree.DebugGetStructure(boxCollection);
}

void SceneHolder::SetRecalculateColliders()
{
	_recalculateColliders = true;
}

void SceneHolder::ResetSceneHolder()
{
	for (const SceneContents::SceneEntity *ent : _entities)
		delete ent;

	_entityCounter = 0;

	_bounds = {};
	_entities = {};
	_recalculateColliders = false;

#ifdef QUADTREE_CULLING
	_volumeTree = {};
#elif defined OCTREE_CULLING
	Octree _volumeTree;
#endif

	_treeInsertionQueue = {};
	_entityRemovalQueue = {};
}

bool SceneHolder::GetRecalculateColliders() const
{
	return _recalculateColliders;
}