#include "stdafx.h"
#include "InteractorBehaviour.h"
#include "InteractableBehaviour.h"
#include "PickupBehaviour.h"
#include "HideBehaviour.h"
#include "Scene.h"
#include "PictureBehaviour.h"
#include "InventoryBehaviour.h"
#include <format>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

bool InteractorBehaviour::Start()
{
	if (_name == "")
		_name = "InteractorBehaviour"; // For categorization in ImGui.

	return true;
}

bool InteractorBehaviour::Update(Time &time, const Input &input)
{
	Scene *scene = GetScene();
	SceneHolder *sceneHolder = scene->GetSceneHolder();

	if (_picPieces.size() == 0) // Post start
	{
		for (UINT i = 0; i < _totalPieces; i++)
		{
			std::string name = std::format("PicPiece{}", i + 1);
			_picPieces.push_back(std::move(sceneHolder->GetEntityByName(name)));

			if (!_picPieces.at(i))
				continue;

			_picPieces.at(i)->Disable();

			MeshBehaviour* mesh = nullptr;

			if (_picPieces.at(i)->GetBehaviourByType<MeshBehaviour>(mesh))
			{
				Material meshMat = *mesh->GetMaterial();
				meshMat.vsID = scene->GetContent()->GetShaderID("VS_Geometry");
				mesh->SetMaterial(&meshMat);
			}
		}

		for (UINT i = 0; i < _totalCollected; i++)
		{
			if (!_picPieces.at(i))
				continue;

			_picPieces.at(i)->Enable();
			MeshBehaviour *mesh = nullptr;

			if (_picPieces.at(i)->GetBehaviourByType<MeshBehaviour>(mesh))
			{
				BoundingOrientedBox box = BoundingOrientedBox({ 0,1,0 }, { 0.01f, 0.01f, 0.01f }, { 0,0,0,1 });
				mesh->SetBounds(box);
			}
		}
	}
	
	if (input.GetKey(KeyCode::E) == KeyState::Pressed)
	{
		if (input.IsCursorLocked())
		{
			Transform *transform = scene->GetViewCamera()->GetTransform();

			// Interact with any object
			RaycastOut out;
			InteractableBehaviour *interactableBehaviour = nullptr;
			if (sceneHolder->RaycastScene(transform->GetPosition(World), transform->GetForward(World), out) &&
				out.entity->GetBehaviourByType<InteractableBehaviour>(interactableBehaviour) &&
				out.distance <= _interactionRange)
			{
				PictureBehaviour *picture = nullptr;
				if (!std::count(_picPieces.begin(), _picPieces.end(), out.entity) &&
					out.entity->GetBehaviourByType<PictureBehaviour>(picture))
				{
					InventoryBehaviour* inventory = nullptr;
					GetEntity()->GetBehaviourByType<InventoryBehaviour>(inventory);
					if(inventory)
						inventory->SetHeldItem(false, HandState::PicturePiece);
					else
					{
						ErrMsg("Failed to get inventory behaviour!");
						return false;
					}

					if (!sceneHolder->RemoveEntity(out.entity))
					{
						ErrMsg("Failed to remove entity!");
						return false;
					}
					_totalCollected = _totalCollected > _totalPieces ? _totalPieces : _totalCollected + 1;
					for (UINT i = 0; i < _totalCollected; i++)
					{
						_picPieces.at(i)->Enable();
					}
				}

				interactableBehaviour->OnInteraction();

				PickupBehaviour *pickup = nullptr;
				if (out.entity->GetBehaviourByType<PickupBehaviour>(pickup))
				{
					_isHolding = true;
					_holdingEnt = out.entity;
				}

				HideBehaviour *hide = nullptr;
				if (out.entity->GetBehaviourByType<HideBehaviour>(hide))
				{
					_isHiding = true;
					_hidingEnt = out.entity;
				}
			}

			interactableBehaviour = nullptr;
			const std::vector<Entity*> *children = sceneHolder->GetEntityByName("playerCamera")->GetChildren();
			UINT i = 0;
			for (i; i < children->size(); i++)
			{
				if (children->at(i) == _holdingEnt)
				{
					children->at(i)->GetBehaviourByType<InteractableBehaviour>(interactableBehaviour);
					break;
				}
			}
			if (_isHolding && !_isHiding && children->size() > 1 && interactableBehaviour) // Drop holding object
			{
				interactableBehaviour->OnInteraction();

				if (children->at(i) == _holdingEnt)
				{
					_isHolding = false;
				}

				return true;
			}

			if (_isHiding)
			{
				_isHiding = false;
			}
		}
	}

	return true;
}

