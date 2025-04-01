#include "stdafx.h"
#include "Entity.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

Entity::Entity(UINT id, const BoundingOrientedBox &bounds)
{
	_entityID = id;
	_bounds = bounds;
}

Entity::~Entity()
{
	_isRemoved = true;
	_behaviours.clear();

	for (auto& child : _children)
	{
		if (child != nullptr)
			child->SetParent(nullptr);
	}
	_children.clear();

	if (_parent)
		_parent->RemoveChild(this);
}

bool Entity::Initialize(ID3D11Device *device, Scene *scene, const std::string &name)
{
	if (_isInitialized)
	{
		ErrMsg("Entity is already initialized!");
		return false;
	}

	SetScene(scene);
	SetName(name);

	if (!_transform.Initialize(device))
	{
		ErrMsg("Failed to initialize entity transform!");
		return false;
	}

	_transform.AddDirtyCallback(std::bind(&Entity::SetDirtyImmediate, this));

	_isInitialized = true;
	return true;
}
bool Entity::IsInitialized() const
{
	return _isInitialized;
}

void Entity::SetSerialization(bool state)
{
	_doSerialize = state;
}
bool Entity::IsSerializable() const
{
	return _doSerialize;
}

void Entity::AddBehaviour(Behaviour *behaviour)
{
	if (!behaviour)
	{
		ErrMsg("Behaviour must not be null!");
		return;
	}

	if (behaviour->IsInitialized())
	{
		ErrMsg("Behaviour must not be initialized before being added to an entity!");
		return;
	}

	_behaviours.push_back(std::unique_ptr<Behaviour>(behaviour));
}

Behaviour *Entity::GetBehaviour(UINT index)
{
	if (index >= _behaviours.size())
		return nullptr;
	return _behaviours.at(index).get();
}

std::vector<std::unique_ptr<Behaviour>> *Entity::GetBehaviours()
{
	return &_behaviours;
}

UINT Entity::GetBehaviourCount()
{
	return static_cast<UINT>(_behaviours.size());
}

bool Entity::IsEnabled() const
{
	return _inheritedDisables <= 0 && _isEnabled;
}
bool Entity::IsEnabledSelf() const
{
	return _isEnabled;
}

void Entity::Enable()
{
	if (IsEnabledSelf())
		return;

	_isEnabled = true;

	if (_inheritedDisables <= 0)
	{
		for (auto &behaviour : _behaviours)
			behaviour->InheritEnabled(true);
	}

	for (auto &child : _children)
		child->DecrementDisable();
}
void Entity::Disable()
{
	if (!IsEnabledSelf())
		return;

	_isEnabled = false;

	if (_inheritedDisables <= 0)
	{
		for (auto &behaviour : _behaviours)
			behaviour->InheritEnabled(false);
	}

	for (auto &child : _children)
		child->IncrementDisable();
}

void Entity::IncrementDisable()
{
	if (_inheritedDisables++ <= 0 && _isEnabled)
	{
		for (auto &behaviour : _behaviours)
			behaviour->InheritEnabled(false);
	}

	for (auto &child : _children)
		child->IncrementDisable();
}
void Entity::DecrementDisable()
{
	if (_inheritedDisables <= 0)
		return;

	_inheritedDisables--;

	if (IsEnabled())
	{
		for (auto &behaviour : _behaviours)
			behaviour->InheritEnabled(true);
	}

	for (auto &child : _children)
		child->DecrementDisable();
}
void Entity::SetInheritedDisables(UINT count)
{
	bool prevIsEnabled = _inheritedDisables <= 0;
	bool newIsEnabled = count <= 0;

	_inheritedDisables = count;

	if (_isEnabled && prevIsEnabled != newIsEnabled)
	{
		for (auto &behaviour : _behaviours)
			behaviour->InheritEnabled(newIsEnabled);
	}

	for (auto &child : _children)
		child->SetInheritedDisables(count);
}

