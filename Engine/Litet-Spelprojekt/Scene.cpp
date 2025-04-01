#pragma region Includes, Usings & Defines
#include "stdafx.h"
#include "Scene.h"
#include "Game.h"
#ifdef DEBUG_BUILD
#include "ExampleCollisionBehaviour.h"
#include "UIButtonExampleBehaviour.h"
#include "TransformGizmoBehaviour.h"
#endif
#include "PlayerMovementBehaviour.h"
#include "BreadcrumbPileBehaviour.h"
#include "RestrictedViewBehaviour.h"
#include "BillboardMeshBehaviour.h"
#include "AmbientSoundBehaviour.h"
#include "MonsterHintBehaviour.h"
#ifdef DEBUG_BUILD
#include "DebugPlayerBehaviour.h"
#endif
#include "SolidObjectBehaviour.h"
#include "EndCutSceneBehaviour.h"
#include "FlashlightBehaviour.h"
#include "CameraItemBehaviour.h"
#include "InteractorBehaviour.h"
#include "PlayerViewBehaviour.h"
#include "MenuCameraBehaviour.h"
#include "BreadcrumbBehaviour.h"
#include "InventoryBehaviour.h"
#include "GraphNodeBehaviour.h"
#include "ColliderBehaviour.h"
#include "CompassBehaviour.h"
#include "PlayerCutsceneBehaviour.h"
#ifdef DEBUG_BUILD
#endif
#include "ExampleBehaviour.h"
#include "ButtonBehaviours.h"
#include "PictureBehaviour.h"
#include "MonsterBehaviour.h"
#include "CreditsBehaviour.h"
#include "PickupBehaviour.h"
#include "SoundBehaviour.h"
#include "HideBehaviour.h"
#include "GraphManager.h"
#include "SoundEngine.h"

using namespace DirectX;
using namespace Collisions;

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

#pragma endregion

#pragma region Initialization
Scene::Scene()
{
	_spotlights = std::make_unique<SpotLightCollection>();
	_pointlights = std::make_unique<PointLightCollection>();
}
Scene::~Scene()
{

}

bool Scene::Initialize(ID3D11Device *device, ID3D11DeviceContext *context, 
	Game *game, Content *content, Graphics *graphics, float gameVolume, const std::string &saveFile)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(86494680, "Scene Initialize");
#endif

	if (_initialized)
		return false;

	_game = game;
	_device = device;
	_context = context;
	_content = content;
	_graphics = graphics;
	_saveFile = saveFile;

	if (!_soundEngine.Initialize(
		AudioEngine_EnvironmentalReverb | AudioEngine_ReverbUseFilters /*| AudioEngine_UseMasteringLimiter*/,
		Reverb_Cave, gameVolume))
	{
		ErrMsg("Failed to initialize sound engine!");
		return false;
	}

	// Create scene content holder
	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(600.0f, 110.0f, 600.0f));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	if (!_pointlights->Initialize(device, 256))
	{
		ErrMsg("Failed to initialize pointlight collection!");
		return false;
	}

	if (!_spotlights->Initialize(device, 512))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}


#ifdef DEBUG_BUILD
	// Create global selection marker
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_WireframeCube");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_Red");
		mat.ambientID = _content->GetTextureID("Tex_Red");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Selection Marker"))
		{
			ErrMsg("Failed to initialize selection marker object!");
			return false;
		}

		MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to selection marker object!");
			return false;
		}

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global transform gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("TranslationGizmo");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_TransformGizmo");
		mat.ambientID = _content->GetTextureID("Tex_TransformGizmo");
		mat.samplerID = _content->GetSamplerID("SS_Point");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Transform Gizmo"))
		{
			ErrMsg("Failed to initialize transform gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global pointer gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_Sphere");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_White");
		mat.ambientID = _content->GetTextureID("Tex_White");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Pointer Gizmo"))
		{
			ErrMsg("Failed to initialize pointer gizmo object!");
			return false;
		}

		MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to pointer gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.1f, 0.1f, 0.1f });

		_globalEntities.push_back(std::move(entPtr));
	}
#endif

	std::ifstream file(std::format("Content/Saves/{}.txt", _saveFile));
	uintmax_t fileSize = 0;
	if (file.is_open())
	{
		fileSize = std::filesystem::file_size(std::format("Content/Saves/{}.txt", _saveFile));
	}

	if (fileSize > 0)
	{
		// Deserialize scene
		if (!Deserialize()) 
		{
			ErrMsg("Could not load in saved Entities");
			return false;
		}
	}
	else
	{
		// Create Light
		{
			Entity* light = nullptr;
			if (!CreateSpotLightEntity(&light, "lS", {60,60,60}, 1.2f, 90.0f, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			light->SetSerialization(false);
			light->GetTransform()->SetPosition({ 0.0f, 16.0f, 0.0f });
			light->GetTransform()->SetEuler({ 90.0f * DEG_TO_RAD, 0.0f, 0.0f });
		}

		// Create Light
		{
			Entity* light = nullptr;
			if (!CreatePointLightEntity(&light, "lP", {60,0,0}, 2.5f, 0.1f, 5))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			light->SetSerialization(false);
			light->GetTransform()->SetPosition({ 8.0f, 3.0f, 12.0f });
		}

#ifdef DEBUG_BUILD
		// Create debug player
		{
			Entity* player = nullptr;
			if (!CreateEntity(&player, "DebugPlayer", { {}, {.1f,.1f,.1f}, {0,0,0,1} }, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			DebugPlayerBehaviour* behaviour = new DebugPlayerBehaviour();
			if (!behaviour->Initialize(player))
			{
				ErrMsg("Failed to initialize debug player behaviour!");
				return false;
			}

			_debugPlayer = behaviour;
		}
#endif

		// Create room
		{
			UINT meshID = content->GetMeshID("Mesh_Plane");
			Material mat;
			mat.textureID = content->GetTextureID("Tex_Metal");
			mat.normalID = content->GetTextureMapID("TexMap_Metal_Normal");
			mat.specularID = content->GetTextureMapID("TexMap_Metal_Specular");
			mat.samplerID = content->GetSamplerID("SS_Wrap");
			mat.psID = content->GetShaderID("PS_TriPlanar");

			Entity* ent = nullptr;
			if (!CreateMeshEntity(&ent, "Room Floor", meshID, mat, false, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			ent->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });

			ColliderBehaviour* collision = new ColliderBehaviour();
			if (!collision->Initialize(ent))
			{
				ErrMsg("Failed to initialize collision behaviour!");
				return false;
			}

			BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
			bounds.Extents.x += 0.25f;
			bounds.Extents.y += 0.5f;
			bounds.Extents.z += 0.25f;
			bounds.Center.y -= 0.5f;
			collision->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));
#ifdef DEBUG_BUILD
			if (!collision->SetDebugCollider(this, content))
			{
				ErrMsg("Failed to create collider to player!");
				return false;
			}
#endif


			if (!CreateMeshEntity(&ent, "Room Roof", meshID, mat, false, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			ent->GetTransform()->SetPosition({ 0.0f, 30.0f, 0.0f });
			ent->GetTransform()->SetEuler({ 0.0f, 0.0f, XM_PI });
			ent->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });


			if (!CreateMeshEntity(&ent, "Room Wall South", meshID, mat, false, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			ent->GetTransform()->SetPosition({ 0.0f, 15.0f, -15.0f });
			ent->GetTransform()->SetEuler({ 0.5f * XM_PI, 0.0f, 0.0f });
			ent->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });

			collision = new ColliderBehaviour();
			if (!collision->Initialize(ent))
			{
				ErrMsg("Failed to initialize collision behaviour!");
				return false;
			}

			bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
			bounds.Extents.x += 0.25f;
			bounds.Extents.y += 0.5f;
			bounds.Extents.z += 0.25f;
			bounds.Center.y -= 0.5f;
			collision->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));
#ifdef DEBUG_BUILD
			if (!collision->SetDebugCollider(this, content))
			{
				ErrMsg("Failed to create collider to player!");
				return false;
			}
#endif


			if (!CreateMeshEntity(&ent, "Room Wall North", meshID, mat, false, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			ent->GetTransform()->SetPosition({ 0.0f, 15.0f, 15.0f });
			ent->GetTransform()->SetEuler({ -0.5f * XM_PI, 0.0f, 0.0f });
			ent->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });

			collision = new ColliderBehaviour();
			if (!collision->Initialize(ent))
			{
				ErrMsg("Failed to initialize collision behaviour!");
				return false;
			}

			bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
			bounds.Extents.x += 0.25f;
			bounds.Extents.y += 0.5f;
			bounds.Extents.z += 0.25f;
			bounds.Center.y -= 0.5f;
			collision->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));
#ifdef DEBUG_BUILD
			if (!collision->SetDebugCollider(this, content))
			{
				ErrMsg("Failed to create collider to player!");
				return false;
			}
#endif


			if (!CreateMeshEntity(&ent, "Room Wall West", meshID, mat, false, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			ent->GetTransform()->SetPosition({ -15.0f, 15.0f, 0.0f });
			ent->GetTransform()->SetEuler({ 0.0f, 0.0f, -0.5f * XM_PI });
			ent->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });

			collision = new ColliderBehaviour();
			if (!collision->Initialize(ent))
			{
				ErrMsg("Failed to initialize collision behaviour!");
				return false;
			}

			bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
			bounds.Extents.x += 0.25f;
			bounds.Extents.y += 0.5f;
			bounds.Extents.z += 0.25f;
			bounds.Center.y -= 0.5f;
			collision->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));
#ifdef DEBUG_BUILD
			if (!collision->SetDebugCollider(this, content))
			{
				ErrMsg("Failed to create collider to player!");
				return false;
			}
#endif


			if (!CreateMeshEntity(&ent, "Room Wall East", meshID, mat, false, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			ent->GetTransform()->SetPosition({ 15.0f, 15.0f, 0.0f });
			ent->GetTransform()->SetEuler({ 0.0f, 0.0f, 0.5f * XM_PI });
			ent->GetTransform()->SetScale({ 15.0f, 15.0f, 15.0f });

			collision = new ColliderBehaviour();
			if (!collision->Initialize(ent))
			{
				ErrMsg("Failed to initialize collision behaviour!");
				return false;
			}

			bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
			bounds.Extents.x += 0.25f;
			bounds.Extents.y += 0.5f;
			bounds.Extents.z += 0.25f;
			bounds.Center.y -= 0.5f;
			collision->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));
#ifdef DEBUG_BUILD
			if (!collision->SetDebugCollider(this, content))
			{
				ErrMsg("Failed to create collider to player!");
				return false;
			}
#endif
		}

#ifdef DEBUG_BUILD
		// Create Maxwell
		{
			UINT meshID = content->GetMeshID("Mesh_Maxwell");
			Material mat = Material::MakeMat(
				content->GetTextureID("Tex_Maxwell"),
				CONTENT_NULL,
				content->GetTextureMapID("TexMap_Default_Specular"),
				CONTENT_NULL,
				content->GetTextureID("Tex_AmbientBright")
			);

			Entity* maxwell = nullptr;
			if (!CreateMeshEntity(&maxwell, "Maxwell", meshID, mat))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			maxwell->GetTransform()->SetPosition({ 7.0f, 0.75f, 8.0f });


			meshID = content->GetMeshID("Mesh_Whiskers");
			mat.textureID = content->GetTextureID("Tex_Whiskers");
			mat.specularID = content->GetTextureMapID("TexMap_Black_Specular");

			Entity* whiskers = nullptr;
			if (!CreateMeshEntity(&whiskers, "Whiskers", meshID, mat, true))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			whiskers->SetParent(maxwell);


			ExampleBehaviour* behaviour = new ExampleBehaviour();
			if (!behaviour->Initialize(maxwell))
			{
				ErrMsg("Failed to initialize example behaviour!");
				return false;
			}

			/*
			SoundBehaviour *soundBehaviour = new SoundBehaviour("Maxwell Cat Loop",
			SoundEffectInstance_Use3D | SoundEffectInstance_ReverbUseFilters, true);
			if (!soundBehaviour->Initialize(maxwell))
			{
			ErrMsg("Failed to Initialize sound behaviour!");
			return false;
			}
			soundBehaviour->SetEnabled(false);

			soundBehaviour->SetEmitterPosition({ 7.0f, 0.75f, 8.0f });
			*/

			PickupBehaviour* pickupBehaviour = new PickupBehaviour();
			if (!pickupBehaviour->Initialize(maxwell))
			{
				ErrMsg("Failed to Initialize pickup behaviour!");
				return false;
			}
		}
#endif

		// Create Breadcrumb Pile
		{
			const BoundingOrientedBox bounds = { {0,0,0}, {0.25f, 0.25f, 0.25f}, {0,0,0,1} };

			Entity* crumbPile = nullptr;
			if (!CreateEntity(&crumbPile, "Breadcrumb Pile", bounds, true))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			crumbPile->GetTransform()->SetPosition({ -12.0f, 0.02f, 14.0f });

			BreadcrumbPileBehaviour* behaviour = new BreadcrumbPileBehaviour();
			if (!behaviour->Initialize(crumbPile))
			{
				ErrMsg("Failed to initialize breadcrumb pile behaviour!");
				return false;
			}
		}

		// Create Breadcrumb
		{
			const BoundingOrientedBox bounds = { {0,0,0}, {0.2f, 0.2f, 0.2f}, {0,0,0,1} };

			Entity* crumb = nullptr;
			if (!CreateEntity(&crumb, "Breadcrumb Red", bounds, true))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			crumb->GetTransform()->SetPosition({ 10.0f, 0.55f, -14.0f });

			BreadcrumbBehaviour* behaviour = new BreadcrumbBehaviour(BreadcrumbColor::Red);
			if (!behaviour->Initialize(crumb))
			{
				ErrMsg("Failed to initialize breadcrumb behaviour!");
				return false;
			}


			if (!CreateEntity(&crumb, "Breadcrumb Green", bounds, true))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			crumb->GetTransform()->SetPosition({ 12.0f, 0.6f, -13.5f });

			behaviour = new BreadcrumbBehaviour(BreadcrumbColor::Green);
			if (!behaviour->Initialize(crumb))
			{
				ErrMsg("Failed to initialize breadcrumb behaviour!");
				return false;
			}


			if (!CreateEntity(&crumb, "Breadcrumb Blue", bounds, true))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			crumb->GetTransform()->SetPosition({ 13.0f, 0.65f, -4.5f });

			behaviour = new BreadcrumbBehaviour(BreadcrumbColor::Blue);
			if (!behaviour->Initialize(crumb))
			{
				ErrMsg("Failed to initialize breadcrumb behaviour!");
				return false;
			}
		}

		// Create camera behaviour
		{
			UINT meshID = content->GetMeshID("Mesh_Camera");
			Material mat = Material::MakeMat(
				content->GetTextureID("Tex_Camera"),
				content->GetTextureMapID("TexMap_Camera_Normal"),
				content->GetTextureMapID("TexMap_Camera_Specular"),
				CONTENT_NULL,
				CONTENT_NULL,
				CONTENT_NULL,
				content->GetTextureMapID("TexMap_Camera_Occlusion")
			);

			Entity* ent = nullptr;
			if (!CreateMeshEntity(&ent, "Camera", meshID, mat))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			ent->GetTransform()->SetPosition({ 7.0f, 1.75f, 0.0f });
			ent->GetTransform()->SetScale({ 0.7f, 0.7f, 0.7f });

			CameraItemBehaviour* behaviour = new CameraItemBehaviour();
			if (!behaviour->Initialize(ent))
			{
				ErrMsg("Failed to initialize camera behaviour!");
				return false;
			}
		}

