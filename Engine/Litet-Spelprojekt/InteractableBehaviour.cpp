#include "stdafx.h"
#include "InteractableBehaviour.h"
#include "PickupBehaviour.h"
#include "HideBehaviour.h"
#include "InteractorBehaviour.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool InteractableBehaviour::Start()
{
	if (_name == "")
		_name = "InteractableBehaviour"; // For categorization in ImGui.

	SetSerialization(false);

	Entity *playerEnt = GetScene()->GetPlayer();
	if (!playerEnt)
		return true;

	InteractorBehaviour *interactor = nullptr;
	if (!playerEnt->GetBehaviourByType<InteractorBehaviour>(_interactor))
		return true;

	return true;
}

bool InteractableBehaviour::Update(Time &time, const Input &input)
{
	if (_interactor)
		_interactionRange = _interactor->GetInteractionRange();
	return true;
}

bool InteractableBehaviour::Serialize(std::string *code) const
{
	*code += _name + "( )";
	return true;
}

bool InteractableBehaviour::Deserialize(const std::string &code)
{
	return true;
}

void InteractableBehaviour::AddInteractionCallback(std::function<void(void)> callback)
{
	_interactionCallbacks.push_back(callback);
}

void InteractableBehaviour::OnInteraction()
{
	for (auto &callback : _interactionCallbacks)
		callback();
}

bool InteractableBehaviour::OnHover()
{
	if (GetEntity()->IsRemoved())
		return true;

	MeshBehaviour *mb = nullptr;
	if (!GetEntity()->GetBehaviourByType<MeshBehaviour>(mb))
	{
		ErrMsg("Failed getting mesh behaviour!");
		return false;
	}
	Material m = Material(*mb->GetMaterial());
	if (!_isHovered)
	{
		_defaultAmbientId = m.ambientID; // Save default id to reset back to
		_isHovered = true;
	}
	UINT id = GetScene()->GetContent()->GetTextureID("HighlightColor"); // Should change
	m.ambientID = id;
	if (!mb->SetMaterial(&m))
	{
		ErrMsg("Failed setting material");
		return false;
	}

	return true;
}

bool InteractableBehaviour::OffHover()
{
	MeshBehaviour *mb = nullptr;
	if (!GetEntity()->GetBehaviourByType<MeshBehaviour>(mb))
	{
		ErrMsg("Failed getting mesh behaviour!");
		return false;
	}
	Material m = Material(*mb->GetMaterial());
	m.ambientID = _defaultAmbientId;
	if (!mb->SetMaterial(&m))
	{
		ErrMsg("Failed setting material");
		return false;
	}
	_isHovered = false;

	return true;
}

