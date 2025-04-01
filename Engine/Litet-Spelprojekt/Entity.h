#pragma once

#include <type_traits>
#include "Transform.h"
#include "Time.h"
#include "Input.h"
#include "Graphics.h"
#include "Behaviour.h"
#include "RenderQueuer.h"
#include "Colliders.h"

class Scene;

class Entity
{
private:
	UINT _entityID = -1;
	UINT _deserializedID = -1;
	std::string _name = "";
	bool _isRemoved = false;

	DirectX::BoundingOrientedBox _bounds = {{0,0,0},{1,1,1},{0,0,0,1}};
	DirectX::BoundingOrientedBox _transformedBounds = { {0,0,0},{1,1,1},{0,0,0,1} };
	bool _recalculateBounds = true;

	bool _doSerialize = true;
	UINT _inheritedDisables = 0;

	void IncrementDisable();
	void DecrementDisable();
	void SetInheritedDisables(UINT count);

protected:
	bool _isInitialized = false;
	bool _isEnabled = true;
	Transform _transform;

	bool _recalculateCollider = true;

	Scene *_scene = nullptr;

	Entity *_parent = nullptr;
	std::vector<Entity *> _children;
	std::vector<std::unique_ptr<Behaviour>> _behaviours;

	inline void AddChild(Entity *child, bool keepWorldTransform = false);
	inline void RemoveChild(Entity *child, bool keepWorldTransform = false);

public:
	Entity(UINT id, const DirectX::BoundingOrientedBox &bounds);
	virtual ~Entity();
	Entity(const Entity &other) = delete;
	Entity &operator=(const Entity &other) = delete;
	Entity(Entity &&other) = delete;
	Entity &operator=(Entity &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, Scene *scene, const std::string &name);
	[[nodiscard]] bool IsInitialized() const;

	void SetSerialization(bool state);
	[[nodiscard]] bool IsSerializable() const;

	[[nodiscard]] bool IsEnabled() const;
	[[nodiscard]] bool IsEnabledSelf() const;

	void AddBehaviour(Behaviour *behaviour);
	[[nodiscard]] Behaviour *GetBehaviour(UINT index);
	[[nodiscard]] std::vector<std::unique_ptr<Behaviour>> *GetBehaviours();
	[[nodiscard]] UINT GetBehaviourCount();
	template <class T>
	bool GetBehaviourByType(T *&behaviour) const;
	template <class T>
	bool GetBehaviourByType(Behaviour *&behaviour) const;
	template <class T>
	bool GetBehavioursByType(std::vector<T*> &behaviours) const;
	template <class T>
	bool GetBehavioursByType(std::vector<Behaviour> &behaviours) const;

	void Enable();
	void Disable();

	void SetDirty();
	void SetDirtyImmediate();

	void MarkAsRemoved();
	[[nodiscard]] bool IsRemoved() const;

	void SetParent(Entity *parent, bool keepWorldTransform = false);
	[[nodiscard]] Entity *GetParent();
	[[nodiscard]] const std::vector<Entity *> *GetChildren();
	void GetChildrenRecursive(std::vector<Entity *> &children) const;
	[[nodiscard]] bool IsChildOf(const Entity *ent) const;

	void SetScene(Scene *scene);
	// If this is called from a class with a circular dependency to Scene, you'll have to explicity include Scene.h from the caller.
	[[nodiscard]] Scene *GetScene() const;

	[[nodiscard]] Transform *GetTransform();

	void SetName(const std::string &name);
	[[nodiscard]] const std::string GetName() const;

	[[nodiscard]] UINT GetID() const;

	[[nodiscard]] UINT GetDeserializedID() const;
	[[nodiscard]] void SetDeserializedID(UINT id);

	[[nodiscard]] bool HasBounds(bool includeTriggers, DirectX::BoundingOrientedBox &out);
	void SetEntityBounds(DirectX::BoundingOrientedBox &bounds);
	void StoreEntityBounds(DirectX::BoundingOrientedBox &bounds);

	[[nodiscard]] bool InitialUpdate(Time &time, const Input &input);
	[[nodiscard]] bool InitialParallelUpdate(const Time &time, const Input &input);
	[[nodiscard]] bool InitialLateUpdate(Time &time, const Input &input);
	[[nodiscard]] bool InitialFixedUpdate(const float &deltaTime, const Input &input);
	[[nodiscard]] bool InitialBeforeRender();
	[[nodiscard]] bool InitialRender(const RenderQueuer &queuer, const RendererInfo &rendererInfo);
#ifdef USE_IMGUI
	[[nodiscard]] bool InitialRenderUI();
#endif
	[[nodiscard]] bool InitialBindBuffers();
	[[nodiscard]] bool InitialOnHover();
	[[nodiscard]] bool InitialOffHover();
	[[nodiscard]] bool InitialOnSelect();

};

template<class T>
inline bool Entity::GetBehaviourByType(T *&behaviour) const
{
	if (_isRemoved)
		return false;

	if (!std::is_base_of<Behaviour, T>::value)
		return false;

	UINT behaviourCount = static_cast<UINT>(_behaviours.size());
	for (UINT i = 0; i < behaviourCount; i++)
	{
		T *castedBehaviour = dynamic_cast<T*>(_behaviours[i].get());

		if (!castedBehaviour)
			continue;

		behaviour = castedBehaviour;
		return true;
	}

	behaviour = nullptr;
	return false;
}

template<class T>
inline bool Entity::GetBehaviourByType(Behaviour *&behaviour) const
{
	if (_isRemoved)
		return false;

	if (!std::is_base_of<Behaviour, T>::value)
		return false;

	UINT behaviourCount = static_cast<UINT>(_behaviours.size());
	for (UINT i = 0; i < behaviourCount; i++)
	{
		T *castedBehaviour = dynamic_cast<T *>(_behaviours[i].get());

		if (!castedBehaviour)
			continue;

		behaviour = _behaviours[i].get();
		return true;
	}

	behaviour = nullptr;
	return false;
}

template<class T>
inline bool Entity::GetBehavioursByType(std::vector<T*> &behaviours) const
{
	if (_isRemoved)
		return false;

	if (!std::is_base_of<Behaviour, T>::value)
		return false;
	
	bool found = false;

	UINT behaviourCount = static_cast<UINT>(_behaviours.size());
	for (UINT i = 0; i < behaviourCount; i++)
	{
		T *castedBehaviour = dynamic_cast<T*>(_behaviours[i].get());

		if (!castedBehaviour)
			continue;

		behaviours.push_back(castedBehaviour);
		found = true;
	}

	return found;
}

template<class T>
inline bool Entity::GetBehavioursByType(std::vector<Behaviour> &behaviours) const
{
	if (_isRemoved)
		return false;

	if (!std::is_base_of<Behaviour, T>::value)
		return false;
	
	bool found = false;

	UINT behaviourCount = static_cast<UINT>(_behaviours.size());
	for (UINT i = 0; i < behaviourCount; i++)
	{
		T *castedBehaviour = dynamic_cast<T*>(_behaviours[i].get());

		if (!castedBehaviour)
			continue;

		behaviours.push_back(_behaviours[i].get());
		found = true;
	}

	return found;
}