void Entity::SetDirty()
{
	for (auto &behaviour : _behaviours)
		behaviour.get()->SetDirty();

	_recalculateBounds = true;
	_recalculateCollider = true;

	for (auto &child : _children)
		child->SetDirty();
}
void Entity::SetDirtyImmediate()
{
	_recalculateBounds = true;
	_recalculateCollider = true;
	for (auto &behaviour : _behaviours)
		behaviour.get()->SetDirty();
}

void Entity::MarkAsRemoved()
{
	_isRemoved = true;

	for (auto &child : _children)
		child->MarkAsRemoved();
}
bool Entity::IsRemoved() const
{
	return _isRemoved;
}

inline void Entity::AddChild(Entity *child, bool keepWorldTransform)
{
	if (!child)
		return;

	if (!_children.empty())
	{
		auto it = std::find(_children.begin(), _children.end(), child);
		if (it != _children.end())
			return;
	}

	_children.push_back(child);

	child->SetParent(this, keepWorldTransform);
	child->_transform.SetParent(&_transform, keepWorldTransform);
}
inline void Entity::RemoveChild(Entity *child, bool keepWorldTransform)
{
	if (!child)
		return;

	if (_children.empty())
		return;

	auto it = std::find(_children.begin(), _children.end(), child);
	if (it != _children.end())
		_children.erase(it);

	child->_transform.SetParent(nullptr, keepWorldTransform);
}

void Entity::SetParent(Entity *newParent, bool keepWorldTransform)
{
	if (_parent == newParent)
		return;

	if (newParent == this)
	{
		ErrMsg("Cannot parent an entity to itself!");
		return;
	}

	// Check if new parent is a child of this entity
	if (newParent)
	{
		Entity *parentInHierarchy = newParent->GetParent();
		while (parentInHierarchy)
		{
			if (parentInHierarchy == this)
			{
				ErrMsg("Cannot parent an entity to it's child! (Did you mean to unparent the child first?)");
				return;
			}

			parentInHierarchy = parentInHierarchy->GetParent();
		}
	}

	if (_parent)
	{
		_parent->RemoveChild(this, keepWorldTransform);
	}

	_parent = newParent;

	if (newParent)
	{
		newParent->AddChild(this, keepWorldTransform);
		SetInheritedDisables(newParent->_inheritedDisables + (newParent->_isEnabled ? 0 : 1));
	}
	else
	{
		_transform.SetParent(nullptr, keepWorldTransform);
		SetInheritedDisables(0);
	}

	SetDirty();
}
Entity *Entity::GetParent()
{
	return _parent;
}
const std::vector<Entity *> *Entity::GetChildren()
{
	return &_children;
}
void Entity::GetChildrenRecursive(std::vector<Entity *> &children) const
{
	children.insert(children.end(), _children.begin(), _children.end());

	for (auto &child : _children)
		child->GetChildrenRecursive(children);
}

bool Entity::IsChildOf(const Entity *ent) const
{
	if (!ent)
		return false;

	Entity *parentIter = _parent;
	while (parentIter)
	{
		if (parentIter == ent)
			return true;

		parentIter = parentIter->GetParent();
	}

	return false;
}

void Entity::SetScene(Scene *scene)
{
	_scene = scene;
}
Scene *Entity::GetScene() const
{
	return _scene;
}

UINT Entity::GetID() const
{
	return _entityID;
}
void Entity::SetName(const std::string &name)
{
	_name.assign(name);
}
const std::string Entity::GetName() const
{
	return _name;
}
Transform *Entity::GetTransform()
{
	return &_transform;
}

UINT Entity::GetDeserializedID() const
{
	return _deserializedID;
}

void Entity::SetDeserializedID(UINT id)
{
	_deserializedID = id;
}

bool Entity::HasBounds(bool includeTriggers, BoundingOrientedBox &out)
{
	for (auto &behaviour : _behaviours)
	{
		MeshBehaviour *meshBehaviour = dynamic_cast<MeshBehaviour *>(behaviour.get());

		if (meshBehaviour)
		{
			meshBehaviour->StoreBounds(out);
			return true;
		}
	}

	return false;
}

