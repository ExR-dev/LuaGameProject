#include "stdafx.h"
#include "Scene.h"
#include "BreadcrumbPileBehaviour.h"
#include "InventoryBehaviour.h"
#include "FlashlightBehaviour.h"
#include "Game.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

bool BreadcrumbPileBehaviour::Start()
{
	if (_name == "")
		_name = "BreadcrumbPileBehaviour"; // For categorization in ImGui.

	Entity *ent = GetEntity();
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
	interactableBehaviour->AddInteractionCallback(std::bind(&BreadcrumbPileBehaviour::Pickup, this));

	// Create a rock mesh behaviour
	Content *content = GetScene()->GetContent();
	UINT meshID = content->GetMeshID("Mesh_Breadcrumb_Collection");
	BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();

	Material mat;
	mat.textureID = content->GetTextureID("Tex_Breadcrumb_Collection");
	mat.normalID = content->GetTextureMapID("TexMap_Breadcrumb_Collection_Normal");
	mat.specularID = content->GetTextureMapID("TexMap_Breadcrumb_Collection_Specular");
	mat.glossinessID = content->GetTextureMapID("TexMap_Breadcrumb_Collection_Glossiness");
	mat.occlusionID = content->GetTextureMapID("TexMap_Breadcrumb_Collection_Occlusion");
	mat.lightID = content->GetTextureID("Tex_Breadcrumb_Collection_Light");

	MeshBehaviour *meshBehaviour = new MeshBehaviour(bounds, meshID, &mat);
	if (!meshBehaviour->Initialize(GetEntity()))
	{
		ErrMsg("Failed to create mesh!");
		return false;
	}
	meshBehaviour->SetSerialization(false);

	// Create a billboard mesh behaviour as a child entity
	mat = {};
	mat.textureID = content->GetTextureID("Tex_Flare");
	mat.ambientID = content->GetTextureID("Tex_White");

	float normalOffset = 0.25f;
	if (GetScene()->GetSaveFile() == "MenuSave") // If menu scene is active
		normalOffset *= 10.0f;

	Entity *entity;
	if (!GetScene()->CreateBillboardMeshEntity(&entity, "Flare Billboard Mesh", mat, 0.0f, normalOffset, 0.01f))
	{
		ErrMsg("Failed to create billboard mesh entity!");
		return false;
	}
	entity->SetParent(GetEntity());
	entity->SetSerialization(false);

	entity->GetBehaviourByType<BillboardMeshBehaviour>(_flare);
	_flare->GetTransform()->Move({ 0.0f, 0.05f, 0.0f }, Local);

	return true;
}

bool BreadcrumbPileBehaviour::Update(Time &time, const Input &input)
{
	Scene *scene = GetScene();

	XMFLOAT3A breadcrumbPos = GetTransform()->GetPosition(World);

	Transform *lightTransform;
	float lightAngle;
	float sizeMultiplier;

	if (scene->GetGame()->GetActiveScene() == 0) // If menu scene is active
	{
		if (!_flashlightEntity)
			_flashlightEntity = scene->GetSceneHolder()->GetEntityByName("Spotlight Flashlight");

		if (!_flashlightEntity)
			return true;

		SimpleSpotLightBehaviour *simpleSpotlight;
		if (!_flashlightEntity->GetBehaviourByType<SimpleSpotLightBehaviour>(simpleSpotlight))
		{
			ErrMsg("Failed to find spotlight!");
			return true;
		}

		if (!simpleSpotlight->IsEnabled())
		{
			_flare->SetSize(0.0f);
			return true;
		}

		if (!simpleSpotlight->ContainsPoint(breadcrumbPos))
		{
			_flare->SetSize(0.0f);
			return true;
		}

		sizeMultiplier = 3.0f;
		lightTransform = simpleSpotlight->GetTransform();
		lightAngle = simpleSpotlight->GetLightBufferData().angle;
	}
	else
	{
		if (!_flashlightEntity)
			_flashlightEntity = scene->GetSceneHolder()->GetEntityByName("Flashlight");

		if (!_flashlightEntity)
			return true;

		FlashlightBehaviour *flashlight;
		if (!_flashlightEntity->GetBehaviourByType<FlashlightBehaviour>(flashlight))
			return true;

		SpotLightBehaviour *spotlight = flashlight->GetLight();

		if (!spotlight->IsEnabled())
		{
			_flare->SetSize(0.0f);
			return true;
		}

		if (!spotlight->ContainsPoint(breadcrumbPos))
		{
			_flare->SetSize(0.0f);
			return true;
		}

		sizeMultiplier = 1.0f;
		lightTransform = spotlight->GetTransform();
		lightAngle = spotlight->GetShadowCamera()->GetFOV();
	}

	XMVECTOR crumbPos = Load(breadcrumbPos);
	XMVECTOR lightPos = Load(lightTransform->GetPosition(World));
	XMVECTOR lightDir = Load(lightTransform->GetForward(World));
	XMVECTOR crumbDir = XMVector3Normalize(crumbPos - lightPos);

	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(lightDir, crumbDir));

	float flareStrength = 1.0f - std::clamp(angle / lightAngle, 0.0f, 1.0f);

	flareStrength = pow(flareStrength, 3.0f);
	_flare->SetSize(flareStrength * 0.5f * sizeMultiplier);

	return true;
}

bool BreadcrumbPileBehaviour::Serialize(std::string *code) const
{
	*code += _name + "( )";
	return true;
}

bool BreadcrumbPileBehaviour::Deserialize(const std::string &code)
{
	return true;
}

void BreadcrumbPileBehaviour::Pickup()
{
	SceneHolder *sceneHolder = GetScene()->GetSceneHolder();

	if (Entity *playerEnt = sceneHolder->GetEntityByName("Player Entity"))
	{
		InventoryBehaviour *inventory;
		if (playerEnt->GetBehaviourByType<InventoryBehaviour>(inventory))
		{
			inventory->AddBreadcrumbs(10);
		}
	}

	if (!sceneHolder->RemoveEntity(GetEntity()))
	{
		ErrMsg("Failed to remove entity from scene holder!");
		return;
	}
}