float InteractorBehaviour::GetInteractionRange() const
{
	return _interactionRange;
}

bool InteractorBehaviour::Serialize(std::string *code) const
{
	// Standard code for Serialize
	UINT holdingID = _holdingEnt ? _holdingEnt->GetID() : CONTENT_NULL;
	UINT hidingID = _hidingEnt ? _hidingEnt->GetID() : CONTENT_NULL;

	*code += _name + "("
		+ std::to_string(_showingPic) + " " + std::to_string(_totalCollected) + " " + std::to_string(_isHolding)
		+ " " + std::to_string(holdingID) + " " + std::to_string(_isHiding) + " " + std::to_string(hidingID) +
	" )";
	return true;
}

bool InteractorBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<UINT> attributes;
	std::istringstream stream(code);

	std::string value;
	stream >> value;
	_showingPic = std::stoi(value);

	while (stream >> value) // Automatically handles spaces correctly
	{
		UINT attribute = std::stoul(value);
		attributes.push_back(attribute);
	}

	_totalCollected = attributes.at(0);
	_isHolding = attributes.at(1);

	//if (attributes.at(2) != CONTENT_NULL)
	//{
	//	_holdingEnt = _sceneHolder->GetEntityByDeserializedID(attributes.at(2));
	//	InteractableBehaviour * interactableBehaviour = nullptr;
	//	_holdingEnt->GetBehaviourByType<InteractableBehaviour>(interactableBehaviour);
	//	interactableBehaviour->OnInteraction();
	//}
	//_isHiding = attributes.at(3);
	//if (attributes.at(4) != CONTENT_NULL)
	//{
	//	_hidingEnt = _sceneHolder->GetEntityByDeserializedID(attributes.at(4));
	//	InteractableBehaviour *interactableBehaviour = nullptr;
	//	_hidingEnt->GetBehaviourByType<InteractableBehaviour>(interactableBehaviour);
	//	interactableBehaviour->OnInteraction();
	//}

	return true;
}

void InteractorBehaviour::PostDeserialize()
{
	if (_isHolding)
	{
		const std::vector<Entity *> *children = GetScene()->GetSceneHolder()->GetEntityByName("playerCamera")->GetChildren();
		for (Entity *entity : *children)
		{
			std::string name = entity->GetName();
			if (name != "Flashlight" &&
				name != "PicPiece1" &&
				name != "PicPiece2" &&
				name != "PicPiece3" &&
				name != "PicPiece4" &&
				name != "PicPiece5")
			{
				entity->SetParent(nullptr);
				DirectX::XMFLOAT3A _offset = { -0.9f, -0.6f, 0.8f }; // Local offset from player camera
				DirectX::XMFLOAT3A playerPos = GetEntity()->GetTransform()->GetPosition(World);
				entity->GetTransform()->Move(DirectX::XMFLOAT3A(playerPos.x - _offset.x, 1.0f - _offset.y, playerPos.z - _offset.z), World); // TODO: Change y to ground height

				DirectX::XMFLOAT4A playerRot = GetEntity()->GetTransform()->GetRotation(World);
				playerRot.x = 0.0f;
				playerRot.z = 0.0f;
				Store(playerRot, DirectX::XMVector4Normalize(Load(playerRot)));
				entity->GetTransform()->SetRotation(playerRot, World);
				break;
			}
		}
	}
}

void InteractorBehaviour::ShowPicture()
{
	for (UINT i = 0; i < _totalCollected; i++)
	{
		_picPieces.at(i)->Enable();
	}
}

void InteractorBehaviour::HidePicture()
{
	for (UINT i = 0; i < _totalCollected; i++)
	{
		_picPieces.at(i)->Disable();
	}
}