void Entity::SetEntityBounds(DirectX::BoundingOrientedBox &bounds)
{
	_bounds = bounds;
	_recalculateBounds = false;
	SetDirtyImmediate();
}
void Entity::StoreEntityBounds(DirectX::BoundingOrientedBox &bounds)
{
	if (_recalculateBounds)
	{
		XMFLOAT4X4A worldMatrix = GetTransform()->GetWorldMatrix();
		_bounds.Transform(_transformedBounds, Load(&worldMatrix));
		_recalculateBounds = false;
	}

	bounds = _transformedBounds;
}

bool Entity::InitialUpdate(Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(174681845, std::format("Entity Update '{}'", GetName()).c_str());
#endif

	if (!_isEnabled)
		return true;

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialUpdate(time, input))
		{
			ErrMsg("Failed to update behaviour!");
			return false;
		}
	}

	return true;
}
bool Entity::InitialParallelUpdate(const Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(754681845, std::format("Entity Parallel Update '{}'", GetName()).c_str());
#endif

	if (!_isEnabled)
		return true;

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialParallelUpdate(time, input))
		{
#pragma omp critical
			{
				ErrMsg("Failed to update behaviour in parallel!");
			}
			return false;
		}
	}

	return true;
}
bool Entity::InitialLateUpdate(Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(768184426, std::format("Entity Late Update '{}'", GetName()).c_str());
#endif
	if (!_isEnabled)
		return true;

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialLateUpdate(time, input))
		{
			ErrMsg("Failed to late update behaviour!");
			return false;
		}
	}

	return true;
}
bool Entity::InitialFixedUpdate(const float &deltaTime, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(488164865, std::format("Entity Fixed Update '{}'", GetName()).c_str());
#endif

	if (!_isEnabled)
		return true;

	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialFixedUpdate(deltaTime, input))
		{
			ErrMsg("Failed to update behaviour at fixed step!");
			return false;
		}
	}

	return true;
}