#ifdef DEBUG_BUILD
		// Create collision example
		{
			// Nr. 1
			{
				UINT meshID = content->GetMeshID("Mesh_Maxwell");
				Material mat = Material::MakeMat(
					content->GetTextureID("Tex_Maxwell"),
					CONTENT_NULL,
					CONTENT_NULL,
					CONTENT_NULL,
					content->GetTextureID("Tex_AmbientBright")
				);

				Entity* maxwell = nullptr;
				if (!CreateMeshEntity(&maxwell, "Maxwell the first", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				maxwell->GetTransform()->SetPosition({ -7.0f, 0.75f, 6.0f });
				BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				//maxwell->StoreEntityBounds(bounds);
				//maxwell->SetCollider(new Collisions::Capsule(bounds.Center, { 0, 1, 0 }, bounds.Extents.y, bounds.Extents.z));
				//if (!maxwell->GetCollider()->SetDebugEnt(this, content))
				//{
				//	ErrMsg("Failed to create collider debug entity!");
				//	return false;
				//}
				ColliderBehaviour* colB = new ColliderBehaviour();
				if (!colB->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize Maxwell the 1st!");
					return false;
				}

				colB->SetCollider(new Collisions::Capsule(bounds.Center, { 0, 1, 0 }, 0.3f, 0.95f));

				ExampleCollisionBehaviour* behaviour = new ExampleCollisionBehaviour();
				if (!behaviour->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize example behaviour!");
					return false;
				}

				meshID = content->GetMeshID("Mesh_Whiskers");
				mat.textureID = content->GetTextureID("Tex_Whiskers");
				mat.specularID = content->GetTextureMapID("TexMap_Black_Specular");

				Entity* whiskers = nullptr;
				if (!CreateMeshEntity(&whiskers, "Whiskers", meshID, mat, true))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				whiskers->SetParent(maxwell);
			}

			// Nr. 2
			{
				UINT meshID = content->GetMeshID("Mesh_Maxwell");
				Material mat = Material::MakeMat(
					content->GetTextureID("Tex_Maxwell"),
					CONTENT_NULL,
					CONTENT_NULL,
					CONTENT_NULL,
					content->GetTextureID("Tex_AmbientBright")
				);

				Entity* maxwell = nullptr;
				if (!CreateMeshEntity(&maxwell, "Maxwell the second", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				maxwell->GetTransform()->SetPosition({ -7.0f, 0.75f, 8.0f });
				BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				/*
				BoundingOrientedBox bounds;
				maxwell->StoreEntityBounds(bounds);
				//maxwell->SetCollider(new Collisions::Capsule(bounds.Center, { 0, 1, 0 }, bounds.Extents.y, bounds.Extents.z));
				maxwell->SetCollider(new Collisions::OBB(bounds));
				if (!maxwell->GetCollider()->SetDebugEnt(this, content))
				{
				ErrMsg("Failed to create collider debug entity!");
				return false;
				}
				*/

				ColliderBehaviour* colB = new ColliderBehaviour();
				if (!colB->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize Maxwell the 2nd!");
					return false;
				}
				colB->SetCollider(new Collisions::OBB(bounds));

				ExampleCollisionBehaviour* behaviour = new ExampleCollisionBehaviour();
				if (!behaviour->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize example behaviour!");
					return false;
				}


				meshID = content->GetMeshID("Mesh_Whiskers");
				mat.textureID = content->GetTextureID("Tex_Whiskers");
				mat.specularID = content->GetTextureMapID("TexMap_Black_Specular");

				Entity* whiskers = nullptr;
				if (!CreateMeshEntity(&whiskers, "Whiskers", meshID, mat, true))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				whiskers->SetParent(maxwell);
			}

			// Nr. 3
			{
				UINT meshID = content->GetMeshID("Mesh_Maxwell");
				Material mat = Material::MakeMat(
					content->GetTextureID("Tex_Maxwell"),
					CONTENT_NULL,
					CONTENT_NULL,
					CONTENT_NULL,
					content->GetTextureID("Tex_AmbientBright")
				);

				Entity* maxwell = nullptr;
				if (!CreateMeshEntity(&maxwell, "Maxwell the third", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				maxwell->GetTransform()->SetPosition({ -8.5f, 0.75f, 6.5f });
				BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				ColliderBehaviour* colB = new ColliderBehaviour();
				if (!colB->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize Maxwell the 3rd!");
					return false;
				}

				colB->SetCollider(new Collisions::Sphere(bounds.Center, bounds.Extents.x, (Collisions::ColliderTags)(Collisions::GROUND_TAG | Collisions::OBJECT_TAG)));

				ExampleCollisionBehaviour* behaviour = new ExampleCollisionBehaviour();
				if (!behaviour->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize example behaviour!");
					return false;
				}

				meshID = content->GetMeshID("Mesh_Whiskers");
				mat.textureID = content->GetTextureID("Tex_Whiskers");
				mat.specularID = content->GetTextureMapID("TexMap_Black_Specular");

				Entity* whiskers = nullptr;
				if (!CreateMeshEntity(&whiskers, "Whiskers", meshID, mat, true))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				whiskers->SetParent(maxwell);
			}

			// Nr. 4
			{
				UINT meshID = content->GetMeshID("Mesh_Maxwell");
				Material mat = Material::MakeMat(
					content->GetTextureID("Tex_Maxwell"),
					CONTENT_NULL,
					CONTENT_NULL,
					CONTENT_NULL,
					content->GetTextureID("Tex_AmbientBright")
				);

				Entity* maxwell = nullptr;
				if (!CreateMeshEntity(&maxwell, "Maxwell the fourth", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				maxwell->GetTransform()->SetPosition({ -5.0f, 0.75f, 6.5f });
				BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				ColliderBehaviour* colB = new ColliderBehaviour();
				if (!colB->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize Maxwell the 4th!");
					return false;
				}

				colB->SetCollider(new Collisions::AABB(bounds));
				//colB->SetCollider(new Collisions::Ray(bounds.Center, { 0, 0, -1 }, 1));

				ExampleCollisionBehaviour* behaviour = new ExampleCollisionBehaviour();
				if (!behaviour->Initialize(maxwell))
				{
					ErrMsg("Failed to initialize example behaviour!");
					return false;
				}

				meshID = content->GetMeshID("Mesh_Whiskers");
				mat.textureID = content->GetTextureID("Tex_Whiskers");
				mat.specularID = content->GetTextureMapID("TexMap_Black_Specular");

				Entity* whiskers = nullptr;
				if (!CreateMeshEntity(&whiskers, "Whiskers", meshID, mat, true))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				whiskers->SetParent(maxwell);
			}

#ifndef FAST_LOAD_MODE
			// Terrian Colldier
			{
				UINT meshID = content->GetMeshID("Mesh_HeightMapExample");
				Material mat = Material::MakeMat(
					content->GetTextureID("Tex_Green"),
					CONTENT_NULL,
					content->GetTextureMapID("TexMap_Default_Specular"),
					content->GetTextureID("Tex_AmbientBright"),
					CONTENT_NULL,
					CONTENT_NULL);

				Entity* terrain = nullptr;
				if (!CreateMeshEntity(&terrain, "Terrain", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				terrain->GetTransform()->SetPosition({ -5.0f, 0.4f, 2.5f });
				//terrain->GetTransform()->SetScale({ 7.1f, 1.0f, 2.0f });

				BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				ColliderBehaviour* colB = new ColliderBehaviour();
				if (!colB->Initialize(terrain))
				{
					ErrMsg("Failed to initialize Maxwell the 4th!");
					return false;
				}

				colB->SetCollider(new Collisions::Terrain(bounds.Center, bounds.Extents, content->GetHeightMap("HM_ExampleHeightMap")));
			}
#endif

			// Slope Colldier
			{
				UINT meshID = content->GetMeshID("Mesh_Fallback");
				Material mat = Material::MakeMat(
					content->GetTextureID("Tex_Fallback"),
					CONTENT_NULL,
					CONTENT_NULL,
					CONTENT_NULL,
					content->GetTextureID("Tex_Fallback")
				);

				Entity* ent = nullptr;
				if (!CreateMeshEntity(&ent, "Slope 10", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				ent->GetTransform()->SetPosition({ -12.75f, 0.3f, -3.0f });
				ent->GetTransform()->SetEuler({ 0.0f, 0.0f, -10.0f * DEG_TO_RAD });
				ent->GetTransform()->SetScale({ 5.0f, 0.2f, 1.0f });

				BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				ColliderBehaviour* colB = new ColliderBehaviour();
				if (!colB->Initialize(ent))
				{
					ErrMsg("Failed to initialize slope collider!");
					return false;
				}

				colB->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));


				if (!CreateMeshEntity(&ent, "Slope 20", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				ent->GetTransform()->SetPosition({ -12.75f, 0.3f, -5.0f });
				ent->GetTransform()->SetEuler({ 0.0f, 0.0f, -20.0f * DEG_TO_RAD });
				ent->GetTransform()->SetScale({ 5.0f, 0.2f, 1.0f });

				bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				colB = new ColliderBehaviour();
				if (!colB->Initialize(ent))
				{
					ErrMsg("Failed to initialize slope collider!");
					return false;
				}

				colB->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));


				if (!CreateMeshEntity(&ent, "Slope 30", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				ent->GetTransform()->SetPosition({ -12.75f, 0.3f, -7.0f });
				ent->GetTransform()->SetEuler({ 0.0f, 0.0f, -30.0f * DEG_TO_RAD });
				ent->GetTransform()->SetScale({ 5.0f, 0.2f, 1.0f });

				bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				colB = new ColliderBehaviour();
				if (!colB->Initialize(ent))
				{
					ErrMsg("Failed to initialize slope collider!");
					return false;
				}

				colB->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));


				if (!CreateMeshEntity(&ent, "Slope 40", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				ent->GetTransform()->SetPosition({ -12.75f, 0.3f, -9.0f });
				ent->GetTransform()->SetEuler({ 0.0f, 0.0f, -40.0f * DEG_TO_RAD });
				ent->GetTransform()->SetScale({ 5.0f, 0.2f, 1.0f });

				bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				colB = new ColliderBehaviour();
				if (!colB->Initialize(ent))
				{
					ErrMsg("Failed to initialize slope collider!");
					return false;
				}

				colB->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));


				if (!CreateMeshEntity(&ent, "Slope 50", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				ent->GetTransform()->SetPosition({ -12.75f, 0.3f, -11.0f });
				ent->GetTransform()->SetEuler({ 0.0f, 0.0f, -50.0f * DEG_TO_RAD });
				ent->GetTransform()->SetScale({ 5.0f, 0.2f, 1.0f });

				bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				colB = new ColliderBehaviour();
				if (!colB->Initialize(ent))
				{
					ErrMsg("Failed to initialize slope collider!");
					return false;
				}

				colB->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));


				if (!CreateMeshEntity(&ent, "Slope 60", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				ent->GetTransform()->SetPosition({ -12.75f, 0.3f, -13.0f });
				ent->GetTransform()->SetEuler({ 0.0f, 0.0f, -60.0f * DEG_TO_RAD });
				ent->GetTransform()->SetScale({ 5.0f, 0.2f, 1.0f });

				bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
				colB = new ColliderBehaviour();
				if (!colB->Initialize(ent))
				{
					ErrMsg("Failed to initialize slope collider!");
					return false;
				}

				colB->SetCollider(new Collisions::OBB(bounds, Collisions::MAP_COLLIDER_TAGS));
			}
		}

		// Create UI button example behaviour
		{
			UINT meshID = content->GetMeshID("Mesh_Maxwell");
			Material mat = Material::MakeMat(
				content->GetTextureID("Tex_Maxwell"),
				CONTENT_NULL,
				CONTENT_NULL,
				CONTENT_NULL,
				content->GetTextureID("Tex_AmbientBright")
			);

			Entity* maxwell = nullptr;
			if (!CreateMeshEntity(&maxwell, "Maxwell 2", meshID, mat))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			maxwell->GetTransform()->SetPosition({ 5.0f, 0.75f, 8.0f });

			UIButtonExampleBehaviour* behaviour = new UIButtonExampleBehaviour();
			if (!behaviour->Initialize(maxwell))
			{
				ErrMsg("Failed to initialize UI button example behaviour!");
				return false;
			}
		}
#endif
		/*
		// Create player movement behaviour
		{
			Entity* playerHolderEntity = nullptr;
			if (!CreateEntity(&playerHolderEntity, "Player Holder", { {0,0,0},{1,1,1},{0,0,0,1} }, false))
			{
				ErrMsg("Failed to initialize player holder!");
				return false;
			}


			Entity* playerEntity = nullptr;
			if (!CreateEntity(&playerEntity, "Player Entity", { {0,0,0},{1,1,1},{0,0,0,1} }, false))
			{
				ErrMsg("Failed to initialize player!");
				return false;
			}

			playerEntity->SetParent(playerHolderEntity);
			playerEntity->GetTransform()->SetPosition({ 9.0f, 0.0f, 8.0f });

			PlayerMovementBehaviour* movementBehaviour = new PlayerMovementBehaviour();
			if (!movementBehaviour->Initialize(playerEntity))
			{
				ErrMsg("Failed to initialize movement example behaviour!");
				return false;
			}

			InteractorBehaviour *interactorBehaviour = new InteractorBehaviour();
			if (!interactorBehaviour->Initialize(playerEntity))
			{
				ErrMsg("Failed to initialize interactor behaviour!");
				return false;
			}

			ColliderBehaviour *collision = new ColliderBehaviour();
			if (!collision->Initialize(playerEntity))
			{
				ErrMsg("Failed to initialize collision behaviour!");
				return false;
			}

			collision->SetCollider(new Collisions::Capsule( { 0.0f, 11.8858f * 0.3f / 2.0f, 0.0f }, {0, 1, 0}, 0.3f, 11.8858f * 0.3f));
#ifdef DEBUG_BUILD
			if (!collision->SetDebugCollider(this, content))
			{
				ErrMsg("Failed to create collider to player!");
				return false;
			}
#endif


			Entity *playerHeadTracker = nullptr;
			if (!CreateEntity(&playerHeadTracker, "Player Head Tracker", { {0,0,0},{.1f,.1f,.1f},{0,0,0,1} }, false))
			{
				ErrMsg("Failed to initialize player head tracker!");
				return false;
			}

			playerHeadTracker->SetParent(playerEntity);
			playerHeadTracker->GetTransform()->SetPosition({ 0.0f, 3.3f, 0.0f });

			movementBehaviour->SetHeadTracker(playerHeadTracker);


			Entity* cam = nullptr;
			if (!CreateEntity(&cam, "playerCamera", { {0,0,0},{.1f,.1f,.1f},{0,0,0,1} }, false))
			{
				ErrMsg("Failed to create playerCamera entity!");
				return false;
			}

			cam->SetParent(playerHolderEntity);
			cam->GetTransform()->SetPosition(playerHeadTracker->GetTransform()->GetPosition(World), World);
			cam->GetTransform()->SetRotation(playerHeadTracker->GetTransform()->GetRotation(World), World);

			ProjectionInfo camInfo = ProjectionInfo(80.0f * DEG_TO_RAD, 16.0f / 9.0f, { 0.05f, 60.0f });
			CameraBehaviour* camBehaviour = new CameraBehaviour(camInfo);
			if (!camBehaviour->Initialize(cam))
			{
				ErrMsg("Failed to initialize UI example behaviour!");
				return false;
			}

			movementBehaviour->SetPlayerCamera(camBehaviour);

			PlayerViewBehaviour *viewBehaviour = new PlayerViewBehaviour(movementBehaviour);
			if (!viewBehaviour->Initialize(cam))
			{
				ErrMsg("Failed to initialize player view behaviour!");
				return false;
			}


			SimplePointLightBehaviour *lightBehaviour = new SimplePointLightBehaviour({0.045f, 0.05f, 0.055f}, 0.2f);
			if (!lightBehaviour->Initialize(cam))
			{
				ErrMsg("Failed to initialize player darkness light behaviour!");
				return false;
			}

		}

		// Create flashlight
		{
			UINT meshID = content->GetMeshID("Mesh_FlashlightBody");
			Material mat = Material::MakeMat(
				content->GetTextureID("Tex_FlashlightBody"),
				content->GetTextureMapID("TexMap_FlashlightBody_Normal"),
				content->GetTextureMapID("TexMap_FlashlightBody_Specular"),
				content->GetTextureMapID("TexMap_FlashlightBody_Glossiness")
			);

			Entity* flashlight = nullptr;
			if (!CreateMeshEntity(&flashlight, "Flashlight", meshID, mat, false, false))
			{
				ErrMsg("Failed to initialize flashlight");
				return false;
			}
			flashlight->SetParent(_sceneHolder.GetEntityByName("playerCamera"));
			flashlight->GetTransform()->SetPosition({ 0.7f, -0.4f, 0.8f });
			flashlight->GetTransform()->SetEuler({ 0.0f, 0.0f, DirectX::XM_PI });
			flashlight->GetTransform()->SetScale({ 0.15f, 0.15f, 0.15f });

			meshID = content->GetMeshID("Mesh_FlashlightLever");
			mat = Material::MakeMat(
				content->GetTextureID("Tex_FlashlightLever"),
				content->GetTextureMapID("TexMap_FlashlightLever_Normal"),
				content->GetTextureMapID("TexMap_FlashlightLever_Specular"),
				content->GetTextureMapID("TexMap_FlashlightLever_Glossiness")
			);

			Entity *flashlightLever = nullptr;
			if (!CreateMeshEntity(&flashlightLever, "FlashlightLever", meshID, mat, false, false))
			{
				ErrMsg("Failed to initialize flashlight lever");
				return false;
			}
			flashlightLever->SetParent(_sceneHolder.GetEntityByName("Flashlight"));

			FlashlightBehaviour* flashlightBehaviour = new FlashlightBehaviour();
			if (!flashlightBehaviour->Initialize(flashlight))
			{
				ErrMsg("Failed to initialize flashlight behaviour!");
				return false;
			}

		}
		*/



		// Sound emitters
		{
			float range = 10.0f;
			std::vector<std::string > soundFiles = { "Ambience_Water_Dripping_Slow_02_Loop", "Ambience_Water_Dripping_Fast_Loop", "Ambience_Water_Dripping_Slow_01_Loop", };

			for (auto soundFile : soundFiles)
			{
				Entity* emitter = nullptr;
				if (!CreateSoundEmitterEntity(&emitter, soundFile, soundFile, DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters, true))
				{
					ErrMsg("Failed to create sound emitter entity!");
					return false;
				}
				float x = -range + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2 * range)));
				float y = -range + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2 * range)));
				float z = -range + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2 * range)));

				emitter->GetTransform()->SetPosition({ x, y, z });
			}
		}


		// Create disjointed graph starter node
		if (_graphManager.GetNodeCount() <= 0)
		{
			Entity* ent = nullptr;
			GraphNodeBehaviour* node = nullptr;
			if (!CreateGraphNodeEntity(&ent, &node, { 0, 0, 0 }))
			{
				ErrMsg("Failed to create graph node entity!");
				return false;
			}
		}
	}

#ifdef DEBUG_BUILD
	// Create transform gizmo controller
	{
		Entity *ent = nullptr;
		if (!CreateEntity(&ent, "Transform Controller", BoundingOrientedBox({ 0,0,0 }, { 0.01f, 0.01f, 0.01f }, { 0,0,0,1 }), false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		ent->SetSerialization(false);
		ent->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		_transformGizmo = new TransformGizmoBehaviour();
		if (!_transformGizmo->Initialize(ent))
		{
			ErrMsg("Failed to initialize transform gizmo behaviour!");
			return false;
		}
	}
#endif

	_collisionHandler.Initialize(this);

	_initialized = true;
	return true;
	}
bool Scene::InitializeMenu(ID3D11Device* device, ID3D11DeviceContext* context,
	Game* game, Content* content, Graphics* graphics, float gameVolume, const std::string& saveFile)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(86494680, "Scene Initialize");
#endif

	if (_initialized)
		return false;

	_game = game;
	_device = device;
	_context = context;
	_content = content;
	_graphics = graphics;
	_saveFile = saveFile;

	if (!_soundEngine.Initialize(
		AudioEngine_EnvironmentalReverb | AudioEngine_ReverbUseFilters /*| AudioEngine_UseMasteringLimiter*/,
		Reverb_Cave, gameVolume))
	{
		ErrMsg("Failed to initialize sound engine!");
		return false;
	}

	// Create scene content holder
	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(50.0f, 50.0f, 50.0f));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	if (!_pointlights->Initialize(device, 256))
	{
		ErrMsg("Failed to initialize pointlight collection!");
		return false;
	}

	if (!_spotlights->Initialize(device, 512))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}

#ifdef DEBUG_BUILD
	// Create global selection marker
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_WireframeCube");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_Red");
		mat.ambientID = _content->GetTextureID("Tex_Red");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Selection Marker"))
		{
			ErrMsg("Failed to initialize selection marker object!");
			return false;
		}

		MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to selection marker object!");
			return false;
		}

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global transform gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("TranslationGizmo");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_TransformGizmo");
		mat.ambientID = _content->GetTextureID("Tex_TransformGizmo");
		mat.samplerID = _content->GetSamplerID("SS_Point");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Transform Gizmo"))
		{
			ErrMsg("Failed to initialize transform gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		/*MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
		ErrMsg("Failed to initialize transform gizmo object!");
		return false;
		}*/

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global pointer gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_Sphere");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_White");
		mat.ambientID = _content->GetTextureID("Tex_White");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Pointer Gizmo"))
		{
			ErrMsg("Failed to initialize pointer gizmo object!");
			return false;
		}

		MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to pointer gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.1f, 0.1f, 0.1f });

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create debug player
	{
		Entity* player = nullptr;
		if (!CreateEntity(&player, "DebugPlayer", { {}, {.1f,.1f,.1f}, {0,0,0,1} }, false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		DebugPlayerBehaviour* behaviour = new DebugPlayerBehaviour();
		if (!behaviour->Initialize(player))
		{
			ErrMsg("Failed to initialize debug player behaviour!");
			return false;
		}

		_debugPlayer = behaviour;
		behaviour->SetEnabled(false);
	}
#endif

	{
		// Cave Wall
		{
			UINT meshID = content->GetMeshID("Mesh_Cave_Wall");
			Material mat;
			mat.textureID = content->GetTextureID("Tex_MineSection");
			mat.normalID = content->GetTextureMapID("TexMap_MineSection_Normal");

			Entity* ent = nullptr;
			if (!CreateMeshEntity(&ent, "Cave Wall", meshID, mat, false, false))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			ent->GetTransform()->SetPosition({ 0.0f, 0.0f, 10.0f });
			ent->GetTransform()->SetScale({ 5.0f, 5.0f, 5.0f });
		}

		// Buttons
		{
			UINT meshID = content->GetMeshID("Mesh_GraniteRock");
			Material mat;
			mat.normalID = content->GetTextureMapID("TexMap_Cave_Normal");

			XMFLOAT3A buttonPos = { -3.5f, 1.5f, 5.0f };

			// Start button
			{
				std::ifstream file(std::format("Content/Saves/{}.txt", "GameSave"));
				uintmax_t fileSize = 0;
				if (file.is_open())
				{
					fileSize = std::filesystem::file_size(std::format("Content/Saves/{}.txt", "GameSave"));
				}

				if (fileSize == 0)
					mat.textureID = content->GetTextureID("Tex_Button_Start_Texture");
				else
					mat.textureID = content->GetTextureID("Tex_Button_Load_Texture");

				Entity* ent = nullptr;
				if (!CreateMeshEntity(&ent, "StartButton", meshID, mat, false, false))
				{
					ErrMsg("Failed to create object!");
					return false;
				}
				ent->GetTransform()->SetPosition(buttonPos);
				ent->GetTransform()->SetScale({ 0.7f, 0.2f, 0.2f });

				PlayButtonBehaviour* play = new PlayButtonBehaviour();
				if (!play->Initialize(ent))
				{
					ErrMsg("Failed to initialize play button behaviour!");
					return false;
				}
			}

			// Save button
			{
				mat.textureID = content->GetTextureID("Tex_Button_Save_Texture");

				Entity* ent = nullptr;
				if (!CreateMeshEntity(&ent, "SaveButton", meshID, mat, false, false))
				{
					ErrMsg("Failed to create object!");
					return false;
				}
				buttonPos.y = 0.75f;
				ent->GetTransform()->SetPosition(buttonPos);
				ent->GetTransform()->SetScale({ 0.7f, 0.2f, 0.2f });

				SaveButtonBehaviour* save = new SaveButtonBehaviour();
				if (!save->Initialize(ent))
				{
					ErrMsg("Failed to initialize save button behaviour!");
					return false;
				}
			}

			// NewSave button
			{
				mat.textureID = content->GetTextureID("Tex_Button_NewSave_Texture");

				Entity* ent = nullptr;
				if (!CreateMeshEntity(&ent, "NewSaveButton", meshID, mat, false, false))
				{
					ErrMsg("Failed to create object!");
					return false;
				}
				buttonPos.y = 0.0f;
				ent->GetTransform()->SetPosition(buttonPos);
				ent->GetTransform()->SetScale({ 0.7f, 0.2f, 0.2f });

				NewSaveButtonBehaviour* save = new NewSaveButtonBehaviour();
				if (!save->Initialize(ent))
				{
					ErrMsg("Failed to initialize new save button behaviour!");
					return false;
				}
			}

			// Credits button
			{
				mat.textureID = content->GetTextureID("Tex_Button_Credits_Texture");

				Entity *ent = nullptr;
				if (!CreateMeshEntity(&ent, "CreditsButton", meshID, mat, false, false))
				{
					ErrMsg("Failed to create object!");
					return false;
				}
				buttonPos.y = -0.75f;
				ent->GetTransform()->SetPosition(buttonPos);
				ent->GetTransform()->SetScale({ 0.7f, 0.2f, 0.2f });

				CreditsButtonBehaviour *save = new CreditsButtonBehaviour();
				if (!save->Initialize(ent))
				{
					ErrMsg("Failed to initialize new save button behaviour!");
					return false;
				}
			}

			// Exit button
			{
				mat.textureID = content->GetTextureID("Tex_Button_Exit_Texture");

				Entity* ent = nullptr;
				if (!CreateMeshEntity(&ent, "ExitButton", meshID, mat, false, false))
				{
					ErrMsg("Failed to create object!");
					return false;
				}
				buttonPos.y = -1.5f;
				ent->GetTransform()->SetPosition(buttonPos);
				ent->GetTransform()->SetScale({ 0.7f, 0.2f, 0.2f });

				ExitButtonBehaviour* exit = new ExitButtonBehaviour();
				if (!exit->Initialize(ent))
				{
					ErrMsg("Failed to initialize exit button behaviour!");
					return false;
				}
			}
		}

		// Menu camera
		{
			Entity* menuCam = nullptr;
			if (!CreateEntity(&menuCam, "MenuCamera", { {}, {.1f,.1f,.1f}, {0,0,0,1} }, false))
			{
				ErrMsg("Failed to create menu camera!");
				return false;
			}

			MenuCameraBehaviour* camera = new MenuCameraBehaviour();
			if (!camera->Initialize(menuCam))
			{
				ErrMsg("Failed to initialize menu camera behaviour!");
				return false;
			}
		}

		// Props
		{
			// Flashlight
			{
				UINT meshID = content->GetMeshID("Mesh_FlashlightBody");
				Material mat = Material::MakeMat(
					content->GetTextureID("Tex_FlashlightBody"),
					content->GetTextureMapID("TexMap_FlashlightBody_Normal"),
					content->GetTextureMapID("TexMap_FlashlightBody_Specular"),
					content->GetTextureMapID("TexMap_FlashlightBody_Glossiness")
				);

				Entity *flashlight = nullptr;
				if (!CreateMeshEntity(&flashlight, "FlashlightProp", meshID, mat))
				{
					ErrMsg("Failed to create object!");
					return false;
				}
				flashlight->GetTransform()->SetPosition({ -10.74f, -9.51f, 18.36f });
				flashlight->GetTransform()->Rotate({ -12.69f * DEG_TO_RAD, 60.0f * DEG_TO_RAD, 336.75f * DEG_TO_RAD });
				flashlight->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

				MeshBehaviour *meshBehaviour = nullptr;
				flashlight->GetBehaviourByType<MeshBehaviour>(meshBehaviour);
				meshBehaviour->SetCastShadows(false);

				meshID = content->GetMeshID("Mesh_FlashlightLever");
				mat = Material::MakeMat(
					content->GetTextureID("Tex_FlashlightLever"),
					content->GetTextureMapID("TexMap_FlashlightLever_Normal"),
					content->GetTextureMapID("TexMap_FlashlightLever_Specular"),
					content->GetTextureMapID("TexMap_FlashlightLever_Glossiness")
				);

				Entity *flashlightLever = nullptr;
				if (!CreateMeshEntity(&flashlightLever, "FlashlightLever", meshID, mat))
				{
					ErrMsg("Failed to initialize flashlight lever");
					return false;
				}
				flashlightLever->SetParent(_sceneHolder.GetEntityByName("FlashlightProp"));

				class FlashlightPropBehaviour : public Behaviour
				{
				private:
					bool _isOn = true;

					SpotLightBehaviour *_lightBehaviour = nullptr;

				protected:
					[[nodiscard]] bool Start() override
					{
						if (_name == "")
							_name = "FlashlightPropBehaviour"; // For categorization in ImGui.

						Entity *spotLight = nullptr;
						if (!GetScene()->CreateSpotLightEntity(&spotLight, "Spotlight Flashlight", { 500.0f, 500.0f, 500.0f }, 1.0f, 60.0f))
						{
							ErrMsg("Failed to create spotlight entity to flashlight!");
							return false;
						}
						spotLight->SetParent(GetScene()->GetSceneHolder()->GetEntityByName("FlashlightProp"));
						//spotLight->GetTransform()->SetPosition({ 0.0f, 0.0f, 1.5f });
						spotLight->SetSerialization(false);

						spotLight->GetBehaviourByType<SpotLightBehaviour>(_lightBehaviour);
					
						return true;
					}

					[[nodiscard]] bool Update(Time &time, const Input &input) override
					{
						if (input.GetKey(KeyCode::F) == KeyState::Pressed)
						{
							_isOn = !_isOn;
						}
						return true;
					}

					[[nodiscard]] bool FixedUpdate(const float &deltaTime, const Input &input) override
					{
						float flickerChance = !_isOn ? 1.0f : 0.15f; // Chance to flicker per fixed update
						if (rand() % 1000 < flickerChance * 1000)
						{
							_lightBehaviour->SetEnabled(false);
						}
						else
						{
							_lightBehaviour->SetEnabled(true);
						}

						return true;
					}

				public:
					FlashlightPropBehaviour() = default;
					~FlashlightPropBehaviour() = default;
				};

				FlashlightPropBehaviour *flpb = new FlashlightPropBehaviour();
				if (!flpb->Initialize(flashlight))
				{
					ErrMsg("Failed to initialize flashlight prop behaviour!");
					return false;
				}
			}

			// Breadcrumb
			{
				const BoundingOrientedBox bounds = { {0,0,0}, {0.2f, 0.2f, 0.2f}, {0,0,0,1} };

				Entity *crumbPile = nullptr;
				if (!CreateEntity(&crumbPile, "Breadcrumb Pile", bounds, true))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				crumbPile->GetTransform()->SetPosition({ 10.74f, -11.5f, 21.36f });
				crumbPile->GetTransform()->SetScale({ 5.0f, 5.0f, 5.0f });

				BreadcrumbPileBehaviour *behaviour = new BreadcrumbPileBehaviour();
				if (!behaviour->Initialize(crumbPile))
				{
					ErrMsg("Failed to initialize breadcrumb pile behaviour!");
					return false;
				}
			}

			// Controlls sign
			{
				UINT meshID = content->GetMeshID("minesign2legsflipped");
				Material mat;
				mat.textureID = content->GetTextureID("Tex_minesign2legs_ControllsSign_Diffuse");
				mat.normalID = content->GetTextureMapID("TexMap_minesign2legs_ControllsSign_Normal");
				mat.specularID = content->GetTextureMapID("TexMap_minesign2legs_ControllsSign_Specular");
				mat.glossinessID = content->GetTextureMapID("TexMap_minesign2legs_ControllsSign_Glossiness");

				Entity *sign = nullptr;
				if (!CreateMeshEntity(&sign, "Controlls sign", meshID, mat))
				{
					ErrMsg("Failed to create controlls sign mesh!");
					return false;
				}

				sign->GetTransform()->SetPosition({ 9.74f, -12.2f, 24.36f });
				sign->GetTransform()->SetRotation({ 0.0f, 95.0f * DEG_TO_RAD, 0.0f, 1.0f });
				sign->GetTransform()->SetScale({ 1.3f, 1.3f, 1.3f });
			}
		}
	}

#ifdef DEBUG_BUILD
	// Create transform gizmo controller
	{
		Entity *ent = nullptr;
		if (!CreateEntity(&ent, "Transform Controller", BoundingOrientedBox({ 0,0,0 }, { 0.01f, 0.01f, 0.01f }, { 0,0,0,1 }), false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		ent->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		_transformGizmo = new TransformGizmoBehaviour();
		if (!_transformGizmo->Initialize(ent))
		{
			ErrMsg("Failed to initialize transform gizmo behaviour!");
			return false;
		}
	}
#endif

	_collisionHandler.Initialize(this);

	_initialized = true;
	return true;
}
bool Scene::InitializeStartCutscene(ID3D11Device *device, ID3D11DeviceContext *context, Game *game, Content *content, Graphics *graphics, float gameVolume, const std::string &saveFile)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(86494680, "Start Cutscene Initialize");
#endif

	if (_initialized)
		return false;

	_game = game;
	_device = device;
	_context = context;
	_content = content;
	_graphics = graphics;
	_saveFile = saveFile;

	if (!_soundEngine.Initialize(
		AudioEngine_EnvironmentalReverb | AudioEngine_ReverbUseFilters /*| AudioEngine_UseMasteringLimiter*/,
		Reverb_Cave, gameVolume))
	{
		ErrMsg("Failed to initialize sound engine!");
		return false;
	}

	// Create scene content holder
	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(600.0f, 110.0f, 600.0f));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	if (!_pointlights->Initialize(device, 256))
	{
		ErrMsg("Failed to initialize pointlight collection!");
		return false;
	}

	if (!_spotlights->Initialize(device, 512))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}


	// Create global selection marker
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_WireframeCube");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_Red");
		mat.ambientID = _content->GetTextureID("Tex_Red");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Selection Marker"))
		{
			ErrMsg("Failed to initialize selection marker object!");
			return false;
		}

		MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to selection marker object!");
			return false;
		}

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global transform gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("TranslationGizmo");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_TransformGizmo");
		mat.ambientID = _content->GetTextureID("Tex_TransformGizmo");
		mat.samplerID = _content->GetSamplerID("SS_Point");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Transform Gizmo"))
		{
			ErrMsg("Failed to initialize transform gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		/*MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
		ErrMsg("Failed to initialize transform gizmo object!");
		return false;
		}*/

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global pointer gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_Sphere");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_White");
		mat.ambientID = _content->GetTextureID("Tex_White");
		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Pointer Gizmo"))
		{
			ErrMsg("Failed to initialize pointer gizmo object!");
			return false;
		}

		MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to pointer gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.1f, 0.1f, 0.1f });

		_globalEntities.push_back(std::move(entPtr));
	}

	//GetGraphics()->SetAmbientColor({ 0.22, 0.2, 0.1, 0 });
	GetGraphics()->SetAmbientColor({ 0.1f, 0.1f, 0.1f, 0 });

	// Floor
	{
		BoundingOrientedBox bounds = { {0, 0.444183499f, 0}, {0.5f, 0.444177479f, 0.5f}, {0, 0, 0, 1} };

		Entity *terrainEnt = nullptr;
		if (!CreateEntity(&terrainEnt, "Terrain Floor", bounds, true))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		terrainEnt->SetSerialization(false);
		terrainEnt->GetTransform()->SetPosition({0.0f, -33.125f, 0.0f });
		terrainEnt->GetTransform()->SetScale({ 600.0f, 112.5f, 600.0f });

		ColliderBehaviour *colB = new ColliderBehaviour();
		if (!colB->Initialize(terrainEnt))
		{
			ErrMsg("Failed to initialize cave floor terrain!");
			return false;
		}

		Collisions::Terrain *terrainCol = new Collisions::Terrain(bounds.Center, bounds.Extents, content->GetHeightMap("HM_CaveHeightmap"));
		colB->SetCollider(terrainCol);

		_terrainBehaviour = colB;
		_terrain = dynamic_cast<const Collisions::Terrain *>(colB->GetCollider());
	}

	// Create Player
	if (!_sceneHolder.GetEntityByName("Player Holder"))
	{
		Entity *player = nullptr;
		if (!CreatePlayerEntity(&player))
		{
			ErrMsg("Failed to create player entity!");
			return false;
		}

		PlayerCutsceneBehaviour *pcb = new PlayerCutsceneBehaviour();
		if (!pcb->Initialize(player, "Player Cutscene Controller"))
		{
			ErrMsg("Failed to initialize player cutscene behaviour!");
			return false;
		}

		_player->SetSerialization(false);
	}

	std::ifstream file(std::format("Content/Saves/{}.txt", "StartCutsceneSave"));
	uintmax_t fileSize = 0;
	if (file.is_open())
	{
		fileSize = std::filesystem::file_size(std::format("Content/Saves/{}.txt", "StartCutsceneSave"));
	}
	
	if (fileSize == 0) // Load a new game
	{
		std::ifstream newGameFile(std::format("Content/Saves/{}.txt", "MapSave"));
		if (newGameFile.is_open())
		{
			fileSize = std::filesystem::file_size(std::format("Content/Saves/{}.txt", "MapSave"));
			_saveFile = "MapSave";
		}
	}

	if (fileSize > 0)
	{
		// Deserialize scene
		if (!Deserialize())
		{
			ErrMsg("Could not load in saved Entities");
			return false;
		}
	}
	else
	{
		ErrMsg("There is no file to load from!");
		return false;
	}


