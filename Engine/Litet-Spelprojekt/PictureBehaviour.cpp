#include "stdafx.h"
#include "PictureBehaviour.h"
#include "PickupBehaviour.h"
#include "HideBehaviour.h"
#include "Scene.h"
#include "Entity.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool PictureBehaviour::Start()
{
	if (_name == "")
		_name = "PictureBehaviour"; // For categorization in ImGui.

	_ent = GetEntity();
	_t = _ent->GetTransform();
	PickupBehaviour *pickupBehaviour;
	if (_ent->GetBehaviourByType<PickupBehaviour>(pickupBehaviour))
	{
		ErrMsg("Entity cannot have picture and pickup behaviour!");
		return false;
	}

	HideBehaviour *hideBehaviour;
	if (_ent->GetBehaviourByType<HideBehaviour>(hideBehaviour))
	{
		ErrMsg("Entity cannot have picture and hide behaviour!");
		return false;
	}

	InteractableBehaviour *interactableBehaviour;
	if (!_ent->GetBehaviourByType<InteractableBehaviour>(interactableBehaviour))
	{
		interactableBehaviour = new InteractableBehaviour();
		if (!interactableBehaviour->Initialize(_ent))
		{
			ErrMsg("Failed to initialize interactable behaviour!");
			return false;
		}
	}
	interactableBehaviour->AddInteractionCallback(std::bind(&PictureBehaviour::Pickup, this));

	SoundBehaviour *sound = new SoundBehaviour("CollectPicture1");
	if (!sound->Initialize(GetEntity()))
	{
		ErrMsg("Failed to initialize sound to picture!");
		return false;
	}
	_sound = sound;
	_sound->SetVolume(5.0f);
	_sound->SetSerialization(false);

	if (!_isGeneric)
	{
		MeshBehaviour *mesh = nullptr;
		GetEntity()->GetBehaviourByType<MeshBehaviour>(mesh);
		mesh->SetOverlay(true);
		_ent->Disable();
	}

	return true;
}

void PictureBehaviour::OnEnable()
{
	_ent->SetParent(GetScene()->GetSceneHolder()->GetEntityByName("playerCamera"));
	_t->SetPosition(_offset);
}

void PictureBehaviour::OnDisable()
{

}

PictureBehaviour::PictureBehaviour(bool isGeneric) : _isGeneric(isGeneric)
{
}

PictureBehaviour::PictureBehaviour(DirectX::XMFLOAT3A offset)
{
	_offset = offset;
}

void PictureBehaviour::Pickup()
{
	_ent->SetParent(GetScene()->GetSceneHolder()->GetEntityByName("playerCamera"));
	_t->SetPosition(_offset);
	_sound->Play();
}

bool PictureBehaviour::Serialize(std::string *code) const
{
	// Standard code for Serialize
	*code += _name + "(";

	*code += std::to_string(_offset.x) + " ";
	*code += std::to_string(_offset.y) + " ";
	*code += std::to_string(_offset.z) + " ";
	*code += (_isGeneric ? "1 " : "0 ");

	*code += ")";
	return true;
}

bool PictureBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<float> attributes;
	std::istringstream stream(code);

	std::string value;
	while (stream >> value) // Automatically handles spaces correctly
	{
		float attribute = std::stof(value);
		attributes.push_back(attribute);
	}

	_offset.x = attributes.at(0);
	_offset.y = attributes.at(1);
	_offset.z = attributes.at(2);
	_isGeneric = attributes.at(3) > 0.5f;

	return true;
}