bool Entity::InitialBeforeRender()
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(614646645, std::format("Entity Before Render '{}'", GetName()).c_str());
#endif

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialBeforeRender())
		{
			ErrMsg("Failed to run BeforeRender on behaviour!");
			return false;
		}
	}

	if (!_transform.UpdateConstantBuffer(GetScene()->GetContext()))
	{
		ErrMsg("Failed to update entity constant buffers!");
		return false;
	}
	return true;
}
bool Entity::InitialRender(const RenderQueuer &queuer, const RendererInfo &rendererInfo)
{
	if (!_isEnabled)
		return true;

	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialRender(queuer, rendererInfo))
		{
			ErrMsg("Failed to render behaviour!");
			return false;
		}
	}

	return true;
}
#ifdef USE_IMGUI
bool Entity::InitialRenderUI()
{
	ImGui::PushID("enabled");
	bool entEnabled = IsEnabled();
	if (ImGui::Checkbox("Active", &entEnabled))
	{
		if (entEnabled)
			Enable();
		else
			Disable();
	}
	ImGui::PopID();

	ImGui::PushID("entSerialize");
	ImGui::Checkbox("Serialize", &_doSerialize);
	ImGui::PopID();

	for (int i = 0; i < _behaviours.size(); i++)
	{
		auto &behaviour = _behaviours.at(i);
		ImGui::PushID(("Behaviour " + std::to_string(i)).c_str());
		if (ImGui::TreeNode(behaviour.get()->GetName().c_str()))
		{
			ImGui::BeginChild("Behaviour", ImVec2(0, 0), true);
			if (!behaviour.get()->InitialRenderUI())
			{
				ErrMsg("Failed to render behaviour UI!");
				ImGui::EndChild();
				ImGui::TreePop();
				ImGui::PopID();
				return false;
			}
			ImGui::EndChild();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::Dummy({ 0.0f, 6.0f });

	if (ImGui::TreeNode("Add Collider"))
	{	
		ImGuiChildFlags childFlags = 0;
		childFlags |= ImGuiChildFlags_Border;
			//childFlags |= ImGuiChildFlags_ResizeY;

		ImGui::BeginChild("Scene Hierarchy", ImVec2(0, 70), childFlags);

		const int nColliders = 5;
		const char *colliderNames[nColliders] = { "Ray Collider", "Sphere Collider", "Capsule Collider",
									   "AABB Collider", "OBB Collider" };

		// TODO: Fix Ray Collider
		static int selectedIndex = 0;
		const char* preview = colliderNames[selectedIndex];


		if (ImGui::Button("Add Selected Collider"))
		{
			Collisions::ColliderTypes type = (Collisions::ColliderTypes)selectedIndex;

			ColliderBehaviour *cb = new ColliderBehaviour();
			std::vector<ColliderBehaviour *> cbs;
			bool found = GetBehavioursByType<ColliderBehaviour>(cbs);

			if (!cb->Initialize(this))
			{
				ErrMsg("Failed to initialize new collider behaviour!");
				return false;
			}

			switch (type)
			{
			case Collisions::RAY_COLLIDER:
				cb->SetCollider(new Collisions::Ray(_bounds.Center, { 0, 0, 1 }, 1, Collisions::MAP_COLLIDER_TAGS));
				return true;
			case Collisions::SPHERE_COLLIDER:
				cb->SetCollider(new Collisions::Sphere(_bounds.Center, std::min<float>(std::min<float>(_bounds.Extents.x, _bounds.Extents.z), _bounds.Extents.y), Collisions::MAP_COLLIDER_TAGS));
				break;
			case Collisions::CAPSULE_COLLIDER:
				cb->SetCollider(new Collisions::Capsule(_bounds.Center, { 0, 1, 0 }, std::min<float>(_bounds.Extents.x, _bounds.Extents.z), _bounds.Extents.y, Collisions::MAP_COLLIDER_TAGS));
				break;
			case Collisions::AABB_COLLIDER:
				cb->SetCollider(new Collisions::AABB(_bounds, Collisions::MAP_COLLIDER_TAGS));
				break;
			case Collisions::OBB_COLLIDER:
				cb->SetCollider(new Collisions::OBB(_bounds, Collisions::MAP_COLLIDER_TAGS));
				break;
			case Collisions::TERRAIN_COLLIDER:
				break;
			case Collisions::NULL_COLLIDER:
			default:
				ErrMsg("Incorrect collider type!");
				return false;
			}

			GetScene()->GetSceneHolder()->SetRecalculateColliders();
		}

		if (ImGui::BeginCombo("Colliders", preview))
		{
			for (int i = 0; i < nColliders; i++)
			{
				const bool isSelected = (selectedIndex == i);
				if (ImGui::Selectable(colliderNames[i], isSelected))
					selectedIndex = i;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Dummy(ImVec2( 0, 8 ));

		ImGui::EndChild();
		ImGui::TreePop();
	}


	return true;
}
#endif
bool Entity::InitialBindBuffers()
{
	//if (!_transform.UpdateConstantBuffer(GetScene()->GetContext()))
	//{
	//	ErrMsg("Failed to update entity constant buffers!");
	//	return false;
	//}

	ID3D11Buffer *const wmBuffer = _transform.GetConstantBuffer();
	GetScene()->GetContext()->VSSetConstantBuffers(0, 1, &wmBuffer);

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialBindBuffers())
		{
			ErrMsg("Failed to bind behaviour buffers!");
			return false;
		}
	}

	return true;
}
bool Entity::InitialOnHover()
{
	if (!_isEnabled)
		return true;

	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialOnHover())
		{
			ErrMsg(std::format("InitialOnHover() failed for behaviour '{}'!", behaviour->GetName()));
			return false;
		}
	}

	return true;
}
bool Entity::InitialOffHover()
{
	if (!_isEnabled)
		return true;

	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialOffHover())
		{
			ErrMsg(std::format("InitialOffHover() failed for behaviour '{}'!", behaviour->GetName()));
			return false;
		}
	}

	return true;
}
bool Entity::InitialOnSelect()
{
	if (!_isEnabled)
		return true;

	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	for (auto &behaviour : _behaviours)
	{
		if (!behaviour.get()->InitialOnSelect())
		{
			ErrMsg(std::format("InitialOnSelect() failed for behaviour '{}'!", behaviour->GetName()));
			return false;
		}
	}

	return true;
}