#ifdef DEBUG_BUILD
	// Create transform gizmo controller
	{
		Entity *ent = nullptr;
		if (!CreateEntity(&ent, "Transform Controller", BoundingOrientedBox({ 0,0,0 }, { 0.01f, 0.01f, 0.01f }, { 0,0,0,1 }), false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		ent->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		_transformGizmo = new TransformGizmoBehaviour();
		if (!_transformGizmo->Initialize(ent))
		{
			ErrMsg("Failed to initialize transform gizmo behaviour!");
			return false;
		}
		ent->SetSerialization(false);
	}
#endif

	_collisionHandler.Initialize(this);

	_initialized = true;

	// Set view to player camera
	Entity *cam = _sceneHolder.GetEntityByName("playerCamera");
	CameraBehaviour *camBehaviour;
	if (cam->GetBehaviourByType<CameraBehaviour>(camBehaviour))
		SetViewCamera(camBehaviour);
	else
	{
		ErrMsg("Failed to get player camera behaviour!");
		return false;
	}

	return true;
}
bool Scene::InitializeGame(ID3D11Device *device, ID3D11DeviceContext *context, 
	Game *game, Content *content, Graphics *graphics, float gameVolume, const std::string &saveFile)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(86494680, "Scene Initialize");
#endif

	if (_initialized)
		return false;

	_game = game;
	_device = device;
	_context = context;
	_content = content;
	_graphics = graphics;
	_saveFile = saveFile;

	if (!_soundEngine.Initialize(
		AudioEngine_EnvironmentalReverb | AudioEngine_ReverbUseFilters /*| AudioEngine_UseMasteringLimiter*/,
		Reverb_Cave, gameVolume))
	{
		ErrMsg("Failed to initialize sound engine!");
		return -1;
	}

	// Create scene content holder
	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(600.0f, 110.0f, 600.0f));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	if (!_pointlights->Initialize(device, 256))
	{
		ErrMsg("Failed to initialize pointlight collection!");
		return false;
	}

	if (!_spotlights->Initialize(device, 512))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}


	// Create global selection marker
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_WireframeCube");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_Red");
		mat.ambientID = _content->GetTextureID("Tex_Red");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Selection Marker"))
		{
			ErrMsg("Failed to initialize selection marker object!");
			return false;
		}

		MeshBehaviour *mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to selection marker object!");
			return false;
		}

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global transform gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("TranslationGizmo");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_TransformGizmo");
		mat.ambientID = _content->GetTextureID("Tex_TransformGizmo");
		mat.samplerID = _content->GetSamplerID("SS_Point");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Transform Gizmo"))
		{
			ErrMsg("Failed to initialize transform gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		/*MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to initialize transform gizmo object!");
			return false;
		}*/

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global pointer gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_Sphere");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_White");
		mat.ambientID = _content->GetTextureID("Tex_White");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Pointer Gizmo"))
		{
			ErrMsg("Failed to initialize pointer gizmo object!");
			return false;
		}

		MeshBehaviour *mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to pointer gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.1f, 0.1f, 0.1f });

		_globalEntities.push_back(std::move(entPtr));
	}


#ifdef DEBUG_BUILD
	// Create debug player
	{
		Entity *player = nullptr;
		if (!CreateEntity(&player, "DebugPlayer", { {}, {.1f,.1f,.1f}, {0,0,0,1} }, false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}
		player->SetSerialization(false);

		DebugPlayerBehaviour *behaviour = new DebugPlayerBehaviour();
		if (!behaviour->Initialize(player))
		{
			ErrMsg("Failed to initialize debug player behaviour!");
			return false;
		}

		_debugPlayer = behaviour;
	}

	// Create sound templates
	{
		for (int i = 1; i < 16; i++)
		{
			std::string fileName = "Drop" + std::to_string(i);
			Entity* emitter = nullptr;
			if (!CreateSoundEmitterEntity(&emitter, "WaterTemplate" + std::to_string(i), fileName, false, 0.5f))
			{
				ErrMsg("Failed to create template sound emitter entity for sound " + fileName);
				return false;
			}
			emitter->GetTransform()->SetPosition({ float(i * 15) - 50, 110, -40 }, World);
			emitter->SetSerialization(false);
		}

		for (int i = 1; i < 19; i++)
		{
			std::string fileName = "stone_" + std::to_string(i);
			Entity* emitter = nullptr;
			if (!CreateSoundEmitterEntity(&emitter, "StoneTemplate" + std::to_string(i), fileName, false, 0.5f, 25.0f, 1.0f, 60.0f, 120.0f))
			{
				ErrMsg("Failed to create template sound emitter entity for sound " + fileName);
				return false;
			}
			emitter->GetTransform()->SetPosition({ float(i * 15) - 50, 110, 40 }, World);
			emitter->SetSerialization(false);
		}
	}
#endif


	std::ifstream gameFile(std::format("Content/Saves/{}.txt", "GameSave"));
	UINT fileSize = 0;
	if (gameFile.is_open()) // If a current game exists, load current game
	{
		fileSize = std::filesystem::file_size(std::format("Content/Saves/{}.txt", "GameSave"));
		_saveFile = "GameSave";
	}
	
	if (fileSize == 0) // Load a new game
	{
		std::ifstream newGameFile(std::format("Content/Saves/{}.txt", "MapSave"));
		if (newGameFile.is_open())
		{
			fileSize = std::filesystem::file_size(std::format("Content/Saves/{}.txt", "MapSave"));
			_saveFile = "MapSave";
		}
	}

	if (fileSize > 0)
	{
		// Deserialize scene
		if (!Deserialize())
		{
			ErrMsg("Could not load in saved Entities");
			return false;
		}
	}
	else
	{
		ErrMsg("There is no file to load from!");
		return false;
	}

#ifndef EDIT_MODE
	// Create Player
	if (!_sceneHolder.GetEntityByName("Player Holder"))
	{
		Entity *player = nullptr;
		if (!CreatePlayerEntity(&player))
		{
			ErrMsg("Failed to create player entity!");
			return false;
		}
	}
	
	// Create Monster
	bool spawnMonster = true;
	Entity *monsterEnt = _sceneHolder.GetEntityByName("Monster");
	if (monsterEnt)
	{
		MonsterBehaviour *monster = nullptr;
		if (monsterEnt->GetBehaviourByType<MonsterBehaviour>(monster))
		{
			spawnMonster = false;
		}
		else if (!_sceneHolder.RemoveEntity(monsterEnt))
		{
			ErrMsg("Failed to remove monster entity!");
			return false;
		}
	}

	if (spawnMonster)
	{
		Entity *monster = nullptr;
		if (!CreateMonsterEntity(&monster))
		{
			ErrMsg("Failed to create monster entity!");
			return false;
		}

		monster->GetTransform()->SetScale({ 0.2f, 0.2f, 0.2f });
	}

#ifdef DISABLE_MONSTER
	monsterEnt = _sceneHolder.GetEntityByName("Monster");
	monsterEnt->Disable();
#endif
#endif

	// Terrian Colldier
	{
		// Floor
		BoundingOrientedBox bounds = { {0, 0.444183499f, 0}, {0.5f, 0.444177479f, 0.5f}, {0, 0, 0, 1} };

		Entity *terrainEnt = nullptr;
		if (!CreateEntity(&terrainEnt, "Terrain Floor", bounds, true))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		terrainEnt->SetSerialization(false);
		terrainEnt->GetTransform()->SetPosition({ -5.505f, -33.125f, 0.0f });
		terrainEnt->GetTransform()->SetScale({ 600.0f, 112.5f, 600.0f });

		ColliderBehaviour *colB = new ColliderBehaviour();
		if (!colB->Initialize(terrainEnt))
		{
			ErrMsg("Failed to initialize cave floor terrain!");
			return false;
		}

		Collisions::Terrain *terrainCol = new Collisions::Terrain(bounds.Center, bounds.Extents, content->GetHeightMap("HM_CaveHeightmap"));
		colB->SetCollider(terrainCol);

		_terrainBehaviour = colB;
		_terrain = dynamic_cast<const Collisions::Terrain*>(colB->GetCollider());

		// Roof
		if (!CreateEntity(&terrainEnt, "Terrain Roof", bounds, true))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		terrainEnt->SetSerialization(false);
		terrainEnt->GetTransform()->SetPosition({ -5.0f, -33.125f, 0.0f });
		terrainEnt->GetTransform()->SetScale({ 600.0f, 112.5f, 600.0f });

		colB = new ColliderBehaviour();
		if (!colB->Initialize(terrainEnt))
		{
			ErrMsg("Failed to initialize cave roof terrain!");
			return false;
		}

		colB->SetCollider(new Collisions::Terrain(bounds.Center, bounds.Extents, content->GetHeightMap("HM_CaveRoofHeightmap"), Collisions::NULL_TAG, true));

		// Walls
		if (!CreateEntity(&terrainEnt, "Terrain Walls", bounds, true))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		terrainEnt->SetSerialization(false);
		terrainEnt->GetTransform()->SetPosition({ -5.64f, -33.125f, 0.0f });
		terrainEnt->GetTransform()->SetScale({ 601.0f, 112.5f, 600.0f });

		colB = new ColliderBehaviour();
		if (!colB->Initialize(terrainEnt))
		{
			ErrMsg("Failed to initialize cave roof terrain!");
			return false;
		}

		colB->SetCollider(new Collisions::Terrain(bounds.Center, bounds.Extents, content->GetHeightMap("HM_CaveWallsHeightmap"), Collisions::NULL_TAG, false, true));


#ifdef DEBUG_BUILD
		// Maxwell (for testing purposes)
		{
			UINT meshID = content->GetMeshID("Mesh_Maxwell");
			Material mat = Material::MakeMat(
				content->GetTextureID("Tex_Maxwell"),
				CONTENT_NULL,
				CONTENT_NULL,
				CONTENT_NULL,
				content->GetTextureID("Tex_AmbientBright")
			);

			Entity *maxwell = nullptr;
			if (!CreateMeshEntity(&maxwell, "Maxwell [labrat]", meshID, mat))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			maxwell->SetSerialization(false);

			BoundingOrientedBox bounds = content->GetMesh(meshID)->GetBoundingOrientedBox();
			ColliderBehaviour *colB = new ColliderBehaviour();
			if (!colB->Initialize(maxwell))
			{
				ErrMsg("Failed to initialize Maxwell the labrat!");
				return false;
			}

			colB->SetCollider(new Collisions::Sphere(bounds.Center, bounds.Extents.x, Collisions::OBJECT_TAG));

			ExampleCollisionBehaviour *behaviour = new ExampleCollisionBehaviour();
			if (!behaviour->Initialize(maxwell))
			{
				ErrMsg("Failed to initialize example behaviour!");
				return false;
			}

			meshID = content->GetMeshID("Mesh_Whiskers");
			mat.textureID = content->GetTextureID("Tex_Whiskers");
			mat.specularID = content->GetTextureMapID("TexMap_Black_Specular");

			Entity *whiskers = nullptr;
			if (!CreateMeshEntity(&whiskers, "Whiskers", meshID, mat, true))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			whiskers->SetSerialization(false);
			whiskers->SetParent(maxwell);
		}
#endif
	}

	CreateAnimationCamera();

#ifdef DEBUG_BUILD
	// Create disjointed graph starter node
	if (_graphManager.GetNodeCount() <= 0)
	{
		Entity *ent = nullptr;
		GraphNodeBehaviour *node = nullptr;
		if (!CreateGraphNodeEntity(&ent, &node, { 0, 0, 0 }))
		{
			ErrMsg("Failed to create graph node entity!");
			return false;
		}
	}

	// Create transform gizmo controller
	{
		Entity *ent = nullptr;
		if (!CreateEntity(&ent, "Transform Controller", BoundingOrientedBox({ 0,0,0 }, { 0.01f, 0.01f, 0.01f }, { 0,0,0,1 }), false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		ent->SetSerialization(false);
		ent->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		_transformGizmo = new TransformGizmoBehaviour();
		if (!_transformGizmo->Initialize(ent))
		{
			ErrMsg("Failed to initialize transform gizmo behaviour!");
			return false;
		}
	}
#endif

	_collisionHandler.Initialize(this);
	_initialized = true;

#ifndef DEBUG_BUILD
	// Set view to player camera
	Entity *cam = _sceneHolder.GetEntityByName("playerCamera");
	CameraBehaviour *camBehaviour;
	if (cam->GetBehaviourByType<CameraBehaviour>(camBehaviour))
	{
		SetViewCamera(camBehaviour);
	}
#endif

	return true;
}
bool Scene::InitializeCred(ID3D11Device *device, ID3D11DeviceContext *context,
	Game *game, Content *content, Graphics *graphics, float gameVolume, const std::string &saveFile)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(86494680, "Scene Initialize");
#endif

	if (_initialized)
		return false;

	_game = game;
	_device = device;
	_context = context;
	_content = content;
	_graphics = graphics;
	_saveFile = saveFile;

	if (!_soundEngine.Initialize(
		AudioEngine_EnvironmentalReverb | AudioEngine_ReverbUseFilters /*| AudioEngine_UseMasteringLimiter*/,
		Reverb_Cave, gameVolume))
	{
		ErrMsg("Failed to initialize sound engine!");
		return false;
	}

	// Create scene content holder
	constexpr BoundingBox sceneBounds = BoundingBox(XMFLOAT3(0, 0, 0), XMFLOAT3(50.0f, 50.0f, 50.0f));
	if (!_sceneHolder.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize scene holder!");
		return false;
	}

	if (!_pointlights->Initialize(device, 256))
	{
		ErrMsg("Failed to initialize pointlight collection!");
		return false;
	}

	if (!_spotlights->Initialize(device, 512))
	{
		ErrMsg("Failed to initialize spotlight collection!");
		return false;
	}

#ifdef DEBUG_BUILD
	// Create global selection marker
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_WireframeCube");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_Red");
		mat.ambientID = _content->GetTextureID("Tex_Red");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Selection Marker"))
		{
			ErrMsg("Failed to initialize selection marker object!");
			return false;
		}

		MeshBehaviour *mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to selection marker object!");
			return false;
		}

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global transform gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("TranslationGizmo");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_TransformGizmo");
		mat.ambientID = _content->GetTextureID("Tex_TransformGizmo");
		mat.samplerID = _content->GetSamplerID("SS_Point");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Transform Gizmo"))
		{
			ErrMsg("Failed to initialize transform gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		/*MeshBehaviour* mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
		ErrMsg("Failed to initialize transform gizmo object!");
		return false;
		}*/

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create global pointer gizmo
	{
		BoundingOrientedBox defaultBox = { { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0, 1 } };
		UINT meshID = _content->GetMeshID("Mesh_Sphere");
		Material mat;
		mat.textureID = _content->GetTextureID("Tex_White");
		mat.ambientID = _content->GetTextureID("Tex_White");
		mat.vsID = _content->GetShaderID("VS_Geometry");

		std::unique_ptr<Entity> entPtr = std::make_unique<Entity>(-1, defaultBox);
		if (!entPtr->Initialize(_device, this, "Pointer Gizmo"))
		{
			ErrMsg("Failed to initialize pointer gizmo object!");
			return false;
		}

		MeshBehaviour *mesh = new MeshBehaviour(defaultBox, meshID, &mat, false, false);

		if (!mesh->Initialize(entPtr.get()))
		{
			ErrMsg("Failed to bind mesh to pointer gizmo object!");
			return false;
		}
		entPtr.get()->GetTransform()->SetScale({ 0.1f, 0.1f, 0.1f });

		_globalEntities.push_back(std::move(entPtr));
	}

	// Create debug player
	{
		Entity *player = nullptr;
		if (!CreateEntity(&player, "DebugPlayer", { {}, {.1f,.1f,.1f}, {0,0,0,1} }, false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		DebugPlayerBehaviour *behaviour = new DebugPlayerBehaviour();
		if (!behaviour->Initialize(player))
		{
			ErrMsg("Failed to initialize debug player behaviour!");
			return false;
		}

		_debugPlayer = behaviour;
		behaviour->SetEnabled(false);
	}
#endif

	{
		// Credits mesh
		{
			UINT meshID = content->GetMeshID("Mesh_Plane");
			Material mat = {};
			mat.textureID = content->GetTextureID("Tex_Credit_Logo_Texture");
			mat.ambientID = content->GetTextureID("Tex_White");
			mat.vsID = content->GetShaderID("VS_Geometry");

			Entity *credit;
			if (!CreateMeshEntity(&credit, "Credits Mesh", meshID, mat))
			{
				ErrMsg("Failed to create credit billboard mesh entity!");
				return false;
			}

			const float zLength = 6.0f;
			const float scale = zLength * 0.70625;
			credit->GetTransform()->SetPosition({ 0.0f, 0.0f, zLength });
			credit->GetTransform()->Rotate({ 90.0f * DEG_TO_RAD, 0.0f, 0.0f, 1.0f });
			credit->GetTransform()->SetScale({ -1.6f * scale, 1.0f, 0.9f * scale });
		}

		// Credits behaviour
		{
			Entity *ent;
			if (!CreateEntity(&ent, "Credits Manager", { {}, {.1f,.1f,.1f}, {0,0,0,1} }, false))
			{
				ErrMsg("Failed to create Credits Manager!");
				return false;
			}

			CreditsBehaviour *credits = new CreditsBehaviour();
			if (!credits->Initialize(ent))
			{
				ErrMsg("Failed to initialize credits behaviour!");
				return false;
			}
			credits->SetEnabled(false);
		}

		// Maxwell
		{
			UINT meshID = content->GetMeshID("Mesh_Maxwell");
			Material mat = Material::MakeMat(
				content->GetTextureID("Tex_Maxwell"),
				CONTENT_NULL,
				content->GetTextureMapID("TexMap_Default_Specular"),
				CONTENT_NULL,
				content->GetTextureID("Tex_White")
			);
			mat.vsID = content->GetShaderID("VS_Geometry");

			Entity *maxwell = nullptr;
			if (!CreateMeshEntity(&maxwell, "Maxwell", meshID, mat))
			{
				ErrMsg("Failed to create object!");
				return false;
			}

			maxwell->GetTransform()->SetPosition({ 0.0f, -0.5f, 3.0f });

			meshID = content->GetMeshID("Mesh_Whiskers");
			mat.textureID = content->GetTextureID("Tex_Whiskers");
			mat.specularID = content->GetTextureMapID("TexMap_Black_Specular");

			Entity *whiskers = nullptr;
			if (!CreateMeshEntity(&whiskers, "Whiskers", meshID, mat, true))
			{
				ErrMsg("Failed to create object!");
				return false;
			}
			whiskers->SetParent(maxwell);

			ExampleBehaviour *behaviour = new ExampleBehaviour();
			if (!behaviour->Initialize(maxwell))
			{
				ErrMsg("Failed to initialize example behaviour!");
				return false;
			}

			maxwell->Disable();
		}
	}

#ifdef DEBUG_BUILD
	// Create transform gizmo controller
	{
		Entity *ent = nullptr;
		if (!CreateEntity(&ent, "Transform Controller", BoundingOrientedBox({ 0,0,0 }, { 0.01f, 0.01f, 0.01f }, { 0,0,0,1 }), false))
		{
			ErrMsg("Failed to create object!");
			return false;
		}

		ent->GetTransform()->SetScale({ 0.5f, 0.5f, 0.5f });

		_transformGizmo = new TransformGizmoBehaviour();
		if (!_transformGizmo->Initialize(ent))
		{
			ErrMsg("Failed to initialize transform gizmo behaviour!");
			return false;
		}
	}
#endif

	_collisionHandler.Initialize(this);

	_initialized = true;
	return true;
}

bool Scene::IsInitialized() const
{
	return _initialized;
}
void Scene::SetInitialized(bool state)
{
	_initialized = state;
}
#pragma endregion

#pragma region Update
bool Scene::Update(Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(36884184, "Scene Update");
#endif

	if (!_initialized)
		return false;

	if (BindingCollection::IsTriggered(InputBindings::InputAction::Save))
	{
		std::string code;
		if (!Serialize(&code))
		{
			ErrMsg("Failed to serialize scene!");
			return false;
		}
	}

	_input = &input;

	// Update entities
	const UINT entityCount = _sceneHolder.GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		Entity *ent = _sceneHolder.GetEntity(i);

		if (!ent)
			continue;

		if (ent->IsRemoved())
			continue;

		if (!ent->InitialUpdate(time, input))
		{
			ErrMsg(std::format("Failed to update entity '{}'!", ent->GetName()));
			return false;
		}
	}

#ifdef PARALLEL_UPDATE
	bool parallelFailed = false;
	int entCount = static_cast<int>(entityCount);

#pragma omp parallel for num_threads(PARALLEL_THREADS)
	for (int i = 0; i < entCount; i++)
	{
		if (!parallelFailed)
		{
			Entity *ent = _sceneHolder.GetEntity(i);

			if (!ent)
				continue;

			if (ent->IsRemoved())
				continue;

			if (!ent->InitialParallelUpdate(time, input))
			{
#pragma omp critical
				{
					ErrMsg(std::format("Failed to update entity '{}' in parallel!", ent->GetName()));
					parallelFailed = true;
				}
			}
		}
	}

	if (parallelFailed)
	{
		ErrMsg("Parallel update failed!");
		return false;
	}
#endif

	if (!_collisionHandler.CheckCollisions(time, this, _context))
	{
		ErrMsg("Failed to performed collision checks!");
		return false;
	}

	if (!UpdateSound())
	{
		ErrMsg("Failed to update sound!");
		return false;
	}

	if (!_graphics->SetSpotlightCollection(_spotlights.get()))
	{
		ErrMsg("Failed to set spotlight collection!");
		return false;
	}

	if (!_graphics->SetPointlightCollection(_pointlights.get()))
	{
		ErrMsg("Failed to set pointlight collection!");
		return false;
	}

	if (!_viewCamera->UpdateBuffers())
	{
		ErrMsg("Failed to update view camera's buffers!");
		return false;
	}

	return true;
}
bool Scene::LateUpdate(Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(114004646, "Scene Late Update");
#endif

	if (!_initialized)
		return false;

	// Late update entities
	const UINT entityCount = _sceneHolder.GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		Entity *ent = _sceneHolder.GetEntity(i);

		if (!ent)
			continue;

		if (ent->IsRemoved())
			continue;

		if (!ent->InitialLateUpdate(time, input))
		{
			ErrMsg(std::format("Failed to late update entity '{}'!", ent->GetName()));
			return false;
		}
	}

	// Update volume tree & insert new entities
	if (!_sceneHolder.Update())
	{
		ErrMsg("Failed to update scene holder!");
		return false;
	}

	// Update light collections
	if (!_spotlights->UpdateBuffers(_device, _context))
	{
		ErrMsg("Failed to update spotlight buffers!");
		return false;
	}

	if (!_pointlights->UpdateBuffers(_device, _context))
	{
		ErrMsg("Failed to update pointlight buffers!");
		return false;
	}

	_timelineManager.Update(time);

	return true;
}
bool Scene::FixedUpdate(const float &deltaTime, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(471684088, "Scene Fixed Update");
#endif

	if (!_initialized)
		return false;

	const UINT entCount = _sceneHolder.GetEntityCount();
	for (UINT i = 0; i < entCount; i++)
	{
		Entity *ent = _sceneHolder.GetEntity(i);

		if (!ent)
			continue;

		if (ent->IsRemoved())
			continue;

		if (!ent->InitialFixedUpdate(deltaTime, input))
		{
			ErrMsg("Failed to update entity at fixed step!");
			return false;
		}

		if (ent->GetTransform()->IsScenePosDirty())
		{
			if (!_sceneHolder.UpdateEntityPosition(ent))
			{
				ErrMsg("Failed to update entity transform!");
				return false;
			}
		}
	}

	return true;
}

