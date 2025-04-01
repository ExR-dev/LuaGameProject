#include "stdafx.h"
#include "PickupBehaviour.h"
#include "HideBehaviour.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool PickupBehaviour::Start()
{
	if (_name == "")
		_name = "PickupBehaviour"; // For categorization in ImGui.

	Entity *ent = GetEntity();
	HideBehaviour *hideBehaviour;
	if (ent->GetBehaviourByType<HideBehaviour>(hideBehaviour))
	{
		ErrMsg("Entity cannot have pickup and hide behaviour!");
		return false;
	}

	InteractableBehaviour *interactableBehaviour;
	if (!ent->GetBehaviourByType<InteractableBehaviour>(interactableBehaviour))
	{
		interactableBehaviour = new InteractableBehaviour();
		if (!interactableBehaviour->Initialize(ent))
		{
			ErrMsg("Failed to initialize interactable behaviour!");
			return false;
		}
	}
	interactableBehaviour->AddInteractionCallback(std::bind(&PickupBehaviour::Pickup, this));

	return true;
}

bool PickupBehaviour::Update(Time &time, const Input &input)
{
	if (_isHolding)
	{
		// { 0.7f, -0.4f, 0.8f }

		GetEntity()->SetParent(GetScene()->GetSceneHolder()->GetEntityByName("playerCamera"));
		_objectTransform->SetPosition(_offset);
	}
	else if (_objectTransform != nullptr)
	{
		GetEntity()->SetParent(nullptr);
		DirectX::XMFLOAT3A playerPos = _playerTransform->GetPosition(World);
		_objectTransform->Move(DirectX::XMFLOAT3A(playerPos.x - _offset.x, 1.0f - _offset.y, playerPos.z - _offset.z), World); // TODO: Change y to ground height

		DirectX::XMFLOAT4A playerRot = _playerTransform->GetRotation(World);
		playerRot.x = 0.0f;
		playerRot.z = 0.0f;
		Store(playerRot, DirectX::XMVector4Normalize(Load(playerRot)));
		_objectTransform->SetRotation(playerRot, World);
		_objectTransform = nullptr;
	}

	return true;
}

bool PickupBehaviour::Serialize(std::string *code) const
{	
	*code += _name + "( )";
		//+ std::to_string(valfri variabel) +
		//" )";
	return true;
}

bool PickupBehaviour::Deserialize(const std::string &code)
{
	return true;
}

void PickupBehaviour::Pickup()
{
	_isHolding = !_isHolding;

	_objectTransform = GetEntity()->GetTransform();
	_playerTransform = GetScene()->GetPlayerCamera()->GetTransform();
}
