#pragma once

#include "Entity.h"
#include "Raycast.h"
#include "DebugNew.h"

#define QUADTREE_CULLING
//#define OCTREE_CULLING

#ifdef QUADTREE_CULLING
#include "Quadtree.h"
#elif defined OCTREE_CULLING
#include "Octree.h"
#endif

namespace SceneContents
{
	struct SceneEntity;
}

class SceneHolder
{
private:

	UINT _entityCounter = 0;

	DirectX::BoundingBox _bounds;
	std::vector<SceneContents::SceneEntity *> _entities; 
	bool _recalculateColliders = false;

#ifdef QUADTREE_CULLING
	Quadtree _volumeTree;
#elif defined OCTREE_CULLING
	Octree _volumeTree;
#endif

	std::vector<UINT> _treeInsertionQueue;
	std::vector<Entity *> _entityRemovalQueue;

	[[nodiscard]] bool RemoveEntityImmediate(Entity *entity);

public:
	enum BoundsType {
		Frustum		= 0,
		OrientedBox = 1
	};

	SceneHolder() = default;
	~SceneHolder();
	SceneHolder(const SceneHolder &other) = delete;
	SceneHolder &operator=(const SceneHolder &other) = delete;
	SceneHolder(SceneHolder &&other) = delete;
	SceneHolder &operator=(SceneHolder &&other) = delete;

	[[nodiscard]] bool Initialize(const DirectX::BoundingBox &sceneBounds);
	[[nodiscard]] bool Update();

	// Entity is Not initialized automatically. Initialize manually through the returned pointer.
	[[nodiscard]] Entity *AddEntity(const DirectX::BoundingOrientedBox &bounds, bool addToTree);

	[[nodiscard]] bool RemoveEntity(Entity *entity);
	[[nodiscard]] bool RemoveEntity(UINT index);

	[[nodiscard]] bool IncludeEntityInTree(Entity *entity);
	[[nodiscard]] bool IncludeEntityInTree(UINT index);
	[[nodiscard]] bool ExcludeEntityFromTree(Entity *entity);
	[[nodiscard]] bool ExcludeEntityFromTree(UINT index);

	[[nodiscard]] bool IsEntityIncludedInTree(const Entity *entity) const;
	[[nodiscard]] bool IsEntityIncludedInTree(UINT index) const;

	[[nodiscard]] bool UpdateEntityPosition(Entity *entity);

	[[nodiscard]] const DirectX::BoundingBox &GetBounds() const;

	[[nodiscard]] Entity *GetEntity(UINT i) const;
	[[nodiscard]] Entity *GetEntityByID(UINT id) const;
	[[nodiscard]] Entity *GetEntityByName(const std::string &name) const;
	[[nodiscard]] Entity *GetEntityByDeserializedID(UINT id) const;
	void GetEntities(std::vector<Entity *> &entities) const;

	[[nodiscard]] UINT GetEntityIndex(const Entity *entity) const;
	[[nodiscard]] UINT GetEntityIndex(UINT id) const;
	[[nodiscard]] UINT GetEntityCount() const;

	[[nodiscard]] bool FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const;
	[[nodiscard]] bool BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems) const;
	[[nodiscard]] bool BoxCull(const DirectX::BoundingBox &box, std::vector<Entity *> &containingItems) const;

	bool RaycastScene(const DirectX::XMFLOAT3A &origin, const DirectX::XMFLOAT3A &direction, RaycastOut &result) const;

	void DebugGetTreeStructure(std::vector<DirectX::BoundingBox> &boxCollection) const;

	bool GetRecalculateColliders() const;
	void SetRecalculateColliders();

	void ResetSceneHolder();
};