bool Scene::UpdateSound()
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(714826648, "Scene Update Sound");
#endif

	if (!_soundEngine.Update())
	{
		return false;
	}

	Entity* entity = _sceneHolder.GetEntityByName("Maxwell");
	if (entity)
	{
		if (!entity->IsRemoved() && entity->IsEnabled())
		{
			SoundBehaviour* soundBehaviour;
			if (entity->GetBehaviourByType<SoundBehaviour>(soundBehaviour))
				soundBehaviour->Play();
		}
	}

	return true;
}
#pragma endregion

#pragma region Render
bool Scene::Render(Time &time, const Input &input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(781662826, "Scene Render");
#endif

	if (!_initialized)
		return false;

	if (!_graphics->SetCamera(_viewCamera))
	{
		ErrMsg("Failed to set camera!");
		return false;
	}

	GetDebugDrawer()->SetCamera(_viewCamera);

	std::vector<Entity *> entitiesToRender;
	entitiesToRender.reserve(_viewCamera->GetCullCount());

	union {
		BoundingFrustum frustum = {};
		BoundingOrientedBox box;
	} view;
	bool isCameraOrtho = _viewCamera->GetOrtho();

	time.TakeSnapshot("FrustumCull");
	if (isCameraOrtho)
	{
		if (!_viewCamera->StoreBounds(view.box, false))
		{
			ErrMsg("Failed to store camera box!");
			return false;
		}

		if (!_sceneHolder.BoxCull(view.box, entitiesToRender))
		{
			ErrMsg("Failed to perform box culling!");
			return false;
		}
	}
	else
	{
		if (!_viewCamera->StoreBounds(view.frustum, false))

		{
			ErrMsg("Failed to store camera frustum!");
			return false;
		}

		if (!_sceneHolder.FrustumCull(view.frustum, entitiesToRender))
		{
			ErrMsg("Failed to perform frustum culling!");
			return false;
		}
	}

	for (UINT i = 0; i < entitiesToRender.size(); i++)
	{
		Entity *ent = entitiesToRender[i];

		// TODO: Figure out why entitiesToRender sometimes contains mangled pointers immediately after deleting a large amount of entities

		CamRenderQueuer queuer = { _viewCamera };
		if (!ent->InitialRender(queuer, _viewCamera->GetRendererInfo()))
		{
			ErrMsg("Failed to render entity!");
			return false;
		}
	}
	time.TakeSnapshot("FrustumCull");

	const int spotlightCount = static_cast<int>(_spotlights->GetNrOfLights());
	const int pointlightCount = static_cast<int>(_pointlights->GetNrOfLights());

#pragma warning(disable: 6993)
#pragma omp parallel num_threads(PARALLEL_THREADS)
	{
#pragma omp for nowait
		for (int i = 0; i < spotlightCount; i++)
		{
#ifdef PIX_TIMELINING
			PIXScopedEvent(681468268, std::format("Cull Spotlight #{}", i).c_str());
#endif
			if (!_spotlights.get()->GetLightBehaviour(i)->DoUpdate())
				continue;

			CameraBehaviour *spotlightCamera = _spotlights.get()->GetLightBehaviour(i)->GetShadowCamera();

			std::vector<Entity *> entitiesToCastShadows;
			entitiesToCastShadows.reserve(spotlightCamera->GetCullCount());

			bool isSpotlightOrtho = spotlightCamera->GetOrtho();

			bool intersectResult = false;
			if (isSpotlightOrtho)
			{
				BoundingOrientedBox lightBounds;
				if (!spotlightCamera->StoreBounds(lightBounds, false))
				{
					ErrMsg("Failed to store spotlight camera oriented box!");
					continue;
				}

				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(lightBounds);
				else				intersectResult = intersectResult || view.frustum.Intersects(lightBounds);

				if (!intersectResult)
				{ // Skip rendering if the bounds don't intersect
					_spotlights->SetLightEnabled(i, false);
					continue;
				}

				if (!_sceneHolder.BoxCull(lightBounds, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform box culling for spotlight #{}!", i));
					continue;
				}
			}
			else
			{
				BoundingFrustum lightBounds;
				if (!spotlightCamera->StoreBounds(lightBounds, false))
				{
					ErrMsg("Failed to store spotlight camera frustum!");
					continue;
				}

				if (isCameraOrtho)	intersectResult = intersectResult || view.box.Intersects(lightBounds);
				else				intersectResult = intersectResult || view.frustum.Intersects(lightBounds);

				if (!intersectResult)
				{ // Skip rendering if the bounds don't intersect
					_spotlights->SetLightEnabled(i, false);
					continue;
				}
				_spotlights->SetLightEnabled(i, true);

				if (!_sceneHolder.FrustumCull(lightBounds, entitiesToCastShadows))
				{
					ErrMsg(std::format("Failed to perform frustum culling for spotlight #{}!", i));
					continue;
				}
			}

			for (Entity *ent : entitiesToCastShadows)
			{
				CamRenderQueuer queuer = { spotlightCamera };
				if (!ent->InitialRender(queuer, spotlightCamera->GetRendererInfo()))
				{
					ErrMsg(std::format("Failed to render entity for spotlight #{}!", i));
					break;
				}
			}
		}

#pragma omp for nowait
		for (int i = 0; i < pointlightCount; i++)
		{
#ifdef PIX_TIMELINING
			PIXScopedEvent(681468268, std::format("Cull Pointlight #{}", i).c_str());
#endif
			if (!_pointlights.get()->GetLightBehaviour(i)->DoUpdate())
				continue;

			CameraCubeBehaviour *pointlightCamera = _pointlights.get()->GetLightBehaviour(i)->GetShadowCameraCube();

			std::vector<Entity *> entitiesToCastShadows;
			entitiesToCastShadows.reserve(pointlightCamera->GetCullCount());


			BoundingBox pointlightBox;
			if (!pointlightCamera->StoreBounds(pointlightBox))
			{
				ErrMsg("Failed to store pointlight bounds!");
				continue;
			}

			bool intersectBoxResult = false;
			if (isCameraOrtho)	intersectBoxResult = intersectBoxResult || view.box.Intersects(pointlightBox);
			else				intersectBoxResult = intersectBoxResult || view.frustum.Intersects(pointlightBox);

			if (!intersectBoxResult)
			{ // Skip rendering if the camera frustum doesn't intersect the point light bounds
				for (int j = 0; j < 6; j++)
					_pointlights->SetLightEnabled(i, j, false);
				continue;
			}

			if (!_sceneHolder.BoxCull(pointlightBox, entitiesToCastShadows))
			{
				ErrMsg(std::format("Failed to perform box culling for pointlight #{}!", i));
				continue;
			}

			pointlightCamera->SetCullCount(static_cast<UINT>(entitiesToCastShadows.size()));

			for (UINT j = 0; j < 6; j++)

			{
				BoundingFrustum pointlightFrustum;
				if (!pointlightCamera->StoreBounds(pointlightFrustum, j))

				{
					ErrMsg("Failed to store pointlight camera frustum!");
					continue;
				}

				bool intersectFrustumResult = false;
				if (isCameraOrtho)	intersectFrustumResult = intersectFrustumResult || view.box.Intersects(pointlightFrustum);
				else				intersectFrustumResult = intersectFrustumResult || view.frustum.Intersects(pointlightFrustum);

				if (!intersectFrustumResult)
				{ // Skip rendering if the frustums don't intersect
					_pointlights->SetLightEnabled(i, j, false);
					continue;
				}
				_pointlights->SetLightEnabled(i, j, true);

				for (Entity *ent : entitiesToCastShadows)


				{
					BoundingOrientedBox entBounds;
					ent->StoreEntityBounds(entBounds);

					if (!pointlightFrustum.Intersects(entBounds))
						continue;

					CubeRenderQueuer queuer = { pointlightCamera, j };
					if (!ent->InitialRender(queuer, pointlightCamera->GetRendererInfo()))
					{
						ErrMsg(std::format("Failed to render entity for pointlight #{} camera #{}!", i, j));
						break;
					}
				}
			}
		}
	}
#pragma warning(default: 6993)

	// Calculate light tiles
	{
#ifdef PIX_TIMELINING
		PIXScopedEvent(761426246, "Calculate Light Tiles");
#endif

		_graphics->ResetLightGrid(); // Clear light grid buffer
		const UINT lightTileCount = LIGHT_GRID_RES * LIGHT_GRID_RES;

		const BoundingFrustum *lightGridFrustums = _viewCamera->GetLightGridFrustums();
		if (!lightGridFrustums)
		{
			ErrMsg("Failed to get light grid frustums!");
			return false;
		}

		// Draw light tiles for debugging
		if (false)
		{
			std::vector<DebugDraw::Line> lines;
			lines.reserve(12);

			for (UINT i = 0; i < 12; i++)
			{
				DebugDraw::Line line{};

				line.start.color = { 1.0f, 0.0f, 0.0f, 1.0f };
				line.start.size = 0.01f;

				line.end.color = { 1.0f, 0.0f, 0.0f, 1.0f };
				line.end.size = 0.01f;

				lines.push_back(line);
			}

			//     Near    Far
			//    0----1  4----5
			//    |    |  |    |
			//    |    |  |    |
			//    3----2  7----6

			XMFLOAT3 corners[8];
			for (UINT i = 0; i < lightTileCount; i++)
			{
				lightGridFrustums[i].GetCorners(corners);

				lines[0].start.position = corners[0];
				lines[0].end.position = corners[1];

				lines[1].start.position = corners[1];
				lines[1].end.position = corners[2];

				lines[2].start.position = corners[2];
				lines[2].end.position = corners[3];

				lines[3].start.position = corners[3];
				lines[3].end.position = corners[0];

				lines[4].start.position = corners[4];
				lines[4].end.position = corners[5];

				lines[5].start.position = corners[5];
				lines[5].end.position = corners[6];

				lines[6].start.position = corners[6];
				lines[6].end.position = corners[7];

				lines[7].start.position = corners[7];
				lines[7].end.position = corners[4];

				lines[8].start.position = corners[0];
				lines[8].end.position = corners[4];

				lines[9].start.position = corners[1];
				lines[9].end.position = corners[5];

				lines[10].start.position = corners[2];
				lines[10].end.position = corners[6];

				lines[11].start.position = corners[3];
				lines[11].end.position = corners[7];

				GetDebugDrawer()->DrawLines(lines, false);
			}
		}

		XMFLOAT3A cameraPos = _viewCamera->GetTransform()->GetPosition(World);

		const int spotlightCount = static_cast<int>(_spotlights->GetNrOfLights());
		const int pointlightCount = static_cast<int>(_pointlights->GetNrOfLights());
		const int simpleSpotlightCount = static_cast<int>(_spotlights->GetNrOfSimpleLights());
		const int simplePointlightCount = static_cast<int>(_pointlights->GetNrOfSimpleLights());

#pragma warning(disable: 6993)
#pragma omp parallel num_threads(PARALLEL_THREADS)
		{
#pragma omp for nowait
			for (int i = 0; i < spotlightCount; i++)
			{
#ifdef PIX_TIMELINING
				PIXScopedEvent(681442451, std::format("Spotlight #{}", i).c_str());
#endif

				if (!_spotlights->GetLightEnabled(i))
					continue;

				SpotLightBehaviour *light = _spotlights->GetLightBehaviour(i);

				bool skipIntersectionTests = light->ContainsPoint(cameraPos);

				for (UINT j = 0; j < lightTileCount; j++)
				{
					if (!skipIntersectionTests)
					{
						if (!light->IntersectsLightTile(lightGridFrustums[j]))
							continue;
					}

#pragma omp critical
					{
						_graphics->AddLightToTile(j, i, SPOTLIGHT);
					}
				}
			}

#pragma omp for nowait
			for (int i = 0; i < pointlightCount; i++)
			{
#ifdef PIX_TIMELINING
				PIXScopedEvent(276726455, std::format("Pointlight #{}", i).c_str());
#endif

				bool skip = true;
				for (UINT j = 0; j < 6; j++)
				{
					// If no faces are enabled, the entire light can be skipped
					if (_pointlights->GetLightEnabled(i, j))
					{
						skip = false;
						break;
					}
				}

				if (skip)
					continue;

				PointLightBehaviour *light = _pointlights->GetLightBehaviour(i);

				for (UINT j = 0; j < lightTileCount; j++)
				{
					if (!light->IntersectsLightTile(lightGridFrustums[j]))
						continue;

					for (UINT k = 0; k < 6; k++)
					{
						if (!light->IntersectsLightTile(k, lightGridFrustums[j]))
							continue;

#pragma omp critical
						{
							_graphics->AddLightToTile(j, i * 6 + k, POINTLIGHT);
						}
					}
				}
			}

#pragma omp for nowait
			for (int i = 0; i < simpleSpotlightCount; i++)
			{
#ifdef PIX_TIMELINING
				PIXScopedEvent(671683654, std::format("Simple Spotlight #{}", i).c_str());
#endif

				if (!_spotlights->GetSimpleLightEnabled(i))
					continue;

				SimpleSpotLightBehaviour *light = _spotlights->GetSimpleLightBehaviour(i);

				bool skipIntersectionTests = light->ContainsPoint(cameraPos);

				for (UINT j = 0; j < lightTileCount; j++)
				{
					if (!skipIntersectionTests)
					{
						if (!light->IntersectsLightTile(lightGridFrustums[j]))
							continue;
					}

#pragma omp critical
					{
						_graphics->AddLightToTile(j, i, SIMPLE_SPOTLIGHT);
					}
				}
			}

#pragma omp for nowait
			for (int i = 0; i < simplePointlightCount; i++)
			{
#ifdef PIX_TIMELINING
				PIXScopedEvent(68465351, std::format("Simple Pointlight #{}", i).c_str());
#endif

				if (!_pointlights->GetSimpleLightEnabled(i))
					continue;

				SimplePointLightBehaviour *light = _pointlights->GetSimpleLightBehaviour(i);

				bool skipIntersectionTests = light->ContainsPoint(cameraPos);

				for (UINT j = 0; j < lightTileCount; j++)
				{
					if (!skipIntersectionTests)
					{
						if (!light->IntersectsLightTile(lightGridFrustums[j]))
							continue;
					}

#pragma omp critical
					{
						_graphics->AddLightToTile(j, i, SIMPLE_POINTLIGHT);
					}
				}
			}
		}
#pragma warning(default: 6993)
	}

#ifdef DEBUG_BUILD
	if (_debugPlayer)
	{
		CamRenderQueuer queuer = { _viewCamera };
		if (!_debugPlayer->InitialRender(queuer, _viewCamera->GetRendererInfo()))
		{
			ErrMsg("Failed to render debug player!");
			return false;
		}
	}

	if (_transformGizmo)
	{
		CamRenderQueuer queuer = { _viewCamera };
		if (!_transformGizmo->GetEntity()->InitialRender(queuer, _viewCamera->GetRendererInfo()))
		{
			ErrMsg("Failed to render debug player!");
			return false;
		}
	}
#endif

	// Run BeforeRender on entities
	const UINT entityCount = _sceneHolder.GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		if (!_sceneHolder.GetEntity(i)->InitialBeforeRender())
		{
			ErrMsg(std::format("Failed to run BeforeRender on entity '{}'!", _sceneHolder.GetEntity(i)->GetName()));
			return false;
		}
	}

	const UINT globalEntityCount = static_cast<UINT>(_globalEntities.size());
	for (UINT i = 0; i < globalEntityCount; i++)
	{
		if (!_globalEntities.at(i)->InitialBeforeRender())
		{
			ErrMsg(std::format("Failed to run BeforeRender on global entity '{}'!", _globalEntities.at(i)->GetName()));
			return false;
		}
	}

	return true;
}

#ifdef USE_IMGUI
bool Scene::RenderUI()
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(6681043, "Scene Render UI");
#endif

	if (ImGui::TreeNode("Creation"))
	{
		// Button that takes a file during runtime and creates a new mesh and object
		if (ImGui::Button("Load mesh from file"))
		{
			const char *filterPatterns[] = { "*.obj", "*.fbx", "*.png", "*.jpg", "*.*" };
			const char *selectedFiles = tinyfd_openFileDialog(
				"Open File",
				"",
				5,
				filterPatterns,
				"Supported Files",
				1
			);

			if (selectedFiles)
			{
				std::string fileString = selectedFiles;

				std::vector<std::string> filePaths;
				std::stringstream ss(fileString);
				std::string filePath;
				while (std::getline(ss, filePath, '|'))
				{
					filePaths.push_back(filePath);
				}

				std::string meshFolder = std::filesystem::current_path().string() + "\\Content\\Meshes";
				std::string textureFolder = std::filesystem::current_path().string() + "\\Content\\Textures";

				std::filesystem::create_directories(meshFolder);
				std::filesystem::create_directories(textureFolder);

				UINT meshID = CONTENT_NULL;
				Material mat;
				mat.textureID = _content->GetTextureID("Tex_White");
				mat.ambientID = _content->GetTextureID("Tex_Ambient");

				std::string meshName;

				for (const auto &sourcePath : filePaths)
				{
					std::string fileName = std::filesystem::path(sourcePath).filename().string();

					size_t dot = fileName.find(".");
					std::string name = fileName.substr(0, dot);

					if (fileName.ends_with(".obj"))
					{
						size_t dot = fileName.find(".");

						std::string name = fileName.substr(0, dot);
						UINT id = _content->GetMeshID("Mesh_" + name);
						if (id == _content->GetMeshID("Mesh_Error"))
						{
							std::ofstream file("Content/Meshes/_meshNames.txt", std::ios::app);
							std::string destPath = meshFolder + "\\" + fileName;
							std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);

							file << "\n" + name;
							meshName = name;
							if (!_content->AddMesh(_device, std::format("Mesh_{}", name), std::format("Content\\Meshes\\{}.obj", name).c_str()))
							{
								ErrMsg(std::format("Failed to add Mesh_{}", name));
								return false;
							}
							meshID = _content->GetMeshID("Mesh_" + name);
							file.close();
						}
						else
						{
							meshID = id;
							meshName = name;
						}
					}

					else if (fileName.ends_with(".png"))
					{
						std::string destPath = textureFolder + "\\" + fileName;

						std::filesystem::copy_file(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);

						bool isTexMap = false;
						TextureType type = TextureType::NORMAL;
						UINT *destID = nullptr;

						if (fileName.ends_with("_Ambient.png"))
						{
							destID = &mat.ambientID;
						}
						else if (fileName.ends_with("_Lightmap.png"))
						{
							destID = &mat.lightID;
						}
						else if (fileName.ends_with("_Normal.png"))
						{
							isTexMap = true;
							type = TextureType::NORMAL;
							destID = &mat.normalID;
						}
						else if (fileName.ends_with("_Specular.png"))
						{
							isTexMap = true;
							type = TextureType::SPECULAR;
							destID = &mat.specularID;
						}
						else if (fileName.ends_with("_Glossiness.png"))
						{
							isTexMap = true;
							type = TextureType::GLOSS;
							destID = &mat.glossinessID;
						}
						else if (fileName.ends_with("_Height.png"))
						{
							isTexMap = true;
							type = TextureType::HEIGHT;
							destID = &mat.heightID;
						}
						else if (fileName.ends_with("_Occlusion.png"))
						{
							isTexMap = true;
							type = TextureType::OCCLUSION;
							destID = &mat.occlusionID;
						}
						else
						{
							destID = &mat.textureID;
						}

						if (isTexMap)
						{
							if (_content->GetTextureMapID("TexMap_" + name) == CONTENT_NULL)
							{
								if (_content->AddTextureMap(
									_device, _context, std::format("TexMap_{}", name), type,
									std::format("Content\\Textures\\{}.png", name).c_str()) == CONTENT_NULL)
								{
									ErrMsg(std::format("Failed to add TexMap_{}!", name));
									return false;
								}

								std::ofstream file("Content/Textures/_textureMaps.txt", std::ios::app);
								file << "\n" + name;
							}

							*destID = _content->GetTextureMapID("TexMap_" + name);
						}
						else
						{
							if (!_content->HasTexture("Tex_" + name))
							{
								if (_content->AddTexture(
									_device, _context, std::format("Tex_{}", name),
									std::format("Content\\Textures\\{}.png", name).c_str(), false) == CONTENT_NULL)
								{
									ErrMsg(std::format("Failed to add Tex_{}!", name));
									return false;
								}

								std::ofstream file("Content/Textures/_textures.txt", std::ios::app);
								file << "\n" + name;
							}

							*destID = _content->GetTextureID("Tex_" + name);
						}
					}
				}

				Entity *ent = nullptr;
				if (!CreateMeshEntity(&ent, meshName, meshID, mat, false, false))
				{
					ErrMsg("Failed to create object!");
					return false;
				}

				_debugPlayer->PositionWithCursor(ent);
			}
		}

		if (ImGui::TreeNode("Entity Creator"))
		{
			ImGuiChildFlags childFlags = 0;
			childFlags |= ImGuiChildFlags_Border;
			childFlags |= ImGuiChildFlags_ResizeY;

			ImGui::BeginChild("Entity Creator", ImVec2(0, 300), childFlags);

			if (!RenderEntityCreatorUI())
			{
				ImGui::EndChild();
				ImGui::TreePop();
				ErrMsg("Failed to render entity creator UI!");
				return false;
			}

			ImGui::EndChild();
			ImGui::TreePop();
		}

		ImGui::Separator();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Hierarchy"))
	{
		ImGui::Text(std::format("Objects in Scene: {}", _sceneHolder.GetEntityCount()).c_str());

		if (ImGui::TreeNode("Scene Hierarchy"))
		{
			ImGui::PushID("Scene Hierarchy");
			ImGui::Checkbox("Undock", &_undockSceneHierarchy);

			if (_undockSceneHierarchy)
				ImGui::Begin("Scene Hierarchy", &_undockSceneHierarchy);

			static std::string search = "";
			ImGui::InputText("Search", &search);
			if (ImGui::SmallButton("Clear Search"))
				search = "";

			ImGuiChildFlags childFlags = 0;
			childFlags |= ImGuiChildFlags_Border;
			childFlags |= ImGuiChildFlags_ResizeY;

			ImGui::BeginChild("Scene Hierarchy", ImVec2(0, 300), childFlags);
			std::vector<Entity *> sceneContent;
			_sceneHolder.GetEntities(sceneContent);

			for (auto &entity : sceneContent)
			{
				if (entity->GetParent() != nullptr) // Skip non-root entities
					continue;

				std::string searchLower = search;
				std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
				if (!RenderEntityHierarchyUI(entity, 0, searchLower))
				{
					ImGui::EndChild();

					if (_undockSceneHierarchy)
						ImGui::End();

					ImGui::PopID();
					ImGui::TreePop();
					return false;
				}
			}
			ImGui::EndChild();

			if (_undockSceneHierarchy)
				ImGui::End();

			ImGui::PopID();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Entity Hierarchy"))
		{
			ImGui::PushID("Entity Hierarchy");
			ImGui::Checkbox("Undock", &_undockEntityHierarchy);

			if (_undockEntityHierarchy)
				ImGui::Begin("Entity Hierarchy", &_undockEntityHierarchy);

			static std::string search = "";
			ImGui::InputText("Search", &search);
			if (ImGui::SmallButton("Clear Search"))
				search = "";

			ImGuiChildFlags childFlags = 0;
			childFlags |= ImGuiChildFlags_Border;
			childFlags |= ImGuiChildFlags_ResizeY;

			ImGui::BeginChild("Entity Hierarchy", ImVec2(0, 300), childFlags);
			if (_debugPlayer->GetSelection() >= 0)
			{
				Entity *ent = _sceneHolder.GetEntity(_debugPlayer->GetSelection());

				std::string searchLower = search;
				std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
				if (!RenderEntityHierarchyUI(ent, 0, searchLower))
				{
					ImGui::EndChild();

					if (_undockEntityHierarchy)
						ImGui::End();

					ImGui::PopID();
					ImGui::TreePop();
					return false;
				}
			}
			ImGui::EndChild();

			if (_undockEntityHierarchy)
				ImGui::End();

			ImGui::PopID();
			ImGui::TreePop();
		}

		ImGui::Separator();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Selected Entity"))
	{
		ImGuiChildFlags childFlags = 0;
		childFlags |= ImGuiChildFlags_Border;
		childFlags |= ImGuiChildFlags_ResizeY;

		ImGui::BeginChild("Selection UI", ImVec2(0, 300), childFlags);
		int selection = _debugPlayer->GetSelection();

		Entity *ent = _sceneHolder.GetEntity(selection);
		UINT entID = -1;
		if (ent)
			entID = ent->GetID();

		// check if entity is undocked
		auto dockIter = std::find(_undockedEntities.begin(), _undockedEntities.end(), entID);
		if (dockIter != _undockedEntities.end())
		{
			ImGui::Text("Selected entity is undocked!");
		}
		else if (!RenderEntityUI(entID))
		{
			ImGui::EndChild();
			ImGui::TreePop();
			ErrMsg("Failed to render selected entity UI!");
			return false;
		}
		ImGui::EndChild();

		for (int i = 0; i < _undockedEntities.size(); i++)
		{
			UINT undockedID = _undockedEntities[i];
			Entity *undockedEnt = _sceneHolder.GetEntityByID(undockedID);
			
			// Check if entity was removed
			if (!undockedEnt)
			{
				_undockedEntities.erase(_undockedEntities.begin() + i--);
				continue;
			}

			ImGui::Begin(std::format("{} ({}) Properties", undockedEnt->GetName(), undockedID).c_str());
			if (!RenderEntityUI(undockedID))
			{
				ImGui::End();
				ImGui::TreePop();
				ErrMsg("Failed to render undocked entity UI!");
				return false;
			}
			ImGui::End();
		}

		ImGui::TreePop();
	}

	ImGui::Dummy(ImVec2(0.0f, 6.0f));

	if (ImGui::TreeNode("Graph Manager"))
	{
		ImGui::Text(std::format("Nodes in Scene: {}", _graphManager.GetNodeCount()).c_str());

		if (_monster)
		{
			ImGui::Separator();
			static bool showMonsterPath = true;
			ImGui::Checkbox("Show Monster Path", &showMonsterPath);
			if (showMonsterPath)
			{
				static bool overlayMonsterPath = false;
				ImGui::Checkbox("Overlay##OverlayMonsterPath", &overlayMonsterPath);

				std::vector<XMFLOAT3> *points;
				if (_monster->GetPath(points))
				{
					DebugDrawer *drawer = DebugDrawer::GetInstance();
					drawer->DrawLineStrip(
						points->data(), 
						static_cast<UINT>(points->size()), 
						0.275f, { 0,1,0.5f,1 }, 
						!overlayMonsterPath
					);
				}
			}
			ImGui::Separator();
		}

		int selection = _debugPlayer->GetSelection();
		Entity *ent = _sceneHolder.GetEntity(selection);
		XMFLOAT3 posA = ent ? To3(ent->GetTransform()->GetPosition(World)) : XMFLOAT3(0,0,0);

		XMFLOAT3 fromPos = posA;
		static XMFLOAT3 toPos;
		ImGui::InputFloat3("Path To", &toPos.x);

		if (!_graphManager.RenderUI(fromPos, toPos))
		{
			ImGui::TreePop();
			return false;
		}

		static int firstNode = -1;
		static enum class GraphEditType { 
			None, Connect, Disconnect, Split,
		} graphEditType = GraphEditType::None;

		if (firstNode >= 0)
		{
			ImGui::Text("Select Target Node...");

			if (ImGui::Button("[Num1] Cancel Connection") || _input->GetKey(KeyCode::NumPad1) == KeyState::Pressed)
			{
				_debugPlayer->SetSelection(firstNode);
				firstNode = -1;
			}
		}

		if (ent)
		{
			GraphNodeBehaviour *node;
			if (ent->GetBehaviourByType<GraphNodeBehaviour>(node))
			{
				if (ImGui::TreeNode("Node Properties"))
				{
					ImGui::PushID("NodeProperties");
					if (!node->InitialRenderUI())
					{
						ImGui::PopID();
						ImGui::TreePop();
						ErrMsg("Failed to render selected node properties!");
						return false;
					}
					ImGui::PopID();

					ImGui::TreePop();
				}

				if (firstNode < 0)
				{
					if (ImGui::Button("[Num0] Connect Selected") || _input->GetKey(KeyCode::NumPad0) == KeyState::Pressed)
					{
						graphEditType = GraphEditType::Connect;
						firstNode = selection;
						_debugPlayer->SetSelection(-1);
					}
					if (ImGui::Button("[Num2] Disconnect Selected") || _input->GetKey(KeyCode::NumPad2) == KeyState::Pressed)
					{
						graphEditType = GraphEditType::Disconnect;
						firstNode = selection;
						_debugPlayer->SetSelection(-1);
					}
					ImGui::Text("[Num3] New Connected Node");
					if (ImGui::Button("[Num4] Split Selected") || _input->GetKey(KeyCode::NumPad4) == KeyState::Pressed)
					{
						graphEditType = GraphEditType::Split;
						firstNode = selection;
						_debugPlayer->SetSelection(-1);
					}
					if (ImGui::Button("[Num5] New Connected Node At Camera") || _input->GetKey(KeyCode::NumPad5) == KeyState::Pressed)
					{
						Transform *cameraTransform = _viewCamera->GetTransform();

						Entity *copyEnt = nullptr;
						GraphNodeBehaviour *copyNode = nullptr;

						if (!CreateGraphNodeEntity(&copyEnt, &copyNode, cameraTransform->GetPosition()))
						{
							ErrMsg("Failed to copy node entity!");
							return false;
						}

						copyNode->AddConnection(node);
						_debugPlayer->SetSelectionID(copyEnt->GetID());
					}
					if (ImGui::Button("[Num6] Mark As Mine") || _input->GetKey(KeyCode::NumPad6) == KeyState::Pressed)
					{
						node->SetCost(0.0f);
					}
				}
				else
				{
					Entity *otherEnt = _sceneHolder.GetEntity(firstNode);
					GraphNodeBehaviour *otherNode;
					if (otherEnt->GetBehaviourByType<GraphNodeBehaviour>(otherNode))
					{
						DebugDrawer::GetInstance()->DrawLine(
							posA, otherEnt->GetTransform()->GetPosition(World), 
							0.4f, { 1,0,1,0.3f }, false
						);

						if (ImGui::Button("[Enter] Apply") || _input->GetKey(KeyCode::Enter) == KeyState::Pressed)
						{
							switch (graphEditType)
							{
							case GraphEditType::Connect:
								otherNode->AddConnection(node);
								_debugPlayer->SetSelection(firstNode);
								break;

							case GraphEditType::Disconnect:
								otherNode->RemoveConnection(node);
								_debugPlayer->SetSelection(firstNode);
								break;

							case GraphEditType::Split:
								// Remove existing connection
								otherNode->RemoveConnection(node);

								// Create new node between selected nodes
								XMFLOAT3 newPos;
								Store(newPos, (Load(posA) + Load(otherEnt->GetTransform()->GetPosition(World))) * 0.5f);

								Entity *newEnt;
								GraphNodeBehaviour *newNode;
								if (!CreateGraphNodeEntity(&newEnt, &newNode, newPos))
								{
									ErrMsg("Failed to create new node!");
									return false;
								}

								// Connect new node to both selected nodes
								newNode->AddConnection(node);
								newNode->AddConnection(otherNode);

								// Select new node
								_debugPlayer->SetSelectionID(newEnt->GetID());
								break;
							}

							firstNode = -1;
						}
					}
				}
			}
		}

		ImGui::Separator();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Timeline Manager")) 
	{
		if (!_timelineManager.RenderUI(_viewCamera->GetTransform()))
		{
			ImGui::TreePop();
			return false;
		}

		ImGui::Separator();
		ImGui::TreePop();
	}

	ImGui::Dummy(ImVec2(0.0f, 6.0f));

	if (ImGui::TreeNode("Other"))
	{
		// Transform Gizmo Settings
		static float gizmoScale = 1.0f;
		static float gridSize = 0.5f;

		if (ImGui::SliderFloat("Transform Gizmo Scale", &gizmoScale, 0.0f, 5.0f))
			_transformGizmo->SetGizmoScale(gizmoScale);

		if (ImGui::InputFloat("Grid Size", &gridSize))
			_transformGizmo->SetGridSize(gridSize);

		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		static bool renderEmitterGizmos = false;
		static bool emitterGizmosOverlayed = false;
		bool updateEmitterGizmos = false;

		if (ImGui::Checkbox("Render ambient sound gizmos", &renderEmitterGizmos))
			updateEmitterGizmos = true;

		if (renderEmitterGizmos)
			if (ImGui::Checkbox("Overlay", &emitterGizmosOverlayed))
				updateEmitterGizmos = true;

		if (updateEmitterGizmos)
		{
			UINT entCount = _sceneHolder.GetEntityCount();
			for (UINT i = 0; i < entCount; i++)
			{
				Entity *ent = _sceneHolder.GetEntity(i);

				if (!ent)
					continue;

				AmbientSoundBehaviour* ambientSound;
				if (!ent->GetBehaviourByType<AmbientSoundBehaviour>(ambientSound))
					continue;

				BillboardMeshBehaviour* billboard;
				if (!ent->GetBehaviourByType<BillboardMeshBehaviour>(billboard))
					continue;

				billboard->SetEnabled(renderEmitterGizmos);
				billboard->GetMeshBehaviour()->SetOverlay(emitterGizmosOverlayed);
			}
		}

		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		static bool currState = false;
		static float fadeDuration = 1.0f;
		ImGui::SliderFloat("Fade Duration", &fadeDuration, 0.0f, 3.0f);

		if (ImGui::Button("Begin Fade"))
		{
			_graphics->BeginScreenFade(fadeDuration * (currState ? -1.0f : 1.0f));
			currState = !currState;
		}

		if (ImGui::Button("Clear All Duplicate Binds"))
		{
			_debugPlayer->ClearDuplicateBinds();
		}

		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		if (ImGui::TreeNode("Global Texture Applier"))
		{
			Content *content = _content;

			UINT startVS, endVS;
			UINT startPS, endPS;

			content->GetShaderTypeRange("VS_", startVS, endVS);
			content->GetShaderTypeRange("PS_", startPS, endPS);

			static int
				inputMeshID = (int)-1,
				inputTexID = (int)0,
				inputNormID = (int)-1,
				inputSpecID = (int)-1,
				inputGlossID = (int)-1,
				inputAmbID = (int)-1,
				inputLightID = (int)-1,
				inputOcclusionID = (int)-1,
				inputHeightID = (int)-1,
				inputSampID = (int)-1,
				inputVSID = (int)-1,
				inputPSID = (int)-1;

			static int previewSize = 128;
			ImGui::Text("Preview Size: ");
			ImGui::SameLine();
			ImGui::InputInt("##PreviewSize", &previewSize);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImVec2 previewVec = ImVec2(abs(static_cast<float>(previewSize)), abs(static_cast<float>(previewSize)));
			ImTextureID fallbackImg = (ImTextureID)content->GetTexture("Tex_Fallback")->GetSRV();

			ImGui::PushID("Mats");
			bool isChanged = false;
			int id = 1;

			// Mesh
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Mesh: " + content->GetMeshName((UINT)inputMeshID)).c_str());
				if (ImGui::InputInt("", &inputMeshID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Texture map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Texture: " + content->GetTextureName((UINT)inputTexID)).c_str());
				if (ImGui::InputInt("", &inputTexID))
					isChanged = true;
				ImGui::Image((ImTextureID)content->GetTexture((UINT)inputTexID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Normal map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Normal: " + content->GetTextureMapName((UINT)inputNormID)).c_str());
				if (ImGui::InputInt("", &inputNormID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputNormID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputNormID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Specular map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Specular: " + content->GetTextureMapName((UINT)inputSpecID)).c_str());
				if (ImGui::InputInt("", &inputSpecID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputSpecID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputSpecID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Glossiness map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Glossiness: " + content->GetTextureMapName((UINT)inputGlossID)).c_str());
				if (ImGui::InputInt("", &inputGlossID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputGlossID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputGlossID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Ambient map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Ambient: " + content->GetTextureName((UINT)inputAmbID)).c_str());
				if (ImGui::InputInt("", &inputAmbID))
					isChanged = true;
				ImGui::Image((ImTextureID)content->GetTexture((UINT)inputAmbID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Light map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Baked Light: " + content->GetTextureName((UINT)inputLightID)).c_str());
				if (ImGui::InputInt("", &inputLightID))
					isChanged = true;
				ImGui::Image((ImTextureID)content->GetTexture((UINT)inputLightID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Occlusion map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Ambient Occlusion: " + content->GetTextureMapName((UINT)inputOcclusionID)).c_str());
				if (ImGui::InputInt("", &inputOcclusionID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputOcclusionID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputOcclusionID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Height map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Height: " + content->GetTextureMapName((UINT)inputHeightID)).c_str());
				if (ImGui::InputInt("", &inputHeightID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputHeightID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputHeightID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Sampler
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Sampler: " + content->GetSamplerName((UINT)inputSampID)).c_str());
				if (ImGui::InputInt("", &inputSampID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Vertex Shader
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Vertex Shader: " + content->GetShaderName((UINT)inputVSID)).c_str());
				if (ImGui::InputInt("", &inputVSID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Pixel Shader
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Pixel Shader: " + content->GetShaderName((UINT)inputPSID)).c_str());
				if (ImGui::InputInt("", &inputPSID))
					isChanged = true;
				ImGui::PopID();
			}

			ImGui::PopID();

			if (isChanged)
			{
				inputMeshID += (int)content->GetMeshCount();
				inputMeshID %= (int)content->GetMeshCount();

				inputTexID += (int)content->GetTextureCount();
				inputTexID %= (int)content->GetTextureCount();

				if (inputNormID != -1)
				{
					if (inputNormID < 0)
						inputNormID++;

					inputNormID += (int)content->GetTextureMapCount();
					inputNormID %= (int)content->GetTextureMapCount();
				}

				if (inputSpecID != -1)
				{
					if (inputSpecID < 0)
						inputSpecID++;

					inputSpecID += (int)content->GetTextureMapCount();
					inputSpecID %= (int)content->GetTextureMapCount();
				}

				if (inputGlossID != -1)
				{
					if (inputGlossID < 0)
						inputGlossID++;

					inputGlossID += (int)content->GetTextureMapCount();
					inputGlossID %= (int)content->GetTextureMapCount();
				}

				if (inputAmbID != -1)
				{
					if (inputAmbID < 0)
						inputAmbID++;

					inputAmbID += (int)content->GetTextureCount();
					inputAmbID %= (int)content->GetTextureCount();
				}

				if (inputLightID != -1)
				{
					if (inputLightID < 0)
						inputLightID++;

					inputLightID += (int)content->GetTextureCount();
					inputLightID %= (int)content->GetTextureCount();
				}

				if (inputOcclusionID != -1)
				{
					if (inputOcclusionID < 0)
						inputOcclusionID++;

					inputOcclusionID += (int)content->GetTextureMapCount();
					inputOcclusionID %= (int)content->GetTextureMapCount();
				}

				if (inputHeightID != -1)
				{
					if (inputHeightID < 0)
						inputHeightID++;

					inputHeightID += (int)content->GetTextureMapCount();
					inputHeightID %= (int)content->GetTextureMapCount();
				}

				if (inputSampID != -1)
				{
					if (inputSampID < 0)
						inputSampID++;

					inputSampID += (int)content->GetSamplerCount();
					inputSampID %= (int)content->GetSamplerCount();
				}

				if (inputVSID != -1)
				{
					if (static_cast<UINT>(inputVSID) < startVS)
						inputVSID = endVS;
					else if (static_cast<UINT>(inputVSID) > endVS)
						inputVSID = startVS;
				}

				if (inputPSID != -1)
				{
					if (static_cast<UINT>(inputPSID) < startPS)
						inputPSID = endPS;
					else if (static_cast<UINT>(inputPSID) > endPS)
						inputPSID = startPS;
				}

			}

			if (ImGui::Button("Apply"))
			{
				Material newMat;
				newMat.textureID = (UINT)inputTexID;
				newMat.normalID = (UINT)inputNormID;
				newMat.specularID = (UINT)inputSpecID;
				newMat.glossinessID = (UINT)inputGlossID;
				newMat.ambientID = (UINT)inputAmbID;
				newMat.lightID = (UINT)inputLightID;
				newMat.occlusionID = (UINT)inputOcclusionID;
				newMat.heightID = (UINT)inputHeightID;
				newMat.samplerID = (UINT)inputSampID;
				newMat.vsID = (UINT)inputVSID;
				newMat.psID = (UINT)inputPSID;

				std::vector<Entity *> entities;
				_sceneHolder.GetEntities(entities);


				for (Entity *entity : entities)
				{
					MeshBehaviour *mesh = nullptr;
					if (entity->GetBehaviourByType<MeshBehaviour>(mesh))
					{
						if (mesh->GetMesh() == (UINT)inputMeshID)
						{
							if (!mesh->SetMaterial(&newMat))
							{
								ErrMsg("Failed to set material!");
								return false;
							}

						}
					}
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Global Mesh Applier"))
		{
			Content *content = _content;

			UINT startVS, endVS;
			UINT startPS, endPS;

			content->GetShaderTypeRange("VS_", startVS, endVS);
			content->GetShaderTypeRange("PS_", startPS, endPS);

			static int
				inputMeshID = (int)-1,
				inputTexID = (int)0,
				inputNormID = (int)-1,
				inputSpecID = (int)-1,
				inputGlossID = (int)-1,
				inputAmbID = (int)0,
				inputLightID = (int)0,
				inputOcclusionID = (int)-1,
				inputHeightID = (int)-1,
				inputSampID = (int)-1,
				inputVSID = (int)-1,
				inputPSID = (int)-1,
				outputMeshID = (int)-1;

			static int previewSize = 128;
			ImGui::Text("Preview Size: ");
			ImGui::SameLine();
			ImGui::InputInt("##PreviewSize", &previewSize);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImVec2 previewVec = ImVec2(abs(previewSize), abs(previewSize));
			ImTextureID fallbackImg = (ImTextureID)content->GetTexture("Tex_Fallback")->GetSRV();

			ImGui::PushID("Mats");
			bool isChanged = false;
			int id = 1;

			// Mesh
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Mesh to change: " + content->GetMeshName((UINT)inputMeshID)).c_str());
				if (ImGui::InputInt("", &inputMeshID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Mesh
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Mesh to apply: " + content->GetMeshName((UINT)outputMeshID)).c_str());
				if (ImGui::InputInt("", &outputMeshID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Texture map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Texture: " + content->GetTextureName((UINT)inputTexID)).c_str());
				if (ImGui::InputInt("", &inputTexID))
					isChanged = true;
				ImGui::Image((ImTextureID)content->GetTexture((UINT)inputTexID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Normal map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Normal: " + content->GetTextureMapName((UINT)inputNormID)).c_str());
				if (ImGui::InputInt("", &inputNormID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputNormID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputNormID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Specular map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Specular: " + content->GetTextureMapName((UINT)inputSpecID)).c_str());
				if (ImGui::InputInt("", &inputSpecID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputSpecID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputSpecID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Glossiness map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Glossiness: " + content->GetTextureMapName((UINT)inputGlossID)).c_str());
				if (ImGui::InputInt("", &inputGlossID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputGlossID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputGlossID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Ambient map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Ambient: " + content->GetTextureName((UINT)inputAmbID)).c_str());
				if (ImGui::InputInt("", &inputAmbID))
					isChanged = true;
				ImGui::Image((ImTextureID)content->GetTexture((UINT)inputAmbID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Light map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Baked Light: " + content->GetTextureName((UINT)inputLightID)).c_str());
				if (ImGui::InputInt("", &inputLightID))
					isChanged = true;
				ImGui::Image((ImTextureID)content->GetTexture((UINT)inputLightID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Occlusion map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Ambient Occlusion: " + content->GetTextureMapName((UINT)inputOcclusionID)).c_str());
				if (ImGui::InputInt("", &inputOcclusionID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputOcclusionID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputOcclusionID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Height map
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Height: " + content->GetTextureMapName((UINT)inputHeightID)).c_str());
				if (ImGui::InputInt("", &inputHeightID))
					isChanged = true;
				if (!content->GetTextureMap((UINT)inputHeightID))
					ImGui::Image(fallbackImg, previewVec);
				else
					ImGui::Image((ImTextureID)content->GetTextureMap((UINT)inputHeightID)->GetSRV(), previewVec);
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Sampler
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Sampler: " + content->GetSamplerName((UINT)inputSampID)).c_str());
				if (ImGui::InputInt("", &inputSampID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Vertex Shader
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Vertex Shader: " + content->GetShaderName((UINT)inputVSID)).c_str());
				if (ImGui::InputInt("", &inputVSID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Pixel Shader
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Pixel Shader: " + content->GetShaderName((UINT)inputPSID)).c_str());
				if (ImGui::InputInt("", &inputPSID))
					isChanged = true;
				ImGui::PopID();
			}

			ImGui::PopID();

			if (isChanged)
			{
				inputTexID += (int)content->GetTextureCount();
				inputTexID %= (int)content->GetTextureCount();

				outputMeshID += (int)content->GetMeshCount();
				outputMeshID %= (int)content->GetMeshCount();

				if (inputNormID != -1)
				{
					if (inputNormID < 0)
						inputNormID++;

					inputNormID += (int)content->GetTextureMapCount();
					inputNormID %= (int)content->GetTextureMapCount();
				}

				if (inputSpecID != -1)
				{
					if (inputSpecID < 0)
						inputSpecID++;

					inputSpecID += (int)content->GetTextureMapCount();
					inputSpecID %= (int)content->GetTextureMapCount();
				}

				if (inputGlossID != -1)
				{
					if (inputGlossID < 0)
						inputGlossID++;

					inputGlossID += (int)content->GetTextureMapCount();
					inputGlossID %= (int)content->GetTextureMapCount();
				}

				if (inputAmbID != -1)
				{
					if (inputAmbID < 0)
						inputAmbID++;

					inputAmbID += (int)content->GetTextureCount();
					inputAmbID %= (int)content->GetTextureCount();
				}

				if (inputLightID != -1)
				{
					if (inputLightID < 0)
						inputLightID++;

					inputLightID += (int)content->GetTextureCount();
					inputLightID %= (int)content->GetTextureCount();
				}

				if (inputOcclusionID != -1)
				{
					if (inputOcclusionID < 0)
						inputOcclusionID++;

					inputOcclusionID += (int)content->GetTextureMapCount();
					inputOcclusionID %= (int)content->GetTextureMapCount();
				}

				if (inputHeightID != -1)
				{
					if (inputHeightID < 0)
						inputHeightID++;

					inputHeightID += (int)content->GetTextureMapCount();
					inputHeightID %= (int)content->GetTextureMapCount();
				}

				if (inputSampID != -1)
				{
					if (inputSampID < 0)
						inputSampID++;

					inputSampID += (int)content->GetSamplerCount();
					inputSampID %= (int)content->GetSamplerCount();
				}

				if (inputVSID != -1)
				{
					if (inputVSID < startVS)
						inputVSID = endVS;
					else if (inputVSID > endVS)
						inputVSID = startVS;
				}

				if (inputPSID != -1)
				{
					if (inputPSID < startPS)
						inputPSID = endPS;
					else if (inputPSID > endPS)
						inputPSID = startPS;
				}

			}

			if (ImGui::Button("Apply"))
			{
				Material newMat;
				newMat.textureID = (UINT)inputTexID;
				newMat.normalID = (UINT)inputNormID;
				newMat.specularID = (UINT)inputSpecID;
				newMat.glossinessID = (UINT)inputGlossID;
				newMat.ambientID = (UINT)inputAmbID;
				newMat.lightID = (UINT)inputLightID;
				newMat.occlusionID = (UINT)inputOcclusionID;
				newMat.heightID = (UINT)inputHeightID;
				newMat.samplerID = (UINT)inputSampID;
				newMat.vsID = (UINT)inputVSID;
				newMat.psID = (UINT)inputPSID;

				std::vector<Entity *> entities;
				_sceneHolder.GetEntities(entities);


				for (Entity *entity : entities)
				{
					MeshBehaviour *mesh = nullptr;
					if (entity->GetBehaviourByType<MeshBehaviour>(mesh))
					{
						if (mesh->GetMesh() == (UINT)inputMeshID)
						{
							if (!mesh->SetMaterial(&newMat))
							{
								ErrMsg("Failed to set material!");
								return false;
							}

							if (outputMeshID != -1)
							{
								mesh->SetMesh(outputMeshID, true);
							}
						}
					}
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Global Collision Applier"))
		{
			Content *content = _content;

			UINT startVS, endVS;
			UINT startPS, endPS;

			content->GetShaderTypeRange("VS_", startVS, endVS);
			content->GetShaderTypeRange("PS_", startPS, endPS);

			static int inputMeshID = (int)-1;

			const int nColliders = 5;
			const char *colliderNames[nColliders] = { "Ray Collider", "Sphere Collider", "Capsule Collider",
										   "AABB Collider", "OBB Collider" };

			// TODO: Fix Ray Collider
			static int selectedIndex = 0;
			const char *preview = colliderNames[selectedIndex];

			static int previewSize = 128;
			ImGui::Text("Preview Size: ");
			ImGui::SameLine();
			ImGui::InputInt("##PreviewSize", &previewSize);
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImVec2 previewVec = ImVec2(abs(static_cast<float>(previewSize)), abs(static_cast<float>(previewSize)));
			ImTextureID fallbackImg = (ImTextureID)content->GetTexture("Tex_Fallback")->GetSRV();

			ImGui::PushID("Mats");
			bool isChanged = false;
			int id = 1;

			// Mesh
			{
				ImGui::PushID(("Param " + std::to_string(id++)).c_str());
				ImGui::Text(("Mesh: " + content->GetMeshName((UINT)inputMeshID)).c_str());
				if (ImGui::InputInt("", &inputMeshID))
					isChanged = true;
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			// Collider
			{
				if (ImGui::BeginCombo("Colliders", preview))
				{
					for (int i = 1; i < nColliders; i++)
					{
						const bool isSelected = (selectedIndex == i);
						if (ImGui::Selectable(colliderNames[i], isSelected))
							selectedIndex = i;

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));

			ImGui::PopID();

			if (isChanged)
			{
				inputMeshID += (int)content->GetMeshCount();
				inputMeshID %= (int)content->GetMeshCount();
			}

			if (ImGui::Button("Apply"))
			{
				Collisions::ColliderTypes type = (Collisions::ColliderTypes)selectedIndex;

				std::vector<Entity *> entities;
				_sceneHolder.GetEntities(entities);

				for (Entity *entity : entities)
				{
					MeshBehaviour *mesh = nullptr;
					if (entity->GetBehaviourByType<MeshBehaviour>(mesh))
					{
						if (mesh->GetMesh() == (UINT)inputMeshID)
						{
							std::vector<ColliderBehaviour *> cbs;
							if (entity->GetBehavioursByType<ColliderBehaviour>(cbs))
							{
								continue;
							}

							BoundingOrientedBox bounds = content->GetMesh(inputMeshID)->GetBoundingOrientedBox();
							ColliderBehaviour *cb = new ColliderBehaviour();
							if (!cb->Initialize(entity))
							{
								ErrMsg("Failed to initialize new collider behaviour!");
								return false;
							}

							switch (type)
							{
							case Collisions::RAY_COLLIDER:
								ErrMsg("Ray collider not supported at the moment!");
								return true;
							case Collisions::SPHERE_COLLIDER:
								cb->SetCollider(new Collisions::Sphere(bounds.Center, std::min<float>(std::min<float>(bounds.Extents.x, bounds.Extents.z), bounds.Extents.y)));
								break;
							case Collisions::CAPSULE_COLLIDER:
								cb->SetCollider(new Collisions::Capsule(bounds.Center, { 0, 1, 0 }, std::min<float>(bounds.Extents.x, bounds.Extents.z), bounds.Extents.y));
								break;
							case Collisions::AABB_COLLIDER:
								cb->SetCollider(new Collisions::AABB(bounds));
								break;
							case Collisions::OBB_COLLIDER:
								cb->SetCollider(new Collisions::OBB(bounds));
								break;
							case Collisions::TERRAIN_COLLIDER:
								break;
							case Collisions::NULL_COLLIDER:
							default:
								ErrMsg("Incorrect collider type!");
								return false;
							}
						}
					}
				}
				_sceneHolder.SetRecalculateColliders();
			}

			ImGui::TreePop();
		}

		XMFLOAT3A camPos = _viewCamera->GetTransform()->GetPosition();
		char camXCoord[32]{}, camYCoord[32]{}, camZCoord[32]{};
		snprintf(camXCoord, sizeof(camXCoord), "%.2f", camPos.x);
		snprintf(camYCoord, sizeof(camYCoord), "%.2f", camPos.y);
		snprintf(camZCoord, sizeof(camZCoord), "%.2f", camPos.z);
		ImGui::Text(std::format("Camera Pos: ({}, {}, {})", camXCoord, camYCoord, camZCoord).c_str());

		ImGui::Separator();

		char nearPlane[16]{}, farPlane[16]{};
		for (UINT i = 0; i < _spotlights->GetNrOfLights(); i++)
		{
			const ProjectionInfo projInfo = _spotlights->GetLightBehaviour(i)->GetShadowCamera()->GetCurrProjectionInfo();
			snprintf(nearPlane, sizeof(nearPlane), "%.2f", projInfo.planes.nearZ);
			snprintf(farPlane, sizeof(farPlane), "%.1f", projInfo.planes.farZ);
			ImGui::Text(std::format("({}:{}) Planes Spotlight #{}", nearPlane, farPlane, i).c_str());
		}

		ImGui::TreePop();
	}
	return true;
}
bool Scene::RenderEntityCreatorUI()
{
	ImGuiChildFlags childFlags = 0;
	childFlags |= ImGuiChildFlags_Border;
	childFlags |= ImGuiChildFlags_ResizeY;
	childFlags |= ImGuiChildFlags_ResizeX;

	Entity *ent = nullptr;
	static bool positionWithCursor = false;
	ImGui::Checkbox("Position Entity With Cursor", &positionWithCursor);

	ImGui::BeginChild("Entity Types", ImVec2(216, 0), childFlags);
	// Empty entity creator
	{
		if (ImGui::Button("Empty Entity", ImVec2(200, 35)))
			ImGui::OpenPopup("Empty Entity Creator");

		if (ImGui::BeginPopupModal("Empty Entity Creator", NULL))
		{
			static char entityName[64] = "Empty Entity";

			// Set Entity Name
			{
				ImGui::Text("Entity Name:");
				ImGui::SameLine();
				ImGui::InputText("##EntityName", entityName, sizeof(entityName));
			}

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				// Create entity with given parameters.

				const BoundingOrientedBox bounds = { {0,0,0}, {0.25f, 0.25f, 0.25f}, {0,0,0,1} };

				if (!CreateEntity(&ent, entityName, bounds, true))
				{
					ErrMsg("Failed to create object!");
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::EndChild();
					ImGui::TreePop();
					return false;
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	// Mesh Creator
	{
		if (ImGui::Button("Mesh", ImVec2(200, 35)))
			ImGui::OpenPopup("Mesh Creator");

		if (ImGui::BeginPopupModal("Mesh Creator", NULL))
		{
			// Set entity parameters
			static char entityName[64] = "Mesh Entity";
			static char meshName[64] = "";
			static char textureName[128] = "";
			static char normalName[128] = "";
			static char specularName[128] = "";
			static char ambientName[128] = "";
			static char samplerName[128] = "";
			static char vsName[128] = "";
			static char psName[128] = "";

			static UINT meshIndex = 0;
			static Material meshMaterial = Material::MakeMat(
				_content->GetTextureID("Tex_Fallback")
			);

			// Set Entity Name
			{
				ImGui::Text("Entity Name:");
				ImGui::SameLine();
				ImGui::InputText("##EntityName", entityName, sizeof(entityName));
			}

			// Set Mesh
			{
				ImGui::Text("Mesh Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##MeshName", meshName, sizeof(meshName)))
					meshIndex = _content->GetMeshID(std::format("Mesh_{}", meshName));

				ImGui::SetItemTooltip("Reference name of the mesh you want to use, not including the 'Mesh_' prefix.");
			}

			// Set Texture
			{
				ImGui::Text("Texture Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##TextureName", textureName, sizeof(textureName)))
					meshMaterial.textureID = _content->GetTextureID(std::format("Tex_{}", textureName));

				ImGui::SetItemTooltip("Reference name of the diffuse texture you want to use, not including the 'Tex_' prefix.");
			}

			// Set Normal Map
			{
				ImGui::Text("Normal Map Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##NormalName", normalName, sizeof(normalName)))
				{
					meshMaterial.normalID = _content->GetTextureMapID(std::format("TexMap_{}_Normal", normalName));
					if (meshMaterial.normalID == CONTENT_NULL)
						meshMaterial.normalID = _content->GetTextureMapID(std::format("TexMap_{}", normalName));
				}

				ImGui::SetItemTooltip("Reference name of the normal map you want to use, not including the 'TexMap_' prefix or the '_Normal' suffix.");
			}

			// Set Specular Map
			{
				ImGui::Text("Specular Map Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##SpecularName", specularName, sizeof(specularName)))
				{
					meshMaterial.specularID = _content->GetTextureMapID(std::format("TexMap_{}_Specular", specularName));
					if (meshMaterial.specularID == CONTENT_NULL)
						meshMaterial.specularID = _content->GetTextureMapID(std::format("TexMap_{}", specularName));
				}

				ImGui::SetItemTooltip("Reference name of the specular map you want to use, not including the 'TexMap_' prefix or the '_Specular' suffix.");
			}

			// Set Ambient Map
			{
				ImGui::Text("Ambient Texture Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##AmbientName", ambientName, sizeof(ambientName)))
				{
					meshMaterial.ambientID = _content->GetTextureID(std::format("Tex_{}_Ambient", ambientName));
					if (meshMaterial.ambientID == CONTENT_NULL)
						meshMaterial.ambientID = _content->GetTextureID(std::format("Tex_{}", ambientName));
				}

				ImGui::SetItemTooltip("Reference name of the ambient texture you want to use, not including the 'Tex_' prefix or the '_Ambient' suffix.");
			}

			// Set Sampler
			{
				ImGui::Text("Sampler Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##SamplerName", samplerName, sizeof(samplerName)))
					meshMaterial.samplerID = _content->GetSamplerID(std::format("SS_{}", samplerName));

				ImGui::SetItemTooltip("Reference name of the sampler you want to use, not including the 'SS_' prefix.");
			}

			// Set Vertex Shader
			{
				ImGui::Text("Vertex Shader Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##VertexShaderName", vsName, sizeof(vsName)))
					meshMaterial.vsID = _content->GetShaderID(std::format("VS_{}", vsName));

				ImGui::SetItemTooltip("Reference name of the vertex shader you want to use, not including the 'VS_' prefix.");
			}

			// Set Pixel Shader
			{
				ImGui::Text("Pixel Shader Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##PixelShaderName", psName, sizeof(psName)))
					meshMaterial.psID = _content->GetShaderID(std::format("PS_{}", psName));

				ImGui::SetItemTooltip("Reference name of the pixel shader you want to use, not including the 'PS_' prefix.");
			}

			static bool shadowCaster = true;
			ImGui::Checkbox("Cast Shadows", &shadowCaster);

			ImGui::Separator();
			static bool doCreate = false;
			static Transform *copyTransform = nullptr;

			if (doCreate || ImGui::Button("Create", ImVec2(120, 0)))
			{
				doCreate = false;
				bool isTransparent = meshMaterial.textureID >= _content->GetTextureID("Tex_Transparent");

				// Create entity with given parameters.
				if (!CreateMeshEntity(&ent, entityName, meshIndex, meshMaterial, isTransparent, shadowCaster))
				{
					ErrMsg("Failed to create mesh entity!");
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::EndChild();
					ImGui::TreePop();
					return false;
				}

				if (copyTransform)
				{
					ent->GetTransform()->SetMatrix(copyTransform->GetMatrix(World), World);
					copyTransform = nullptr;
				}

				// Reset inputs
				strcpy_s(entityName, "Mesh Entity");
				meshName[0] = '\0';
				textureName[0] = '\0';
				normalName[0] = '\0';
				specularName[0] = '\0';
				samplerName[0] = '\0';
				vsName[0] = '\0';
				psName[0] = '\0';

				meshIndex = 0;
				meshMaterial = Material::MakeMat(
					_content->GetTextureID("Tex_Fallback")
				);
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Copy Selected", ImVec2(120, 0)))
			{
				int selected = _debugPlayer->GetSelection();

				if (selected >= 0)
				{
					Entity *selectedEnt = _sceneHolder.GetEntity(selected);

					MeshBehaviour *selectedMesh;
					if (selectedEnt->GetBehaviourByType<MeshBehaviour>(selectedMesh))
					{
						copyTransform = selectedEnt->GetTransform();
						meshIndex = selectedMesh->GetMesh();
						meshMaterial = *selectedMesh->GetMaterial();
						doCreate = true;
					}
					else
					{
						ErrMsg("Selected entity is not a mesh entity!");
					}
				}
				else
				{
					ErrMsg("No selection found!");
				}
			}
			ImGui::SetItemDefaultFocus();
			if (ImGui::Button("Close", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	// Light Creator
	{
		if (ImGui::Button("Light", ImVec2(200, 35)))
			ImGui::OpenPopup("Light Creator");

		if (ImGui::BeginPopupModal("Light Creator", NULL))
		{
			// Set entity parameters
			static char entityName[64] = "Light Entity";
			{
				ImGui::Text("Entity Name:");
				ImGui::SameLine();
				ImGui::InputText("##EntityName", entityName, sizeof(entityName));
			}

			static bool isSpotlight = true;
			static bool shadowCaster = false;
			static bool isOrtho = false;

			static float color[3] = { 1.0f, 1.0f, 1.0f };
			static float intensity = 1.0f;
			static float falloff = 1.0f;
			static float angle = 45.0f;

			ImGui::Checkbox("Spotlight", &isSpotlight);
			ImGui::Checkbox("Cast Shadows", &shadowCaster);
			if (isSpotlight)
			ImGui::Checkbox("Orthographic", &isOrtho);

			ImGui::ColorEdit3("Color", color);
			ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.001f);
			ImGui::DragFloat("Falloff", &falloff, 0.05f, 0.001f);

			if (isSpotlight)
			{
				if (isOrtho)
					ImGui::DragFloat("Width", &angle, 0.05f, 0.01f);
				else
					ImGui::SliderFloat("Angle", &angle, 0.05f, shadowCaster ? 179.99f : 359.99f);
			}


			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				// Create entity with given parameters.

				XMFLOAT3 finalColor = { color[0] * intensity, color[1] * intensity, color[2] * intensity };

				if (isSpotlight)
				{
					//float finalAngle = isOrtho ? angle : angle * DEG_TO_RAD;
					float finalAngle = angle;

					if (shadowCaster)
					{
						if (!CreateSpotLightEntity(&ent, entityName, finalColor, falloff, finalAngle, isOrtho))
						{
							ErrMsg("Failed to create spotlight entity!");
							ImGui::CloseCurrentPopup();
							ImGui::EndPopup();
							ImGui::EndChild();
							ImGui::TreePop();
							return false;
						}
					}
					else
					{
						if (!CreateSimpleSpotLightEntity(&ent, entityName, finalColor, falloff, finalAngle, isOrtho))
						{
							ErrMsg("Failed to create simple spotlight entity!");
							ImGui::CloseCurrentPopup();
							ImGui::EndPopup();
							ImGui::EndChild();
							ImGui::TreePop();
							return false;
						}
					}
				}
				else
				{
					if (shadowCaster)
					{
						if (!CreatePointLightEntity(&ent, entityName, finalColor, falloff))
						{
							ErrMsg("Failed to create pointlight entity!");
							ImGui::CloseCurrentPopup();
							ImGui::EndPopup();
							ImGui::EndChild();
							ImGui::TreePop();
							return false;
						}
					}
					else
					{
						if (!CreateSimplePointLightEntity(&ent, entityName, finalColor, falloff))
						{
							ErrMsg("Failed to create simple pointlight entity!");
							ImGui::CloseCurrentPopup();
							ImGui::EndPopup();
							ImGui::EndChild();
							ImGui::TreePop();
							return false;
						}
					}
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	// Breadcrumb Pile Creator
	{
		if (ImGui::Button("Breadcrumb Pile", ImVec2(200, 35)))
			ImGui::OpenPopup("Breadcrumb Pile Creator");

		if (ImGui::BeginPopupModal("Breadcrumb Pile Creator", NULL))
		{
			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				// Create entity with given parameters.

				const BoundingOrientedBox bounds = { {0,0,0}, {0.25f, 0.25f, 0.25f}, {0,0,0,1} };

				if (!CreateEntity(&ent, "Breadcrumb Pile", bounds, true))
				{
					ErrMsg("Failed to create object!");
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::EndChild();
					ImGui::TreePop();
					return false;
				}

				BreadcrumbPileBehaviour *behaviour = new BreadcrumbPileBehaviour();
				if (!behaviour->Initialize(ent))
				{
					ErrMsg("Failed to create breadcrumb pile entity!");
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::EndChild();
					ImGui::TreePop();
					return false;
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	// Lantern Creator
	{
		if (ImGui::Button("Lantern", ImVec2(200, 35)))
			ImGui::OpenPopup("Lantern Creator");

		if (ImGui::BeginPopupModal("Lantern Creator", NULL))
		{
			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				if (!CreateLanternEntity(&ent))
				{
					ErrMsg("Failed to create object!");
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::EndChild();
					ImGui::TreePop();
					return false;
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	// Sound Creator
	{
		if (ImGui::Button("Sound", ImVec2(200, 50)))
			ImGui::OpenPopup("Sound Creator");

		if (ImGui::BeginPopupModal("Sound Creator", NULL))
		{
			// Set entity parameters
			static char entityName[64] = "Sound Entity";
			{
				ImGui::Text("Entity Name:");
				ImGui::SameLine();
				ImGui::InputText("##EntityName", entityName, sizeof(entityName));
			}
			static std::string soundName = "";
			static float volume = 1.0f;
			static float distanceScaler = 50.0f;
			static float reverbScaler = 1.0f;
			static bool loop = false;
			static bool foundFile = false;
			static int amountOfSounds = 1;

			{
				ImGui::Text("Sound Name:");
				ImGui::SameLine();
				if (ImGui::InputText("##SoundName", &soundName))
				{
					std::string fileName = std::format("Content\\Sounds\\{}.wav", soundName);
					struct stat buffer;   
					foundFile = stat(fileName.c_str(), &buffer) == 0;
				}
				ImGui::SetItemTooltip("Name of the sound file you want to use, located in Content/Sounds/.");

				if (ImGui::Button("Browse"))
				{
					const char *filterPatterns[] = { "*.wav" };
					const char *selectedFiles = tinyfd_openFileDialog(
						"Open File",
						"Content\\Sounds\\",
						1,
						filterPatterns,
						"Supported Files",
						0
					);

					if (selectedFiles)
					{
						std::string fileString = selectedFiles;

						std::vector<std::string> filePaths;
						std::stringstream ss(fileString);
						std::string filePath;
						while (std::getline(ss, filePath, '|'))
						{
							filePaths.push_back(filePath);
						}

						if (filePaths.size() > 0)
						{
							soundName = filePaths[0].substr(filePaths[0].find_last_of("\\") + 1);
							soundName = soundName.substr(0, soundName.find_last_of("."));

							std::string fileName = std::format("Content\\Sounds\\{}.wav", soundName);
							struct stat buffer;   
							foundFile = stat(fileName.c_str(), &buffer) == 0;
						}
					}
				}
			}

			ImGui::DragFloat("Volume", &volume, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Distance", &distanceScaler, 0.1f);
			ImGui::DragFloat("Reverb", &reverbScaler, 0.1f);
			ImGui::DragInt("Amount of Sounds", &amountOfSounds, 1, 1, 100);
			ImGui::Checkbox("Loop", &loop);

			if (!foundFile)
			{
				ImGui::BeginDisabled(true);
			}
			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				std::string foo(entityName);
				for (int i = 0; i < amountOfSounds; i++)
				{
					std::string temp = foo;
					if (amountOfSounds > 1)
						temp += std::format("{}", i+1);
					strncpy_s(entityName, sizeof(entityName), temp.c_str(), sizeof(entityName));
					entityName[sizeof(entityName) - 1] = '\0';
					// Create entity with given parameters.
					if (!CreateSoundEmitterEntity(&ent, entityName, soundName, loop, volume, distanceScaler, reverbScaler))
					{
						ErrMsg("Failed to create sound entity!");
						ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
						ImGui::EndChild();
						ImGui::TreePop();
						return false;
					}
				}
				entityName[0] = '\0';
				ImGui::CloseCurrentPopup();
			}
			if (!foundFile)
			{
				ImGui::EndDisabled();
				ImGui::SetItemTooltip("File could not be found.");
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Close", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}


	if (ent)
	{
		if (positionWithCursor)
		{
			_debugPlayer->SetSelection(-1);
			_debugPlayer->PositionWithCursor(ent);
		}
		else
		{
			_debugPlayer->SetSelection(ent->GetID());
		}
	}

	ImGui::EndChild();

	return true;
}
bool Scene::RenderEntityUI(UINT id)
{
	Entity *ent = _sceneHolder.GetEntityByID(id);
	int entIndex = _sceneHolder.GetEntityIndex(ent);

	ImGui::Text(std::format("Index: {}", entIndex).c_str());
	ImGui::Text(std::format("ID: {}", id).c_str());

	if (entIndex < 0)
		return true;
	
	if (!ent)
	{
		ImGui::Text("Entity: NULL");
		return true;
	}

	ImGui::SameLine();
	ImGui::Text(std::format("Entity: {}", ent->GetName()).c_str());

	Entity *parent = ent->GetParent();
	if (parent)
	{
		UINT parentIndex = _sceneHolder.GetEntityIndex(parent);

		ImGui::Text("Parent: ");
		ImGui::SameLine();
		if (ImGui::SmallButton(std::format("{} ({})", parent->GetName(), parentIndex).c_str()))
			_debugPlayer->SetSelection(parentIndex);

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.55f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.65f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.75f, 0.8f));
		if (ImGui::SmallButton("X"))
		{
			ent->SetParent(parent->GetParent(), _debugPlayer->GetEditSpace() == World);
		}
		ImGui::PopStyleColor(3);
	}
	else
	{
		ImGui::Text("Parent: None");
	}

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.55f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.65f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.6f, 0.75f, 0.7f));
	// check if entity is undocked
	auto dockIter = std::find(_undockedEntities.begin(), _undockedEntities.end(), id);
	if (dockIter != _undockedEntities.end()) 
	{ 
		// if undocked, show dock button
		if (ImGui::Button("Dock", { 60, 20 }))
			_undockedEntities.erase(dockIter);
	}
	else if (ImGui::Button("Undock", { 60, 20 })) // if docked, show undock button
	{ 
		_undockedEntities.push_back(id);
	}
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.3f, 0.55f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.3f, 0.65f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.3f, 0.75f, 0.7f));
	if (ImGui::Button("Copy", { 60, 20 }))
	{
		// Copy by serializing and deserializing the entity
		std::string code = "";
		if (!SerializeEntity(&code, ent, true))
		{
			ErrMsg("Failed to serialize entity!");
			ImGui::PopStyleColor(3);
			return false;
		}

		Entity *ent = nullptr;
		if (!DeserializeEntity(code, &ent))
		{
			ErrMsg("Failed to deserialize entity!");
			ImGui::PopStyleColor(3);
			return false;
		}

		if (ent)
			_debugPlayer->SetSelectionID(ent->GetID());
	}
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.55f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.65f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.75f, 0.7f));
	if (ImGui::Button("Delete", { 60, 20 }))
	{
		if (!_sceneHolder.RemoveEntity(entIndex))
		{
			ErrMsg("Failed to remove entity!");
			ImGui::PopStyleColor(3);
			return false;
		}

		_debugPlayer->SetSelection(-1);
	}
	ImGui::PopStyleColor(3);

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.15f, 0.55f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.15f, 0.65f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.15f, 0.75f, 0.7f));
	if (_debugPlayer->HasDuplicateBind(id))
	{
		if (ImGui::Button("Clear Duplicate Bind", { 180, 20 }))
			_debugPlayer->RemoveDuplicateBind(id);

		ImGui::PopStyleColor(3);

		KeyCode keyCode = _debugPlayer->GetDuplicateBind(id);
		std::string keyCodeName = "?";

		for (const auto& [key, value] : KeyCodeNames)
			if (value == keyCode)
				keyCodeName = key;

		ImGui::Text(("Duplicate Bind: " + keyCodeName).c_str());
	}
	else
	{
		if (_debugPlayer->IsAssigningDuplicateToKey(id))
		{
			if (ImGui::Button("Cancel Duplicate Bind", { 180, 20 }))
				_debugPlayer->AssignDuplicateToKey(-1);
			ImGui::PopStyleColor(3);

			ImGui::Text("Press the key you want to assign...");
		}
		else if (ImGui::Button("Add Duplicate Bind", { 180, 20 }))
		{
			_debugPlayer->AssignDuplicateToKey(id);
			ImGui::PopStyleColor(3);
		}
		else
			ImGui::PopStyleColor(3);
	}


	ImGui::Text("Space:");
	ImGui::SameLine();
	if (ImGui::Button((_debugPlayer->GetEditSpace() == World) ? "World" : "Local"))
		_debugPlayer->SetEditSpace((_debugPlayer->GetEditSpace() == World) ? Local : World);

	float inputWidth = 96.0f;
	bool isChanged = false;

	ImGuiInputTextFlags floatInputFlags = ImGuiInputTextFlags_CharsDecimal;

	ImGui::PushID("Transform");
	if (ImGui::CollapsingHeader("Transform"))
	{
		ImGui::PushID("Position");
		{
			XMFLOAT3A entPos = ent->GetTransform()->GetPosition(_debugPlayer->GetEditSpace());

			ImGui::Text("Position: ");
			ImGui::SameLine();
			if (ImGui::InputFloat3("", &entPos.x, "%.4f", floatInputFlags))
			{
				isChanged = true;
			}

			if (isChanged)
				ent->GetTransform()->SetPosition(entPos, _debugPlayer->GetEditSpace());
		}
		ImGui::PopID();

		ImGui::PushID("Rotation");
		{
			isChanged = false;
			XMFLOAT3A entRot = ent->GetTransform()->GetEuler(_debugPlayer->GetEditSpace());
			XMFLOAT3A entRotDeg = {
				entRot.x * RAD_TO_DEG,
				entRot.y * RAD_TO_DEG,
				entRot.z * RAD_TO_DEG
			};

			ImGui::Text("Rotation: ");
			ImGui::SameLine();
			if (ImGui::InputFloat3("", &entRotDeg.x, "%.4f", floatInputFlags))
				isChanged = true;

			if (isChanged)
			{
				entRot = {
					entRotDeg.x * DEG_TO_RAD,
					entRotDeg.y * DEG_TO_RAD,
					entRotDeg.z * DEG_TO_RAD
				};

				ent->GetTransform()->SetEuler(entRot, _debugPlayer->GetEditSpace());
			}
		}
		ImGui::PopID();

		ImGui::PushID("Scale");
		{
			isChanged = false;
			XMFLOAT3A entScale = ent->GetTransform()->GetScale(_debugPlayer->GetEditSpace());

			ImGui::Text("Scale:    ");
			ImGui::SameLine();
			if (ImGui::InputFloat3("", &entScale.x, "%.4f", floatInputFlags))
				isChanged = true;

			if (isChanged)
				ent->GetTransform()->SetScale(entScale, _debugPlayer->GetEditSpace());
		}
		ImGui::PopID();

		ImGui::Separator();

		ImGui::PushID("Matrix");
		{
			isChanged = false;
			XMFLOAT4X4A entMat = ent->GetTransform()->GetMatrix(_debugPlayer->GetEditSpace());

			ImGui::Text("R1: ");
			ImGui::SameLine();
			ImGui::PushID("R1");
			if (ImGui::InputFloat4("", entMat.m[0], "%.3f", floatInputFlags))
				isChanged = true;
			ImGui::PopID();

			ImGui::Text("R2: ");
			ImGui::SameLine();
			ImGui::PushID("R2");
			if (ImGui::InputFloat4("", entMat.m[1], "%.3f", floatInputFlags))
				isChanged = true;
			ImGui::PopID();

			ImGui::Text("R3: ");
			ImGui::SameLine();
			ImGui::PushID("R3");
			if (ImGui::InputFloat4("", entMat.m[2], "%.3f", floatInputFlags))
				isChanged = true;
			ImGui::PopID();

			ImGui::Text("R4: ");
			ImGui::SameLine();
			ImGui::PushID("R4");
			if (ImGui::InputFloat4("", entMat.m[3], "%.3f", floatInputFlags))
				isChanged = true;
			ImGui::PopID();

			if (isChanged)
				ent->GetTransform()->SetMatrix(entMat, _debugPlayer->GetEditSpace());
		}
		ImGui::PopID();

		ImGui::PushID("Transform Catalogue");
		if (ImGui::CollapsingHeader("Transform Catalogue"))
		{
			ImGuiChildFlags childFlags = 0;
			childFlags |= ImGuiChildFlags_Border;
			childFlags |= ImGuiChildFlags_ResizeY;

			ImGui::BeginChild("Transform Catalogue", ImVec2(0, 150), childFlags);
			ImGui::Text("Note: Getters write to input. Setters read from input.");
			ImGui::Text("Methods with an additional float parameter use the w-value.");
			ImGui::Text("The reference space selected above is used for all methods.");

			static XMFLOAT4A parameter = { 0, 0, 0, 0 };
			ImGui::Text("Input: ");
			ImGui::SameLine();
			ImGui::InputFloat4("", &parameter.x, "%.4f", floatInputFlags);

			Transform *trans = ent->GetTransform();
			XMFLOAT4A *vec4 = &parameter;
			XMFLOAT3A *vec3 = reinterpret_cast<XMFLOAT3A *>(&parameter);
			float *vec1 = &parameter.w;

			if (ImGui::TreeNode("Getters"))
			{
				if (ImGui::Button("Vec3 GetRight()"))
				{
					XMFLOAT3A vec = trans->GetRight(_debugPlayer->GetEditSpace());
					std::memcpy(vec3, &vec, sizeof(XMFLOAT3));
				}

				if (ImGui::Button("Vec3 GetUp()"))
				{
					XMFLOAT3A vec = trans->GetUp(_debugPlayer->GetEditSpace());
					std::memcpy(vec3, &vec, sizeof(XMFLOAT3));
				}

				if (ImGui::Button("Vec3 GetForward()"))
				{
					XMFLOAT3A vec = trans->GetForward(_debugPlayer->GetEditSpace());
					std::memcpy(vec3, &vec, sizeof(XMFLOAT3));
				}

				if (ImGui::Button("Vec3 GetPosition()"))
				{
					XMFLOAT3A vec = trans->GetPosition(_debugPlayer->GetEditSpace());
					std::memcpy(vec3, &vec, sizeof(XMFLOAT3));
				}

				if (ImGui::Button("Quat GetRotation()"))
				{
					XMFLOAT4A vec = trans->GetRotation(_debugPlayer->GetEditSpace());
					std::memcpy(vec4, &vec, sizeof(XMFLOAT4));
				}

				if (ImGui::Button("Vec3 GetScale()"))
				{
					XMFLOAT3A vec = trans->GetScale(_debugPlayer->GetEditSpace());
					std::memcpy(vec3, &vec, sizeof(XMFLOAT3));
				}

				if (ImGui::Button("Vec3 GetEuler()"))
				{
					XMFLOAT3A vec = trans->GetEuler(_debugPlayer->GetEditSpace());
					vec = {
						vec.x * RAD_TO_DEG,
						vec.y * RAD_TO_DEG,
						vec.z * RAD_TO_DEG
					};
					std::memcpy(vec3, &vec, sizeof(XMFLOAT3));
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Setters"))
			{
				if (ImGui::Button("SetPosition(Vec3)"))
					trans->SetPosition(*vec3, _debugPlayer->GetEditSpace());

				if (ImGui::Button("SetRotation(Quat)"))
					trans->SetRotation(*vec4, _debugPlayer->GetEditSpace());

				if (ImGui::Button("SetScale(Vec3)"))
					trans->SetScale(*vec3, _debugPlayer->GetEditSpace());

				if (ImGui::Button("Move(Vec3)"))
					trans->Move(*vec3, _debugPlayer->GetEditSpace());

				if (ImGui::Button("Rotate(Vec3)"))
				{
					XMFLOAT3A vec = {
						vec3->x * DEG_TO_RAD,
						vec3->y * DEG_TO_RAD,
						vec3->z * DEG_TO_RAD
					};
					trans->Rotate(vec, _debugPlayer->GetEditSpace());
				}

				if (ImGui::Button("Scale(Vec3)"))
					trans->Scale(*vec3, _debugPlayer->GetEditSpace());

				if (ImGui::Button("MoveRelative(Vec3)"))
					trans->MoveRelative(*vec3, _debugPlayer->GetEditSpace());

				if (ImGui::Button("RotateAxis(Vec3, float)"))
					trans->RotateAxis(*vec3, *vec1 * DEG_TO_RAD, _debugPlayer->GetEditSpace());

				if (ImGui::Button("RotateQuaternion(Quat)"))
					trans->RotateQuaternion(*vec4, _debugPlayer->GetEditSpace());

				if (ImGui::Button("SetEuler(Vec3)"))
				{
					XMFLOAT3A vec = {
						vec3->x * DEG_TO_RAD,
						vec3->y * DEG_TO_RAD,
						vec3->z * DEG_TO_RAD
					};
					trans->SetEuler(vec, _debugPlayer->GetEditSpace());
				}

				if (ImGui::Button("RotatePitch(float)"))
					trans->RotatePitch(*vec1 * DEG_TO_RAD);

				if (ImGui::Button("RotateYaw(float)"))
					trans->RotateYaw(*vec1 * DEG_TO_RAD);

				if (ImGui::Button("RotateRoll(float)"))
					trans->RotateRoll(*vec1 * DEG_TO_RAD);

				ImGui::TreePop();
			}

			ImGui::EndChild();
		}
		ImGui::PopID();

		ImGui::Separator();
	}
	ImGui::PopID();

	ImGui::PushID("Entity UI");
	if (ImGui::CollapsingHeader("Entity & Behaviours"))
	{
		if (!ent->InitialRenderUI())
		{
			ImGui::PopID();
			ErrMsg("Failed to render entity UI!");
			return false;
		}
	}
	ImGui::PopID();

	return true;
}
bool Scene::RenderEntityHierarchyUI(Entity *root, UINT depth, std::string search)
{
	std::string entName = root->GetName();
	UINT entIndex = _sceneHolder.GetEntityIndex(root);
	UINT entID = root->GetID();

	std::string entNameLower = entName;
	std::transform(entNameLower.begin(), entNameLower.end(), entNameLower.begin(), ::tolower);
	if (search != "" && entNameLower.find(search) == std::string::npos)
		return true;

	bool isSelected = false;
	if (_debugPlayer->GetSelection() >= 0)
	{
		isSelected = (entIndex == _debugPlayer->GetSelection());
		if (isSelected)
			entName = std::format("[{}]", entName);
	}

	std::string indent = "";
	for (UINT i = 1; i < depth; i++)
		indent += "|  ";
	if (depth > 0)
		indent += "|--";

	ImGui::Text(indent.c_str());
	ImGui::SameLine();

	// Entity Drag & Drop field
	{
		ImGui::PushID(("Ent:" + std::to_string(entID)).c_str());

		if (isSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.1f, 0.55f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.1f, 0.65f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.1f, 0.75f, 0.7f));
		}

		if (ImGui::Button(std::format("{}", entName).c_str()))
		{
			_debugPlayer->SetSelection(entIndex);
		}

		if (isSelected)
		{
			ImGui::PopStyleColor(3);
		}

		// Our buttons are both drag sources and drag targets here!
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &entID, sizeof(UINT));

			ImGui::Text(std::format("{}", entName).c_str());

			// Display preview (could be anything, e.g. when dragging an image we could decide to display
			// the filename and a small preview of the image, etc.)
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
			{
				IM_ASSERT(payload->DataSize == sizeof(UINT));
				UINT payloadID = *(const UINT *)payload->Data;

				if (payloadID != entID)
				{
					Entity *payloadEnt = _sceneHolder.GetEntityByID(payloadID);

					if (payloadEnt)
					{
						if (!root->IsChildOf(payloadEnt))
							payloadEnt->SetParent(root, _debugPlayer->GetEditSpace() == World);
					}
				}

			}
			ImGui::EndDragDropTarget();
		}

		ImGui::PopID();
	}

	const std::vector<Entity *> *children = root->GetChildren();
	for (auto &child : *children)
	{
		if (!child)
			continue;

		if (!RenderEntityHierarchyUI(child, depth + 1))
		{
			ErrMsg("Failed to render entity hierarchy UI!");
			return false;
		}
	}

	return true;
}
#endif
#pragma endregion

#pragma region Serialization
bool Scene::Serialize(std::string *code) const
{
#ifdef DEBUG_BUILD
	// Copy the previous file to a backup folder (to prevent overwriting important work).
	{
		std::ifstream prevFile(std::format("Content/Saves/{}.txt", _saveFile));

		if (prevFile)
		{
			auto t = std::time(nullptr);
			tm tm;
			localtime_s(&tm, &t);

			std::ostringstream oss;
			oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
			std::string timestamp = oss.str();

			std::ofstream backupFile(std::format("Content/Saves/Backups/{} ({}).txt", _saveFile, timestamp));
			backupFile << prevFile.rdbuf();
			prevFile.close();
			backupFile.close();
		}
	}
#endif

	// Serialize the entities
	std::string fileName;
#ifdef EDIT_MODE
	fileName = "MapSave";
#else
	fileName = "GameSave";
#endif
	std::ofstream file(std::format("Content/Saves/{}.txt", fileName), std::ios::out);
	if (!file) 
	{
		ErrMsg("Could not save game!");
		return false;
	}

	if (Input::GetInstance()->GetKey(KeyCode::RightShift) == KeyState::Held)
	{
		file << "";
		file.close();
		return true;
	}

	std::vector<Entity*> entities;
	_sceneHolder.GetEntities(entities);
	for (auto entity : entities) 
	{
		if (!SerializeEntity(code, entity))
		{
			ErrMsg(std::format("Failed to serialize entity '{}'!", entity->GetName()));
			return false;
		}
	}

	file << *code;
	file.close();

	// Serialize the timeline manager
	code->clear();
	file = std::ofstream("Content/Saves/Sequences.txt", std::ios::out);
	if (!file)
	{
		ErrMsg("Could not open sequences file.");
		return false;
	}

	if (!_timelineManager.Serialize(code))
	{
		ErrMsg("Failed to serialize timeline manager!");
		return false;
	}

	file << *code;
	file.close();

	return true;
}
bool Scene::SerializeEntity(std::string *code, Entity *entity, bool forceSerialize) const
{
	if (entity->IsSerializable() || forceSerialize)
	{
		std::string entName = entity->GetName();
		UINT entID = entity->GetID();
		Entity *parentEntity = entity->GetParent();
		UINT parentID = parentEntity ? parentEntity->GetID() : -1;
		Transform *entTransform = entity->GetTransform();
		XMFLOAT3A pos = entTransform->GetPosition();
		XMFLOAT3A euler = entTransform->GetEuler();
		XMFLOAT3A scale = entTransform->GetScale();
		UINT hasVolume = _sceneHolder.IsEntityIncludedInTree(entity);

		// "[Name] ID:[ID] [parent] [hasVolume]([pos.x] [pos.y] [pos.z] [euler.x] [euler.y] [euler.z] [scale.x] [scale.y] [scale.z] )<"

		code->append(std::format("{} ID:{} {} {}({} {} {} {} {} {} {} {} {} )<",
			entName,	entID,		parentID,		hasVolume,
			pos.x,		pos.y,		pos.z,
			euler.x,	euler.y,	euler.z,
			scale.x,	scale.y,	scale.z
		));

		UINT count = entity->GetBehaviourCount();
		for (int i = 0; i < count; i++) 
		{
			Behaviour *bev = entity->GetBehaviour(i);

			if (!bev->InitialSerialize(code))
			{
				ErrMsg(std::format("Failed to serialize behaviour '{}'!", bev->GetName()));
				return false;
			}
		}
		code->append("> \n"); 
	}
	return true;
}
bool Scene::Deserialize()
{
	std::ifstream file(std::format("Content/Saves/{}.txt", _saveFile));
	std::string line;

	while (std::getline(file, line)) 
	{
		if (!DeserializeEntity(line))
		{
			ErrMsg(std::format("Failed to deserialize entity with line '{}'!", line));
			return false;
		}
	}

	PostDeserialize();

	std::ifstream seqFile(std::format("Content/Saves/{}.txt", "Sequences"));
	uintmax_t fileSize = 0;
	if (seqFile.is_open())
	{
		fileSize = std::filesystem::file_size(std::format("Content/Saves/{}.txt", "Sequences"));
	}

	if (fileSize > 0)
	{
		if (!_timelineManager.Deserialize())
		{
			ErrMsg("Failed to deserialize timeline manager!");
			return false;
		}
	}

	return true;
}
bool Scene::DeserializeEntity(const std::string &line, std::optional<Entity**> out)
{
	// Find the name of the object
	size_t space = line.find("ID:");
	std::string name = line.substr(0, space - 1);

	// Find the serialized ID
	size_t colon = line.find(":");
	size_t parentSpace = line.find(" ", space);
	UINT deserializedID = std::stoul(line.substr(colon + 1, parentSpace));

	// Find entity attributes
	size_t bracket = line.find("<");
	size_t parenthesis = line.find("(");

	std::string entline = line.substr(parenthesis + 1, bracket - parenthesis - 2);

	// Convert entity attributes to floats
	std::vector<float> entAttributes;
	std::istringstream stream(entline);
	std::string value;
	while (stream >> value) // Automatically handles spaces correctly
	{
		float attribute = stof(value);
		entAttributes.push_back(attribute);
	}

	// Check parent
	std::string parent = line.substr(parentSpace + 1, parenthesis - parentSpace - 3);
	UINT parentID = (UINT)std::stoul(parent);

	// Check volume
	std::string testVolume = line.substr(parenthesis - 1, 1);
	bool hasVolume = stoi(testVolume);

	// Create the entity
	Entity* ent = nullptr;
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .1f,.1f,.1f }, { 0,0,0,1 });

	if (!CreateEntity(&ent, name, bounds, hasVolume)) 
	{
		ErrMsg(std::format("Failed to create entity '{}'!", name));
		return false;
	}

	ent->SetDeserializedID(deserializedID);

	ent->GetTransform()->SetPosition({ entAttributes[0],entAttributes[1] ,entAttributes[2] });
	ent->GetTransform()->SetEuler({ entAttributes[3], entAttributes[4], entAttributes[5] });
	ent->GetTransform()->SetScale({ entAttributes[6], entAttributes[7], entAttributes[8] });

	if (parentID != CONTENT_NULL) 
	{
		ent->SetParent(_sceneHolder.GetEntityByDeserializedID(parentID), false);
	}


	// substring with all the behaviours data 
	std::string behavioursLine = line.substr(bracket);
	size_t behavioursSize = behavioursLine.find(">");

	// find parenthesis for one behaviour
	space = 0;
	bool behavioursLeft = behavioursSize > 1;

	while (behavioursLeft)
	{
		parenthesis = behavioursLine.find(")", space + 1);
		std::string currentBev = behavioursLine.substr(space + 1, parenthesis);
		size_t bevLineParenthesis = currentBev.find(")");
		std::string bevName = currentBev.substr(0, currentBev.find("("));
		size_t size = bevName.size();
		std::string bevAttributes = currentBev.substr(size + 1, bevLineParenthesis - size - 2);

		if (bevName == "MeshBehaviour")
		{
			MeshBehaviour *mesh = new MeshBehaviour();
			if (!mesh->Initialize(ent))
			{
				ErrMsg("Failed to bind mesh to entity!");
				return false;
			}

			if (!mesh->Deserialize(bevAttributes))
			{
				return false;
			}
		}
		else if (bevName == "PointLightBehaviour")
		{
			PointLightBehaviour *point = new PointLightBehaviour();
			if (!point->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!point->Initialize(ent))
			{
				ErrMsg("Failed to bind point light to entity!");
				return false;
			}
		}
		else if (bevName == "SimplePointLightBehaviour")
		{
			SimplePointLightBehaviour *simplePoint = new SimplePointLightBehaviour();
			if (!simplePoint->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!simplePoint->Initialize(ent))
			{
				ErrMsg("Failed to bind simple point light to entity!");
				return false;
			}
		}
		else if (bevName == "SpotLightBehaviour")
		{
			SpotLightBehaviour *spot = new SpotLightBehaviour();
			if (!spot->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!spot->Initialize(ent))
			{
				ErrMsg("Failed to bind spotlight to entity!");
				return false;
			}
		}
		else if (bevName == "SimpleSpotLightBehaviour")
		{
			SimpleSpotLightBehaviour *simpleSpot = new SimpleSpotLightBehaviour();
			if (!simpleSpot->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!simpleSpot->Initialize(ent))
			{
				ErrMsg("Failed to bind simple spotlight to entity!");
				return false;
			}
		}
		else if (bevName == "GraphNodeBehaviour")
		{
			GraphNodeBehaviour *beh = new GraphNodeBehaviour();
			if (!beh->Deserialize(bevAttributes))
			{
				ErrMsg("Failed to deserialize graph node entity!");
				return false;
			}

			if (!beh->Initialize(ent))
			{
				ErrMsg("Failed to bind graph node behaviour to entity!");
				return false;
			}
		}
		else if (bevName == "BreadcrumbBehaviour")
		{
			BreadcrumbBehaviour *beh = new BreadcrumbBehaviour();
			if (!beh->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!beh->Initialize(ent))
			{
				ErrMsg("Failed to bind crumb to entity!");
				return false;
			}
		}
		else if (bevName == "BreadcrumbPileBehaviour")
		{
			BreadcrumbPileBehaviour *beh = new BreadcrumbPileBehaviour();
			if (!beh->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!beh->Initialize(ent))
			{
				ErrMsg("Failed to bind crumb pile to entity!");
				return false;
			}
		}
		else if (bevName == "ExampleBehaviour")
		{
#ifdef DEBUG_BUILD
			ExampleBehaviour *example = new ExampleBehaviour();
			if (!example->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!example->Initialize(ent))
			{
				ErrMsg("Failed to bind example to entity!");
				return false;
			}
#endif
		}
		else if (bevName == "DebugPlayerBehaviour")
		{
#ifdef DEBUG_BUILD
			DebugPlayerBehaviour *player = new DebugPlayerBehaviour();
			if (!player->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!player->Initialize(ent))
			{
				ErrMsg("Failed to bind debug to entity!");
				return false;
			}

			_debugPlayer = player;
#endif
		}
		else if (bevName == "FlashlightBehaviour")
		{
			FlashlightBehaviour *flashlight = new FlashlightBehaviour();
			if (!flashlight->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!flashlight->Initialize(ent))
			{
				ErrMsg("Failed to bind flashlight to entity!");
				return false;
			}
		}
		else if (bevName == "CameraBehaviour")
		{
			CameraBehaviour *camera = new CameraBehaviour();
			if (!camera->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!camera->Initialize(ent))
			{
				ErrMsg("Failed to bind camera to entity!");
				return false;
			}

			if (ent->GetName() == "playerCamera")
			{
				PlayerMovementBehaviour *movementBehaviour = nullptr;
				_sceneHolder.GetEntityByName("Player Entity")->GetBehaviourByType<PlayerMovementBehaviour>(movementBehaviour);
				movementBehaviour->SetPlayerCamera(camera);
			}
		}
		else if (bevName == "InteractorBehaviour")
		{
			InteractorBehaviour *interactor = new InteractorBehaviour();
			if (!interactor->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!interactor->Initialize(ent))
			{
				ErrMsg("Failed to bind interactor to entity!");
				return false;
			}
		}
		else if (bevName == "InteractableBehaviour")
		{
			InteractableBehaviour *interactable = new InteractableBehaviour();
			if (!interactable->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!interactable->Initialize(ent))
			{
				ErrMsg("Failed to bind interactable to entity!");
				return false;
			}
		}
		else if (bevName == "HideBehaviour")
		{
			HideBehaviour *hide = new HideBehaviour();
			if (!hide->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!hide->Initialize(ent))
			{
				ErrMsg("Failed to bind hide to entity!");
				return false;
			}
		}
		else if (bevName == "PickupBehaviour")
		{
			PickupBehaviour *pickup = new PickupBehaviour();
			if (!pickup->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!pickup->Initialize(ent))
			{
				ErrMsg("Failed to bind pickup to entity!");
				return false;
			}
		}
		else if (bevName == "InventoryBehaviour")
		{
			InventoryBehaviour *beh = new InventoryBehaviour();
			if (!beh->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!beh->Initialize(ent))
			{
				ErrMsg("Failed to bind inventory to entity!");
				return false;
			}
		}
		else if (bevName == "PictureBehaviour")
		{
			PictureBehaviour *picture = new PictureBehaviour(false);
			if (!picture->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!picture->Initialize(ent))
			{
				ErrMsg("Failed to bind picture to entity!");
				return false;
			}
		}
		else if (bevName == "SoundBehaviour")
		{
			SoundBehaviour *sound = new SoundBehaviour();
			if (!sound->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!sound->Initialize(ent))
			{
				ErrMsg("Failed to bind sound to entity!");
				return false;
			}
		}
		else if (bevName == "PlayerMovementBehaviour")
		{
			PlayerMovementBehaviour *movement = new PlayerMovementBehaviour();
			if (!movement->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!movement->Initialize(ent))
			{
				ErrMsg("Failed to bind player movement to entity!");
				return false;
			}
		}
		else if (bevName == "PlayerViewBehaviour")
		{
			PlayerViewBehaviour *view = new PlayerViewBehaviour();
			if (!view->Initialize(ent))
			{
				ErrMsg("Failed to bind player view to entity!");
				return false;
			}

			if (!view->Deserialize(bevAttributes))
			{
				ErrMsg("Failed to deserialize player view!");
				return false;
			}
		}
		else if (bevName == "ColliderBehaviour")
		{
			ColliderBehaviour *collider = new ColliderBehaviour();
			if (!collider->Initialize(ent))
			{
				ErrMsg("Failed to bind player movement to entity!");
				return false;
			}

			if (!collider->Deserialize(bevAttributes))
			{
				return false;
			}

		}
		else if (bevName == "ExampleCollisionBehaviour")
		{
#ifdef DEBUG_BUILD
			ExampleCollisionBehaviour *exampleCol = new ExampleCollisionBehaviour();
			if (!exampleCol->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!exampleCol->Initialize(ent))
			{
				ErrMsg("Failed to bind player movement to entity!");
				return false;
			}
#endif
		}
		else if (bevName == "CameraItemBehaviour")
		{
			CameraItemBehaviour *cameraItem = new CameraItemBehaviour();
			//if (!cameraItem->Deserialize(bevAttributes))
			//{
			//	return false;
			//}

			if (!cameraItem->Initialize(ent))
			{
				ErrMsg("Failed to bind player movement to entity!");
				return false;
			}
		}
		else if (bevName == "SolidObjectBehaviour")
		{
			SolidObjectBehaviour *solid = new SolidObjectBehaviour();
			if (!solid->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!solid->Initialize(ent))
			{
				ErrMsg("Failed to bind solid object to entity!");
				return false;
			}
		}
		else if (bevName == "BillboardMeshBehaviour")
		{
			BillboardMeshBehaviour *billboard = new BillboardMeshBehaviour();
			if (!billboard->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!billboard->Initialize(ent))
			{
				ErrMsg("Failed to bind billboard object to entity!");
				return false;
			}
		}
		else if (bevName == "AmbientSoundBehaviour")
		{
			AmbientSoundBehaviour* ambient = new AmbientSoundBehaviour();
			if (!ambient->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!ambient->Initialize(ent))
			{
				ErrMsg("Failed to bind ambient sound to entity!");
				return false;
			}
		}
		else if (bevName == "MonsterBehaviour")
		{
			MonsterBehaviour *monster = new MonsterBehaviour();
			if (!monster->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!monster->Initialize(ent))
			{
				ErrMsg("Failed to bind monster to entity!");
				return false;
			}

			_monster = monster;
		}
		else if (bevName == "EndCutSceneBehaviour")
		{
			EndCutSceneBehaviour* cutscene = new EndCutSceneBehaviour();
			if (!cutscene->Deserialize(bevAttributes))
			{
				return false;
			}

			if (!cutscene->Initialize(ent))
			{
				ErrMsg("Failed to bind EndCutSceneBehaviour to entity!");
				return false;
			}
		}
		else
		{
			ErrMsg("Failed to deserialize " + bevName + " to " + name);
		}

		if (behavioursSize - parenthesis == 1)
		{
			behavioursLeft = false;
		}
		space = parenthesis;
	}

	if (out.has_value())
		(*(out.value())) = ent;

	return true;
}

void Scene::PostDeserialize()
{
	Entity *ent = _sceneHolder.GetEntityByName("Player Entity");
	if (ent)
	{
		InteractorBehaviour *interactor = nullptr;
		if (ent->GetBehaviourByType<InteractorBehaviour>(interactor))
			interactor->PostDeserialize();

		PlayerMovementBehaviour *movement = nullptr;
		if (ent->GetBehaviourByType<PlayerMovementBehaviour>(movement))
			movement->SetHeadTracker(_sceneHolder.GetEntityByName("Player Head Tracker"));
	}

	ent = _sceneHolder.GetEntityByName("Flashlight");
	if (ent)
	{
		FlashlightBehaviour *flashlight = nullptr;
		if (ent->GetBehaviourByType<FlashlightBehaviour>(flashlight))
			flashlight->PostDeserialize();
	}

	_graphManager.CompleteDeserialization();
}
#pragma endregion

#pragma region Getters & Setters
Game *Scene::GetGame() const
{
	return _game;
}

SceneHolder *Scene::GetSceneHolder()
{
	return &_sceneHolder;
}

TimelineManager* Scene::GetTimelineManager()
{
	return &_timelineManager;
}


ID3D11Device *Scene::GetDevice() const
{
	return _device;
}
ID3D11DeviceContext *Scene::GetContext() const
{
	return _context;
}

Content *Scene::GetContent() const
{
	return _content;
}
Graphics *Scene::GetGraphics() const
{
	return _graphics;
}
DebugDrawer *Scene::GetDebugDrawer() const
{
	return _graphics->GetDebugDrawer();
}

CollisionHandler *Scene::GetCollisionHandler()
{
	return &_collisionHandler;
}

GraphManager *Scene::GetGraphManager()
{
	return &_graphManager;
}
const Input *Scene::GetInput() const
{
	return _input;
}
std::string Scene::GetSaveFile() const
{
	return _saveFile;
}

Entity *Scene::GetPlayer() const
{
	return _player;
}
MonsterBehaviour *Scene::GetMonster() const
{
	return _monster;
}

ColliderBehaviour *Scene::GetTerrainBehaviour() const
{
	return _terrainBehaviour;
}
const Collisions::Terrain *Scene::GetTerrain() const
{
	return _terrain;
}

SoundEngine *Scene::GetSoundEngine()
{
	return &_soundEngine;
}
void Scene::SetSceneVolume(float volume)
{
	_soundEngine.SetVolume(volume);
}

std::vector<std::unique_ptr<Entity>> *Scene::GetGlobalEntities()
{
	return &_globalEntities;
}
SpotLightCollection *Scene::GetSpotlights() const
{
	return _spotlights.get();
}
PointLightCollection *Scene::GetPointlights() const
{
	return _pointlights.get();
}

#ifdef DEBUG_BUILD
DebugPlayerBehaviour *Scene::GetDebugPlayer() const
{
	return _debugPlayer;
}
void Scene::SetDebugPlayer(DebugPlayerBehaviour *debugPlayer)
{
	_debugPlayer = debugPlayer;
}
#endif

void Scene::SetViewCamera(CameraBehaviour *camera)
{
	_viewCamera = camera;
#ifdef DEBUG_BUILD
	if (_debugPlayer)
		_debugPlayer->SetCamera(camera);
#endif
}
CameraBehaviour *Scene::GetViewCamera()
{
	return _viewCamera;
}
CameraBehaviour *Scene::GetPlayerCamera()
{
	return _playerCamera;
}
CameraBehaviour *Scene::GetAnimationCamera()
{
	return _animationCamera;
}
#pragma endregion

#pragma region Entity Creation
bool Scene::CreateGlobalEntity(Entity **out, std::string name, const BoundingOrientedBox &bounds, bool hasVolume)
{
	*out = new Entity(0, bounds);
	if (!(*out)->Initialize(_device, this, name))
	{
		ErrMsg("Failed to initialize entity '" + name + "'!");
		return false;
	}

	return true;
}
bool Scene::CreateGlobalMeshEntity(Entity **out, const std::string &name, UINT meshID, const Material &material, bool isTransparent, bool shadowCaster, bool isOverlay)
{
	const BoundingOrientedBox bounds = _content->GetMesh(meshID)->GetBoundingOrientedBox();

	if (!CreateGlobalEntity(out, name, bounds, true))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	MeshBehaviour *mesh = new MeshBehaviour(bounds, meshID, &material, isTransparent, shadowCaster, isOverlay);

	if (!mesh->Initialize(*out))
	{
		ErrMsg("Failed to bind mesh to entity '" + name + "'!");
		return false;
	}

	return true;
}

bool Scene::CreateEntity(Entity **out, std::string name, const BoundingOrientedBox &bounds, bool hasVolume)
{
	*out = _sceneHolder.AddEntity(bounds, hasVolume);
	if (!(*out)->Initialize(_device, this, name))
	{
		ErrMsg("Failed to initialize entity '" + name + "'!");
		return false;
	}

	return true;
}

bool Scene::CreateMeshEntity(Entity **out, const std::string &name, UINT meshID, const Material &material, bool isTransparent, bool shadowCaster, bool isOverlay)
{
	const BoundingOrientedBox bounds = _content->GetMesh(meshID)->GetBoundingOrientedBox();

	if (!CreateEntity(out, name, bounds, true))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	MeshBehaviour *mesh = new MeshBehaviour(bounds, meshID, &material, isTransparent, shadowCaster, isOverlay);

	if (!mesh->Initialize(*out))
	{
		ErrMsg("Failed to bind mesh to entity '" + name + "'!");
		return false;
	}

	return true;
}
bool Scene::CreateBillboardMeshEntity(
	Entity **out, const std::string &name, const Material &material, 
	float rotation, float normalOffset, float size, bool keepUpright,
	bool isTransparent, bool castShadows, bool isOverlay)
{
	const BoundingOrientedBox bounds = { {0,0,0}, {0.1f, 0.1f, 0.1f}, {0,0,0,1} };

	if (!CreateEntity(out, name, bounds, false))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	BillboardMeshBehaviour *mesh = new BillboardMeshBehaviour(material, rotation, normalOffset, size, keepUpright, isTransparent, castShadows, isOverlay);

	if (!mesh->Initialize(*out))
	{
		ErrMsg("Failed to bind billboard mesh to entity '" + name + "'!");
		return false;
	}

	return true;
}

bool Scene::CreateCameraEntity(Entity **out, const std::string &name, float fov, float aspect, float nearZ, float farZ)
{
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .1f,.1f,.1f }, { 0,0,0,1 });

	if (!CreateEntity(out, name, bounds, false))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	CameraBehaviour *camera = new CameraBehaviour(ProjectionInfo(fov * DEG_TO_RAD, aspect, { nearZ, farZ }));

	if (!camera->Initialize(*out))
	{
		ErrMsg("Failed to bind camera to entity '" + name + "'!");
		return false;
	}
	return true;
}
bool Scene::CreateAnimationCamera()
{
	Entity *animationCameraEnt = nullptr;
	if (!CreateEntity(&animationCameraEnt, "AnimationCamera", { {}, {.1f,.1f,.1f}, {0,0,0,1} }, false))
	{
		ErrMsg("Failed to create animation camera!");
		return false;
	}
	animationCameraEnt->SetSerialization(false);

	ProjectionInfo camInfo = ProjectionInfo(80.0f * DEG_TO_RAD, 16.0f / 9.0f, { 0.05f, 75.0f });
	_animationCamera = new CameraBehaviour(camInfo);
	if (!_animationCamera->Initialize(animationCameraEnt))
	{
		ErrMsg("Failed to initialize animation camera behaviour!");
		return false;
	}

	RestrictedViewBehaviour *viewBehaviour = new RestrictedViewBehaviour();
	if (!viewBehaviour->Initialize(animationCameraEnt))
	{
		ErrMsg("Failed to initialize animation camera restricted view behaviour!");
		return false;
	}
	viewBehaviour->SetEnabled(false);

	SimplePointLightBehaviour *lightBehaviour = new SimplePointLightBehaviour({ 0.09f, 0.10f, 0.13f }, 0.115f);
	if (!lightBehaviour->Initialize(animationCameraEnt))
	{
		ErrMsg("Failed to initialize animation camera darkness light behaviour!");
		return false;
	}
	lightBehaviour->SetEnabled(false);

	SoundBehaviour *dragSound = new SoundBehaviour("DragBody", (DirectX::SOUND_EFFECT_INSTANCE_FLAGS)(0x3), false, 50.0f, 0.25f);
	if (!dragSound->Initialize(animationCameraEnt))
	{
		ErrMsg("Failed to initialize drag sound behaviour!");
		return false;
	}
	dragSound->SetVolume(1.2f);
	dragSound->SetEnabled(false);

	return true;
}

bool Scene::CreatePlayerEntity(Entity **out)
{
	float height = 1.85f;
	float width = 0.55f;
	float halfHeight = height * 0.5f;
	float halfWidth = width * 0.5f;
	float eyeHeight = height * 0.9f;

	Entity *playerHolderEntity = nullptr;
	if (!CreateEntity(&playerHolderEntity, "Player Holder", { {0,0,0}, {0.1f,0.1f,0.1f}, {0,0,0,1} }, false))
	{
		ErrMsg("Failed to initialize player holder!");
		return false;
	}

	Entity *playerEntity = nullptr;
	if (!CreateEntity(&playerEntity, "Player Entity", { {0, halfHeight, 0}, {halfWidth, halfHeight, halfWidth}, {0,0,0,1} }, false))
	{
		ErrMsg("Failed to initialize player!");
		return false;
	}

	playerEntity->SetParent(playerHolderEntity);
	playerEntity->GetTransform()->SetPosition({ -122.6f, 5.0f, -277.0f });

	PlayerMovementBehaviour *movementBehaviour = new PlayerMovementBehaviour();
	if (!movementBehaviour->Initialize(playerEntity))
	{
		ErrMsg("Failed to initialize movement example behaviour!");
		return false;
	}

	InteractorBehaviour *interactorBehaviour = new InteractorBehaviour();
	if (!interactorBehaviour->Initialize(playerEntity))
	{
		ErrMsg("Failed to initialize interactor behaviour!");
		return false;
	}

	InventoryBehaviour *inventoryBehaviour = new InventoryBehaviour();
	if (!inventoryBehaviour->Initialize(playerEntity))
	{
		ErrMsg("Failed to initialize player inventory behaviour!");
		return false;
	}

	MonsterHintBehaviour* monsterHintBehaviour = new MonsterHintBehaviour();
	if (!monsterHintBehaviour->Initialize(playerEntity))
	{
		ErrMsg("Failed to initialize monster hint behaviour!");
		return false;
	}

	ColliderBehaviour *collision = new ColliderBehaviour();
	if (!collision->Initialize(playerEntity))
	{
		ErrMsg("Failed to initialize collision behaviour!");
		return false;
	}

	collision->SetCollider(new Collisions::Capsule({ 0.0f, halfHeight, 0.0f }, { 0, 1, 0 }, width, height, Collisions::SKIP_TERRAIN_TAG));

	ExampleCollisionBehaviour *ecb = new ExampleCollisionBehaviour();
	if (!ecb->Initialize(playerEntity))
	{
		return false;
	}


	Entity *playerHeadTracker = nullptr;
	if (!CreateEntity(&playerHeadTracker, "Player Head Tracker", { {0,0,0},{.1f,.1f,.1f},{0,0,0,1} }, false))
	{
		ErrMsg("Failed to initialize player head tracker!");
		return false;
	}

	playerHeadTracker->SetParent(playerEntity);
	playerHeadTracker->GetTransform()->SetPosition({ 0.0f, eyeHeight, 0.0f });

	movementBehaviour->SetHeadTracker(playerHeadTracker);


	Entity *cam = nullptr;
	if (!CreateEntity(&cam, "playerCamera", { {0,0,0},{.1f,.1f,.1f},{0,0,0,1} }, false))
	{
		ErrMsg("Failed to create playerCamera entity!");
		return false;
	}

	cam->SetParent(playerHolderEntity);
	cam->GetTransform()->SetPosition(playerHeadTracker->GetTransform()->GetPosition(World), World);
	cam->GetTransform()->SetRotation(playerHeadTracker->GetTransform()->GetRotation(World), World);

	ProjectionInfo camInfo = ProjectionInfo(80.0f * DEG_TO_RAD, 16.0f / 9.0f, { 0.05f, 75.0f });
	_playerCamera = new CameraBehaviour(camInfo);
	if (!_playerCamera->Initialize(cam))
	{
		ErrMsg("Failed to initialize UI example behaviour!");
		return false;
	}

	movementBehaviour->SetPlayerCamera(_playerCamera);
	inventoryBehaviour->SetInventoryItemParents(cam);

	PlayerViewBehaviour *viewBehaviour = new PlayerViewBehaviour(movementBehaviour);
	if (!viewBehaviour->Initialize(cam))
	{
		ErrMsg("Failed to initialize player view behaviour!");
		return false;
	}
	

	SimplePointLightBehaviour *lightBehaviour = new SimplePointLightBehaviour({ 0.024f, 0.026f, 0.03f }, 0.25f);
	if (!lightBehaviour->Initialize(cam))
	{
		ErrMsg("Failed to initialize player darkness light behaviour!");
		return false;
	}


	Entity *flashlight = nullptr;
	if (!CreateEntity(&flashlight, "Flashlight", { {0,0,0},{0.1f,0.1f,0.1f},{0,0,0,1} }, false))
	{
		ErrMsg("Failed to initialize flashlight holder!");
		return false;
	}
	flashlight->SetParent(cam);
	flashlight->GetTransform()->SetPosition({ 0.4f, -0.4f, 0.6f });
	flashlight->GetTransform()->SetEuler({ 0.0f, 0.0f, XM_PI });

	FlashlightBehaviour *flashlightBehaviour = new FlashlightBehaviour();
	if (!flashlightBehaviour->Initialize(flashlight))
	{
		ErrMsg("Failed to initialize flashlight behaviour!");
		return false;
	}

	*out = playerEntity;
	_player = playerEntity;

#ifdef DEBUG_BUILD
	playerHolderEntity->SetSerialization(false);
	playerEntity->SetSerialization(false);
	playerHeadTracker->SetSerialization(false);
	cam->SetSerialization(false);
	flashlight->SetSerialization(false);
#endif
	return true;
}
bool Scene::CreateMonsterEntity(Entity **out)
{
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .1f,.1f,.1f }, { 0,0,0,1 });

	if (!CreateEntity(out, "Monster", bounds, true))
	{
		ErrMsg("Failed to create monster entity!");
		return false;
	}

	_monster = new MonsterBehaviour();
	if (!_monster->Initialize(*out))
	{
		ErrMsg("Failed to bind monster behaviour to entity!");
		return false;
	}

#ifdef DEBUG_BUILD
	(*out)->SetSerialization(false);
#endif
	return true;
}

bool Scene::CreateLanternEntity(Entity **out)
{
	// Create lantern body
	UINT meshID = _content->GetMeshID("Mesh_OldLamp_Metal");
	Material mat{};
	mat.textureID = _content->GetTextureID("Tex_OldLamp_Metal");
	mat.normalID = _content->GetTextureMapID("TexMap_OldLamp_Metal_Normal");
	mat.specularID = _content->GetTextureMapID("TexMap_OldLamp_Metal_Specular");
	mat.glossinessID = _content->GetTextureMapID("TexMap_OldLamp_Metal_Glossiness");
	mat.ambientID = _content->GetTextureID("Tex_AmbientBright");
	mat.occlusionID = _content->GetTextureMapID("TexMap_OldLamp_Metal_Occlusion");

	if (!CreateMeshEntity(out, "Lantern", meshID, mat))
	{
		ErrMsg("Failed to create lantern mesh entity!");
		return false;
	}
	(*out)->GetTransform()->SetScale({ 0.2f, 0.2f, 0.2f });

	// Create lantern glass
	meshID = _content->GetMeshID("Mesh_OldLamp_Glass");
	mat.textureID = _content->GetTextureID("Tex_OldLamp_Glass");
	mat.normalID = _content->GetTextureMapID("TexMap_OldLamp_Glass_Normal");
	mat.specularID = _content->GetTextureMapID("TexMap_OldLamp_Glass_Specular");
	mat.glossinessID = _content->GetTextureMapID("TexMap_OldLamp_Glass_Glossiness");
	mat.ambientID = _content->GetTextureID("Tex_AmbientBright");
	mat.occlusionID = _content->GetTextureMapID("TexMap_OldLamp_Glass_Occlusion");

	Entity *glass = nullptr;
	if (!CreateMeshEntity(&glass, "Lantern Glass", meshID, mat, true, false))
	{
		ErrMsg("Failed to create lantern mesh entity!");
		return false;
	}
	glass->SetParent(*out);

	// Create lantern light
	Entity *light = nullptr;
	if (!CreatePointLightEntity(&light, "Lantern Light", { 8.0f, 6.5f, 4.0f }, 1.65f, 0.1f, 8))
	{
		ErrMsg("Failed to create lantern light entity!");
		return false;
	}
	light->SetParent(*out);
	light->GetTransform()->Move({0.0f, 1.25f, 0.0f});

	return true;
}

bool Scene::CreateSpotLightEntity(Entity **out, const std::string &name, XMFLOAT3 color, float falloff, float angle, bool ortho, float nearZ, UINT updateFrequency)
{
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .2f,.2f,.2f }, {0,0,0,1});

	nearZ = nearZ < 0.1f ? 0.1f : nearZ;

	if (!CreateEntity(out, name, bounds, false))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	ProjectionInfo projInfo = ProjectionInfo(angle * DEG_TO_RAD, 1.0f, { nearZ, 1.0f });
	SpotLightBehaviour *light = new SpotLightBehaviour(projInfo, color, falloff, ortho, updateFrequency);

	if (!light->Initialize(*out))
	{
		ErrMsg("Failed to bind spotlight to entity '" + name + "'!");
		return false;
	}

	return true;
}
bool Scene::CreatePointLightEntity(Entity **out, const std::string &name, XMFLOAT3 color, float falloff, float nearZ, UINT updateFrequency)
{
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .2f,.2f,.2f }, {0,0,0,1});

	nearZ = nearZ < 0.1f ? 0.1f : nearZ;

	if (!CreateEntity(out, name, bounds, false))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	PointLightBehaviour *light = new PointLightBehaviour({ nearZ, 1.0f }, color, falloff, updateFrequency);

	if (!light->Initialize(*out))
	{
		ErrMsg("Failed to bind pointlight to entity '" + name + "'!");
		return false;
	}

	return true;
}
bool Scene::CreateSimpleSpotLightEntity(Entity **out, const std::string &name, XMFLOAT3 color, float falloff, float angle, bool ortho)
{
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .2f,.2f,.2f }, {0,0,0,1});

	if (!CreateEntity(out, name, bounds, false))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	SimpleSpotLightBehaviour *light = new SimpleSpotLightBehaviour(color, angle * DEG_TO_RAD, falloff, ortho);

	if (!light->Initialize(*out))
	{
		ErrMsg("Failed to bind simple spotlight to entity '" + name + "'!");
		return false;
	}

	return true;
}
bool Scene::CreateSimplePointLightEntity(Entity **out, const std::string &name, XMFLOAT3 color, float falloff)
{
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .2f,.2f,.2f }, {0,0,0,1});

	if (!CreateEntity(out, name, bounds, false))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	SimplePointLightBehaviour *light = new SimplePointLightBehaviour(color, falloff);

	if (!light->Initialize(*out))
	{
		ErrMsg("Failed to bind simple pointlight to entity '" + name + "'!");
		return false;
	}

	return true;
}

bool Scene::CreateGraphNodeEntity(Entity **out, GraphNodeBehaviour **node, XMFLOAT3 pos)
{
	if (!CreateEntity(out, "Node", {{0,0,0}, {1,1,1}, {0,0,0,1}}, true))
	{
		ErrMsg("Failed to create Node entity!");
		return false;
	}

	(*out)->GetTransform()->SetPosition(To3(pos), World);

	*node = new GraphNodeBehaviour();
	if (!(*node)->Initialize(*out))
	{
		ErrMsg("Failed to bind GraphNode to entity!");
		return false;
	}

	return true;
}
bool Scene::CreateSoundEmitterEntity(Entity **out, const std::string &name, const std::string &fileName, bool loop, float volume, float distanceScaler, float reverbScaler, float minimumDelay, float maximumDelay)
{
	const BoundingOrientedBox bounds = BoundingOrientedBox({}, { .1f,.1f,.1f }, { 0,0,0,1 });

	if (!CreateEntity(out, name, bounds, true))
	{
		ErrMsg("Failed to create entity '" + name + "'!");
		return false;
	}

	AmbientSoundBehaviour* emitter = new AmbientSoundBehaviour(fileName, SoundEffectInstance_Use3D | SoundEffectInstance_ReverbUseFilters, loop, volume, distanceScaler, reverbScaler, minimumDelay, maximumDelay);

	if (!emitter->Initialize(*out))
	{
		ErrMsg("Failed to bind sound behaviour to entity '" + name + "'!");
		return false;
	}
	return true;
}
#pragma endregion

#pragma region Misc
void Scene::SuspendSceneSound()
{
	_soundEngine.Suspend();
}
void Scene::ResumeSceneSound()
{
	_soundEngine.Resume();
}

void Scene::ResetScene()
{
	_globalEntities.clear();

	_initialized = false;
	_game = nullptr;
	_device = nullptr;
	_context = nullptr;
	_content = nullptr;
	_graphics = nullptr;
	_graphManager = {};
	_sceneHolder.ResetSceneHolder();
#ifdef DEBUG_BUILD
	_debugPlayer = nullptr;
	_transformGizmo = nullptr;
#endif
	_input = nullptr;

	_viewCamera = nullptr;

	_player = nullptr;
	_monster = nullptr;
	_terrainBehaviour = nullptr;
	_terrain = nullptr;

	_collisionHandler = {};
	_soundEngine.ResetSoundEngine();

	_timelineManager = {};

#ifdef DEBUG_BUILD
	_isGeneratingEntityBounds = false;
	_isGeneratingVolumeTree = false;
	_isGeneratingCameraCulling = false;
	_rayCastFromMouse = false;
	_cameraCubeSide = 0;
#endif

	_saveFile = "";

	_spotlights = std::make_unique<SpotLightCollection>();
	_pointlights = std::make_unique<PointLightCollection>();
}

void Scene::👢()
{
}
#pragma endregion
