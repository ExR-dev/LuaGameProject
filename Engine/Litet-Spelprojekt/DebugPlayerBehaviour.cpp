#include "stdafx.h"
#include "DebugPlayerBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "UIButtonBehaviour.h"
#include "FlashlightBehaviour.h"
#include "GraphNodeBehaviour.h"
#include "BillboardMeshBehaviour.h"
#include "PlayerMovementBehaviour.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

#ifdef DEBUG_BUILD
// Start runs once when the behaviour is created.
bool DebugPlayerBehaviour::Start()
{
	if (_name == "")
		_name = "DebugPlayerBehaviour"; // For categorization in ImGui.

	Scene *scene = GetScene();
	SceneHolder *sceneHolder = scene->GetSceneHolder();
	Content *content = scene->GetContent();

	// Create main camera
	{
		Entity *ent = nullptr;
		if (!scene->CreateEntity(&ent, "MainCamera", {{0,0,0},{.1f,.1f,.1f},{0,0,0,1}}, false))
		{
			ErrMsg("Failed to create MainCamera entity!");
			return false;
		}
		ent->GetTransform()->SetPosition({ 0.0f, 2.0f, -3.0f });

		ProjectionInfo projInfo = ProjectionInfo(65.0f * (XM_PI / 180.0f), 16.0f / 9.0f, { 0.2f, 1000.0f });
		CameraBehaviour *camera = new CameraBehaviour(projInfo);

		if (!camera->Initialize(ent))
		{
			ErrMsg("Failed to bind MainCamera behaviour!");
			return false;
		}

		ent->SetSerialization(false);
		camera->SetSerialization(false);

		_mainCamera = camera;
	}
	
	// Create secondary camera
	{
		Entity *ent = nullptr;
		if (!scene->CreateEntity(&ent, "SecondaryCamera", {{0,0,0},{.1f,.1f,.1f},{0,0,0,1}}, false))
		{
			ErrMsg("Failed to create SecondaryCamera entity!");
			return false;
		}

		ent->GetTransform()->SetPosition({ 0.0f, 0.0f, 0.0f });

		ProjectionInfo projInfo = ProjectionInfo(90.0f * (XM_PI / 180.0f), 16.0f / 9.0f, { 0.01f, 100.0f });
		CameraBehaviour *camera = new CameraBehaviour(projInfo);

		if (!camera->Initialize(ent))
		{
			ErrMsg("Failed to bind SecondaryCamera behaviour!");
			return false;
		}

		ent->SetSerialization(false);
		camera->SetSerialization(false);

		_secondaryCamera = camera;
	}

	_currCameraPtr = _mainCamera;
	GetScene()->SetViewCamera(_currCameraPtr);
	scene->SetDebugPlayer(this);

	return true;
}

// Update runs every frame.
bool DebugPlayerBehaviour::Update(Time &time, const Input &input)
{
	Scene *scene = GetScene();
	SceneHolder *sceneHolder = scene->GetSceneHolder();
	std::vector<std::unique_ptr<Entity>> *globalEntities = scene->GetGlobalEntities();
	SpotLightCollection *spotlights = scene->GetSpotlights();
	PointLightCollection *pointlights = scene->GetPointlights();
	ID3D11Device *device = scene->GetDevice();
	Content *content = scene->GetContent();
	Graphics *graphics = scene->GetGraphics();
	DebugDrawer *debugDraw = scene->GetDebugDrawer();

	Entity *secondaryCam = sceneHolder->GetEntityByName("playerCamera"); 
	if (secondaryCam)
	{
		secondaryCam->GetBehaviourByType<CameraBehaviour>(_secondaryCamera);
	}
	else
	{
		secondaryCam = sceneHolder->GetEntityByName("SecondaryCamera"); 
		if (secondaryCam)
		{
			secondaryCam->GetBehaviourByType<CameraBehaviour>(_secondaryCamera);
		}
		else
		{
			_secondaryCamera = _mainCamera;
		}
	}

	if (_rotateLights)
	{
		Entity *lightEnt = sceneHolder->GetEntityByName("Spotlight Red");
		if (lightEnt)
		{
			SpotLightBehaviour *lightBeh = nullptr;
			if (lightEnt->GetBehaviourByType<SpotLightBehaviour>(lightBeh))
			{
				Transform *lightTransform = lightEnt->GetTransform();
				lightTransform->Rotate({ time.deltaTime * -0.5f, 0, 0 }, World);
				lightTransform->MoveRelative({ 0, time.deltaTime * 2.0f, 0 }, World);
			}
		}

		lightEnt = sceneHolder->GetEntityByName("Spotlight Green");
		if (lightEnt)
		{
			SpotLightBehaviour *lightBeh = nullptr;
			if (lightEnt->GetBehaviourByType<SpotLightBehaviour>(lightBeh))
			{
				Transform *lightTransform = lightEnt->GetTransform();
				lightTransform->Rotate({ 0, time.deltaTime * 0.5f, 0 }, World);
				lightTransform->MoveRelative({ time.deltaTime * -2.0f, 0, 0 }, World);
			}
		}

		lightEnt = sceneHolder->GetEntityByName("Spotlight Blue");
		if (lightEnt)
		{
			SpotLightBehaviour *lightBeh = nullptr;
			if (lightEnt->GetBehaviourByType<SpotLightBehaviour>(lightBeh))
			{
				Transform *lightTransform = lightEnt->GetTransform();
				lightTransform->Rotate({ 0, 0, time.deltaTime * 0.5f }, World);
				lightTransform->MoveRelative({ 0, time.deltaTime * 2.0f, 0 }, World);
			}
		}
	}

	if (input.IsInFocus()) // Handle user input while window is in focus
	{
		if (input.GetKey(KeyCode::B) == KeyState::Pressed)	// Toggle ray cast method (from cam/mouse)
			_rayCastFromMouse = !_rayCastFromMouse;

		if (input.GetKey(KeyCode::G) == KeyState::Pressed)
			_rotateLights = !_rotateLights;

		if (input.GetKey(KeyCode::Z) == KeyState::Pressed)
			_useMainCamera = !_useMainCamera;
		else if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			_useMainCamera = true;

		if (_cursorPositioningTarget)
		{
			if (input.GetKey(KeyCode::LeftAlt) == KeyState::Held)
			{
				// Rotate cursor positioning target with mouse scroll
				float scroll = input.GetMouse().scrollY;

				if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
					scroll *= 45.0f;
				else if (input.GetKey(KeyCode::LeftControl) == KeyState::Held)
					scroll *= 5.0f;
				else
					scroll *= 15.0f;

				if (scroll != 0.0f)
					_cursorPositioningTarget->GetTransform()->Rotate({0, scroll * DEG_TO_RAD, 0}, _editSpace);
			}

			static bool skipRayCast = true;
			if (input.GetKey(KeyCode::M4) == KeyState::Pressed)
				skipRayCast = !skipRayCast;

			XMFLOAT3A cursorScenePos;
			XMFLOAT3A castDir;
			RaycastOut out;

			bool rayHit = false;
			if (!_rayCastFromMouse && input.IsCursorLocked())
				rayHit = RayCastFromCamera(out, cursorScenePos, castDir);
			else
				rayHit = RayCastFromMouse(out, cursorScenePos, castDir, input);

			if (rayHit && !skipRayCast)
			{
				_cursorPositioningTarget->GetTransform()->SetPosition(cursorScenePos, World);
			}
			else
			{
				static float dist = 10.0f;

				float scroll = input.GetMouse().scrollY;
				if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
					scroll *= 10.0f;
				if (input.GetKey(KeyCode::LeftControl) == KeyState::Held)
					scroll *= 0.2f;
				if (input.GetKey(KeyCode::LeftAlt) == KeyState::Held)
					scroll *= 0.0f;

				if (scroll != 0.0f)
				{
					dist += scroll * 0.25f;
					if (dist < 0.1f)
						dist = 0.1f;
				}

				const XMFLOAT3A camPos = _currCameraPtr->GetTransform()->GetPosition(World);
				const XMFLOAT3A placementPos = {
					camPos.x + castDir.x * dist,
					camPos.y + castDir.y * dist,
					camPos.z + castDir.z * dist
				};

				_cursorPositioningTarget->GetTransform()->SetPosition(placementPos, World);
			}

			if (input.GetKey(KeyCode::Enter) == KeyState::Pressed || 
				input.GetKey(KeyCode::M2) == KeyState::Pressed || input.GetKey(KeyCode::M3) == KeyState::Pressed)
			{
				if (_includePositioningTargetInTree)
				{
					if (!sceneHolder->IncludeEntityInTree(_cursorPositioningTarget))
					{
						ErrMsg("Failed to include positioned entity in tree!");
						return false;
					}
				}

				SetSelection(sceneHolder->GetEntityIndex(_cursorPositioningTarget));
				_cursorPositioningTarget = nullptr;
			}
		}
		else if (_currSelection >= 0)
		{
			if (input.GetKey(KeyCode::M5) == KeyState::Pressed)
			{
				PositionWithCursor(sceneHolder->GetEntity(_currSelection));
			}
		}

		if (input.GetKey(KeyCode::M2) == KeyState::Pressed)
		{
			RaycastOut out;
			if (_rayCastFromMouse || !input.IsCursorLocked())
			{
				if (RayCastFromMouse(out, input))
				{
					const UINT entIndex = sceneHolder->GetEntityIndex(out.entity);

					if (_currSelection == static_cast<int>(entIndex))
						_currSelection = -1;
					else
						_currSelection = (entIndex == 0xffffffff) ? -1 : static_cast<int>(entIndex);
				}
				else
					_currSelection = -1;
			}
			else
			{
				if (RayCastFromCamera(out))
				{
					const UINT entIndex = sceneHolder->GetEntityIndex(out.entity);

					if (_currSelection == static_cast<int>(entIndex))
						_currSelection = -1;
					else
						_currSelection = (entIndex == 0xffffffff) ? -1 : static_cast<int>(entIndex);
				}
				else
					_currSelection = -1;
			}

			Entity* selected = sceneHolder->GetEntity(_currSelection);
			BillboardMeshBehaviour* billboard = nullptr;
			if (selected)
			{
				if (selected->GetParent())
				{
					selected = selected->GetParent();
					if (selected->GetBehaviourByType<BillboardMeshBehaviour>(billboard))
					{
						_currSelection = sceneHolder->GetEntityIndex(selected);
					}
				}
				
			}
		}

		// Select entity, if it has a UIButtonBehaviour, run it
		if (input.GetKey(KeyCode::M3) == KeyState::Pressed)
		{
			RaycastOut out;
			bool hasHit = false;

			if (_rayCastFromMouse)
			{
				hasHit = RayCastFromMouse(out, input);
			}
			else
			{
				hasHit = RayCastFromCamera(out);
			}

			if (hasHit)
			{
				if (!out.entity->InitialOnSelect())
				{
					ErrMsg("OnSelect Failed!");
					return false;
				}
			}
		}

		if (input.GetKey(KeyCode::E) == KeyState::Pressed)
			_editType = Translate;
		else if (input.GetKey(KeyCode::R) == KeyState::Pressed)
			_editType = Rotate;
		else if (input.GetKey(KeyCode::T) == KeyState::Pressed)
			_editType = Scale;

		if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
			_currSelection = -1;
		else if (input.GetKey(KeyCode::Add) == KeyState::Pressed)
			_currSelection++;
		else if (input.GetKey(KeyCode::Subtract) == KeyState::Pressed)
			_currSelection--;

		if (input.GetKey(KeyCode::Q) == KeyState::Pressed)
		{
			_editSpace = Local;
			_moveRelative = false; 
			_editType = Translate;
			_currCamera = -3;
		}

		if (_currSelection < -1)
			_currSelection = -1;
		else if (_currSelection >= static_cast<int>(sceneHolder->GetEntityCount()))
			_currSelection = static_cast<int>(sceneHolder->GetEntityCount()) - 1;

		float currSpeed = 5.0f;
		if (input.GetKey(KeyCode::LeftShift) == KeyState::Held)
			currSpeed *= 10.0f;
		if (input.GetKey(KeyCode::LeftControl) == KeyState::Held)
			currSpeed *= 0.2f;

		static bool freezeCamera = false;
		if (input.GetKey(KeyCode::K) == KeyState::Pressed)
			freezeCamera = !freezeCamera;

		// Move camera
		if (input.IsCursorLocked() && !freezeCamera)
		{
			Transform *camTransform = _currCameraPtr->GetTransform();

			if (BindingCollection::IsTriggered(InputBindings::InputAction::StrafeRight))
				camTransform->MoveRelative({ time.deltaTime * currSpeed, 0.0f, 0.0f }, World);
			else if (BindingCollection::IsTriggered(InputBindings::InputAction::StrafeLeft))
				camTransform->MoveRelative({ -time.deltaTime * currSpeed, 0.0f, 0.0f }, World);

			if (input.IsPressedOrHeld(KeyCode::Space))
				camTransform->MoveRelative({ 0.0f, time.deltaTime * currSpeed, 0.0f }, World);
			else if (input.IsPressedOrHeld(KeyCode::X))
				camTransform->MoveRelative({ 0.0f, -time.deltaTime * currSpeed, 0.0f }, World);

			if (BindingCollection::IsTriggered(InputBindings::InputAction::WalkForward))
				camTransform->MoveRelative({ 0.0f, 0.0f, time.deltaTime * currSpeed }, World);
			else if (BindingCollection::IsTriggered(InputBindings::InputAction::WalkBackward))
				camTransform->MoveRelative({ 0.0f, 0.0f, -time.deltaTime * currSpeed }, World);

			const MouseState mState = input.GetMouse();
			if (mState.dx != 0.0f) 
			{
				XMFLOAT3A u = camTransform->GetUp();
				float invert = (XMVectorGetX(XMVector3Dot(XMLoadFloat3A(&u), { 0, 1, 0, 0 })) > 0 ? 1.0f : -1.0f);
				camTransform->RotateAxis({0, 1, 0}, (static_cast<float>(mState.dx) / 360.0f) *invert, World);
			}
			if (mState.dy != 0) 
			{
				camTransform->RotatePitch(static_cast<float>(mState.dy) / 360.0f, Local);
			}
		}

		bool doCopy = (BindingCollection::IsTriggered(InputBindings::InputAction::CopySelected) || input.GetKey(KeyCode::NumPad3) == KeyState::Pressed);
		if (doCopy && !input.IsCursorLocked())
		{
			bool doCreate = false;
			bool isNode = false;
			UINT meshIndex = 0;
			Material meshMaterial;
			Transform *copyTransform = nullptr;

			int selected = GetSelection();

			Entity *selectedEnt = sceneHolder->GetEntity(selected);
			GraphNodeBehaviour *selectedNode = nullptr;
			MeshBehaviour *selectedMesh = nullptr;

			if (selectedEnt)
			{
				if (selectedEnt->GetBehaviourByType<GraphNodeBehaviour>(selectedNode) ||
					selectedEnt->GetBehaviourByType<MeshBehaviour>(selectedMesh))
				{
					doCreate = true;
				}
				else
				{
					ErrMsg("Selected entity is not a valid entity type!");
				}
			}
			else
			{
				ErrMsg("No selection found!");
			}

			if (doCreate)
			{
				if (selectedNode)
				{
					Entity *ent = nullptr;
					GraphNodeBehaviour *copyNode = nullptr;

					if (!scene->CreateGraphNodeEntity(&ent, &copyNode, selectedEnt->GetTransform()->GetPosition()))
					{
						ErrMsg("Failed to copy node entity!");
						return false;
					}

					copyNode->AddConnection(selectedNode);
					if (input.GetKey(KeyCode::NumPad3) == KeyState::Pressed)
						PositionWithCursor(ent);
					else
						SetSelectionID(ent->GetID());
				}
				else if (selectedMesh)
				{
					meshMaterial = *selectedMesh->GetMaterial();

					bool isTransparent = meshMaterial.textureID >= content->GetTextureID("Tex_Transparent");

					Entity *ent = nullptr;
					if (!scene->CreateMeshEntity(&ent, selectedEnt->GetName(), selectedMesh->GetMesh(), meshMaterial))
					{
						ErrMsg("Failed to copy mesh entity!");
						return false;
					}

					ent->GetTransform()->SetMatrix(selectedEnt->GetTransform()->GetMatrix(World), World);
					if (input.GetKey(KeyCode::NumPad3) == KeyState::Pressed)
						PositionWithCursor(ent);
					else
						SetSelectionID(ent->GetID());
				}
			}
		}
		else if (BindingCollection::IsTriggered(InputBindings::InputAction::SuperCopySelected))
		{
			Entity *selectedEnt = sceneHolder->GetEntity(GetSelection());

			if (selectedEnt)
			{
				// Copy by serializing and deserializing the entity

				std::string code = "";
				if (!scene->SerializeEntity(&code, selectedEnt, true))
				{
					ErrMsg("Failed to serialize entity!");
					return false;
				}

				Entity *ent = nullptr;
				if (!scene->DeserializeEntity(code, &ent))
				{
					ErrMsg("Failed to deserialize entity!");
					return false;
				}

				if (ent)
					SetSelectionID(ent->GetID());
			}
		}

		for (int i = 0; i < _duplicateBinds.size(); i++)
		{
			Entity *ent = sceneHolder->GetEntityByID(_duplicateBinds[i].second);

			if (ent)
				continue;

			RemoveDuplicateBind(_duplicateBinds[i].second);
			i--;
		}

		for (auto &duplicateBind : _duplicateBinds)
		{
			if (input.GetKey(duplicateBind.first) == KeyState::Pressed)
			{
				// Copy by serializing and deserializing the entity

				std::string code = "";
				Entity *dupeEnt = sceneHolder->GetEntityByID(duplicateBind.second);

				if (dupeEnt)
				{
					if (!scene->SerializeEntity(&code, dupeEnt, true))
					{
						ErrMsg("Failed to serialize entity!");
						return false;
					}

					Entity *ent = nullptr;
					if (!scene->DeserializeEntity(code, &ent))
					{
						ErrMsg("Failed to deserialize entity!");
						return false;
					}

					if (ent)
						PositionWithCursor(ent);
				}
				else
				{
					RemoveDuplicateBind(duplicateBind.second);
				}
			}
		}

		// Move selected entity
		if (_currSelection >= 0) 
		{
			if (input.GetKey(KeyCode::N) == KeyState::Pressed)
				_moveRelative = !_moveRelative;

			if (input.GetKey(KeyCode::M) == KeyState::Pressed)
				_editSpace = (_editSpace == World) ? Local : World;

			XMVECTOR
				right = XMVectorSet(1, 0, 0, 0),
				up = XMVectorSet(0, 1, 0, 0),
				forward = XMVectorSet(0, 0, 1, 0);

			XMVECTOR transformationVector = XMVectorZero();
			bool doMove = false;

			if (input.IsPressedOrHeld(KeyCode::Right))
			{
				transformationVector += right * time.deltaTime * currSpeed;
				doMove = true;
			}
			else if (input.IsPressedOrHeld(KeyCode::Left))
			{
				transformationVector -= right * time.deltaTime * currSpeed;
				doMove = true;
			}

			if (input.IsPressedOrHeld(KeyCode::RightShift))
			{
				transformationVector += up * time.deltaTime * currSpeed;
				doMove = true;
			}
			else if (input.IsPressedOrHeld(KeyCode::RightControl))
			{
				transformationVector -= up * time.deltaTime * currSpeed;
				doMove = true;
			}

			if (input.IsPressedOrHeld(KeyCode::Up))
			{
				transformationVector += forward * time.deltaTime * currSpeed;
				doMove = true;
			}
			else if (input.IsPressedOrHeld(KeyCode::Down))
			{
				transformationVector -= forward * time.deltaTime * currSpeed;
				doMove = true;
			}

			if (doMove)
			{
				Entity *ent = sceneHolder->GetEntity(_currSelection);
				Transform *entityTransform = ent->GetTransform();

				switch (_editType)
				{
				case Translate:
					if (_moveRelative)
						entityTransform->MoveRelative(*reinterpret_cast<XMFLOAT3A *>(&transformationVector), _editSpace);
					else
						entityTransform->Move(*reinterpret_cast<XMFLOAT3A *>(&transformationVector), _editSpace);
					break;

				case Rotate:
					if (_moveRelative)
					{
						XMFLOAT3A right, up, forward;
						entityTransform->GetAxes(&right, &up, &forward, _editSpace);

						XMFLOAT3A dir = *reinterpret_cast<XMFLOAT3A *>(&transformationVector);
						XMFLOAT3A relativeDirection = {
							(right.x * dir.x + up.x * dir.y + forward.x * dir.z),
							(right.y * dir.x + up.y * dir.y + forward.y * dir.z),
							(right.z * dir.x + up.z * dir.y + forward.z * dir.z)
						};

						entityTransform->Rotate(relativeDirection, _editSpace);
					}
					else
					{
						entityTransform->Rotate(*reinterpret_cast<XMFLOAT3A *>(&transformationVector), _editSpace);
					}
					break;

				case Scale:
					if (_moveRelative)
					{
						XMFLOAT3A right, up, forward;
						entityTransform->GetAxes(&right, &up, &forward, _editSpace);

						XMFLOAT3A dir = *reinterpret_cast<XMFLOAT3A *>(&transformationVector);
						XMFLOAT3A relativeDirection = {
							(right.x * dir.x + up.x * dir.y + forward.x * dir.z),
							(right.y * dir.x + up.y * dir.y + forward.y * dir.z),
							(right.z * dir.x + up.z * dir.y + forward.z * dir.z)
						};

						entityTransform->Scale(*reinterpret_cast<XMFLOAT3A *>(&relativeDirection), _editSpace);
					}
					else
					{
						entityTransform->Scale(*reinterpret_cast<XMFLOAT3A *>(&transformationVector), _editSpace);
					}
					break;
				}

				if (!sceneHolder->UpdateEntityPosition(ent))
				{
					ErrMsg("Failed to update entity position!");
					return false;
				}
			}
		}

		if (input.GetKey(KeyCode::M1) == KeyState::Held && (input.IsCursorLocked() || _rayCastFromMouse))
		{
			_drawPointer = true;
		}

		if (input.GetKey(KeyCode::C) == KeyState::Pressed || _currCamera == -3)
		{ // Change camera
			_currCamera++;

			const int spotlightCount = static_cast<int>(spotlights->GetNrOfLights());

			if (_currCamera - 2 - spotlightCount >= 0)
				_currCamera = -2;

			if (_currCamera < 0)
			{
				if (_currCamera == -1)
				{
					scene->SetViewCamera(_secondaryCamera);
				}
				else
				{
					_currCamera = -2;
					scene->SetViewCamera(_mainCamera);
				}
			}
			else if (_currCamera < spotlightCount)
			{
				scene->SetViewCamera(spotlights->GetLightBehaviour(_currCamera)->GetShadowCamera());
			}
		}
	}

	if (_currSelection >= 0 && input.GetKey(KeyCode::Delete) == KeyState::Pressed)
	{
		const Entity *ent = sceneHolder->GetEntity(_currSelection);
		if (!sceneHolder->RemoveEntity(_currSelection))
		{
			ErrMsg("Failed to remove entity!");
			return false;
		}
		_currSelection = -1;
	}

	if (_addDuplicateBindForEntity >= 0)
	{
		Entity *ent = sceneHolder->GetEntityByID(_addDuplicateBindForEntity);

		if (ent)
		{
			KeyCode key = input.GetPressedKey();
			if (key != KeyCode::None)
			{
				if (IsValidDuplicateBind(key))
				{
					AddDuplicateBind(key, (UINT)_addDuplicateBindForEntity);
					_addDuplicateBindForEntity = -1;
				}
			}
		}
		else
		{
			_addDuplicateBindForEntity = -1;
		}
	}

	if (!UpdateGlobalEntities(time, input))
	{
		ErrMsg("Failed to update global entities!");
		return false;
	}

	if (input.GetKey(KeyCode::Z) == KeyState::Pressed)
	{
		PlayerMovementBehaviour *pmb;
		if (scene->GetPlayer())
			if (scene->GetPlayer()->GetBehaviourByType<PlayerMovementBehaviour>(pmb))
				pmb->Catch();
	}

	return true;
}

// Render runs every frame when objects are being queued for rendering.
bool DebugPlayerBehaviour::Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo)
{
	Scene *scene = GetScene();
	std::vector<std::unique_ptr<Entity>> *globalEntities = scene->GetGlobalEntities();

	if (_cursorPositioningTarget)
	{
		if (!_cursorPositioningTarget->InitialRender(queuer, rendererInfo))
		{
			ErrMsg("Failed to render cursor positioning target!");
			return false;
		}
	}

	Entity *playerEnt = scene->GetSceneHolder()->GetEntityByName("Player Entity");
	if (playerEnt)
	{
		if (!playerEnt->InitialRender(queuer, rendererInfo))
		{
			ErrMsg("Failed to render player entity!");
			return false;
		}
	}

	for (auto &entPtr : *globalEntities)
	{
		Entity *ent = entPtr.get();

		if (_currSelection >= 0) // Render selection marker and transform gizmo
		{
			if (ent->GetName() == "Selection Marker")
			{
				if (!ent->InitialRender(queuer, rendererInfo))
				{
					ErrMsg("Failed to render entity!");
					return false;
				}
			}
		}

		if (_drawPointer) // Render pointer
		{
			if (ent->GetName() == "Pointer Gizmo")
			{
				if (!ent->InitialRender(queuer, rendererInfo))
				{
					ErrMsg("Failed to render entity!");
					return false;
				}

				_drawPointer = false;
			}
		}

		if (ent->GetName() == "Culling Tree Wireframe" ||
			ent->GetName() == "Entity Bounds Wireframe" ||
			ent->GetName() == "Camera Culling Wireframe")
		{
			if (!ent->InitialRender(queuer, rendererInfo))
			{
				ErrMsg("Failed to render entity!");
				return false;
			}
		}
	}
	return true;
}

bool DebugPlayerBehaviour::UpdateGlobalEntities(Time &time, const Input &input)
{
	Scene *scene = GetScene();
	SceneHolder *sceneHolder = scene->GetSceneHolder();
	std::vector<std::unique_ptr<Entity>> *globalEntities = scene->GetGlobalEntities();
	SpotLightCollection *spotlights = scene->GetSpotlights();
	PointLightCollection *pointlights = scene->GetPointlights();
	ID3D11Device *device = scene->GetDevice();
	Content *content = scene->GetContent();
	Graphics *graphics = scene->GetGraphics();

	Entity
		*selection = (_currSelection < 0) ? nullptr : sceneHolder->GetEntity(_currSelection),
		*marker = nullptr,
		*gizmo = nullptr,
		*pointer = nullptr;

	for (auto &entPtr : *globalEntities)
	{
		if (entPtr->GetName() == "Selection Marker")
		{
			marker = entPtr.get();
		}
		else if (entPtr->GetName() == "Transform Gizmo")
		{
			gizmo = entPtr.get();
		}
		else if (entPtr->GetName() == "Pointer Gizmo")
		{
			pointer = entPtr.get();
		}
	}

	if (selection)
	{
		BoundingOrientedBox box = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0, 1} };
		selection->StoreEntityBounds(box);

		const XMFLOAT4A rot = {
			box.Orientation.x,
			box.Orientation.y,
			box.Orientation.z,
			box.Orientation.w
		};

		if (marker)
		{
			const XMFLOAT4A
				center = { box.Center.x, box.Center.y, box.Center.z, 0 },
				extents = { box.Extents.x, box.Extents.y, box.Extents.z, 0 };

			marker->GetTransform()->SetPosition(center, World);
			marker->GetTransform()->SetRotation(rot, World);
			marker->GetTransform()->SetScale(extents, World);

			if (!marker->InitialUpdate(time, input))
			{
				ErrMsg("Failed to update marker gizmo!");
				return false;
			}
		}

		if (gizmo)
		{
			gizmo->GetTransform()->SetPosition(selection->GetTransform()->GetPosition(World), World);		

			XMFLOAT4A newRot = { 0, 0, 0, 1 };

			if (_moveRelative)
			{
				newRot = selection->GetTransform()->GetRotation(World);
			}
			else
			{
				Transform *parent = selection->GetTransform()->GetParent();

				if (_editSpace == Local && parent != nullptr)
					newRot = parent->GetRotation(World);
			}

			gizmo->GetTransform()->SetRotation(newRot, World);

			if (!gizmo->InitialUpdate(time, input))
			{
				ErrMsg("Failed to update transform gizmo!");
				return false;
			}
		}
	}

	if (_drawPointer)
	{
		if (pointer)
		{
			RaycastOut out;
			XMFLOAT3A hitPos = { };
			XMFLOAT3A castDir = { };

			if (_rayCastFromMouse)
			{
				if (RayCastFromMouse(out, hitPos, castDir, input))
					pointer->GetTransform()->SetPosition(hitPos, World);
				else
					_drawPointer = false;
			}
			else
			{
				if (RayCastFromCamera(out, hitPos, castDir))
					pointer->GetTransform()->SetPosition(hitPos, World);
				else
					_drawPointer = false;
			}

			if (_drawPointer)
			{
				if (!pointer->InitialUpdate(time, input))
				{
					ErrMsg("Failed to update gizmo gizmo!");
					return false;
				}
			}
		}
		else
		{
			_drawPointer = false;
		}
	}

	return true;
}

bool DebugPlayerBehaviour::Serialize(std::string *code) const
{
	*code += _name + "( )";
	return true;
}
bool DebugPlayerBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize

	return true;
}

void DebugPlayerBehaviour::SetCamera(CameraBehaviour *cam)
{
	_currCameraPtr = cam;
}

void DebugPlayerBehaviour::SetSelection(int index)
{
	_currSelection = index;
}
void DebugPlayerBehaviour::SetSelectionID(int id)
{
	_currSelection = GetScene()->GetSceneHolder()->GetEntityIndex(id);
}
int DebugPlayerBehaviour::GetSelection() const
{
	return _currSelection;
}
void DebugPlayerBehaviour::SetEditSpace(ReferenceSpace space)
{
	_editSpace = space;
}
ReferenceSpace DebugPlayerBehaviour::GetEditSpace() const
{
	return _editSpace;
}
void DebugPlayerBehaviour::SetEditType(TransformationType type)
{
	_editType = type;
}
TransformationType DebugPlayerBehaviour::GetEditType() const
{
	return _editType;
}

void DebugPlayerBehaviour::AssignDuplicateToKey(UINT id)
{
	_addDuplicateBindForEntity = id;
}
bool DebugPlayerBehaviour::IsAssigningDuplicateToKey(UINT id) const
{
	return id == _addDuplicateBindForEntity;
}
void DebugPlayerBehaviour::AddDuplicateBind(KeyCode key, UINT id)
{
	if (id == CONTENT_NULL)
		return;

	_duplicateBinds.push_back(std::pair<KeyCode, UINT>(key, id));
}
void DebugPlayerBehaviour::RemoveDuplicateBind(UINT id)
{
	if (id == CONTENT_NULL)
		return;

	for (auto it = _duplicateBinds.begin(); it != _duplicateBinds.end();)
	{
		if (it->second == id)
			it = _duplicateBinds.erase(it);
		else
			++it;
	}
}
bool DebugPlayerBehaviour::HasDuplicateBind(UINT id) const
{
	if (id == CONTENT_NULL)
		return false;

	for (auto it = _duplicateBinds.begin(); it != _duplicateBinds.end();)
	{
		if (it->second == id)
			return true;
		else
			++it;
	}

	return false;
}
KeyCode DebugPlayerBehaviour::GetDuplicateBind(UINT id)
{
	if (id == CONTENT_NULL)
		return KeyCode::None;

	for (auto it = _duplicateBinds.begin(); it != _duplicateBinds.end();)
	{
		if (it->second == id)
			return it->first;
		else
			++it;
	}

	return KeyCode::None;
}
void DebugPlayerBehaviour::ClearDuplicateBinds()
{
	_duplicateBinds.clear();
}
bool DebugPlayerBehaviour::IsValidDuplicateBind(KeyCode key) const
{
	return (key != KeyCode::W) 
		&& (key != KeyCode::A) 
		&& (key != KeyCode::S) 
		&& (key != KeyCode::D)
		&& (key != KeyCode::X)
		&& (key != KeyCode::Space)
		&& (key != KeyCode::E)
		&& (key != KeyCode::R)
		&& (key != KeyCode::T)
		&& (key != KeyCode::M1)
		&& (key != KeyCode::M2)
		&& (key != KeyCode::Enter)
		&& (key != KeyCode::Delete)
		&& (key != KeyCode::Tab)
		&& (key != KeyCode::LeftControl)
		&& (key != KeyCode::LeftShift)
		&& (key != KeyCode::LeftAlt)
		&& (key != KeyCode::Q)
		&& (key != KeyCode::C)
		&& (key != KeyCode::F5);
}

void DebugPlayerBehaviour::PositionWithCursor(Entity *ent)
{
	SceneHolder *sceneHolder = GetScene()->GetSceneHolder();

	if (_cursorPositioningTarget)
	{
		if (_includePositioningTargetInTree)
		{
			if (!sceneHolder->IncludeEntityInTree(_cursorPositioningTarget))
			{
				ErrMsg("Failed to include previous positioned entity in tree!");
				return;
			}
		}

		_cursorPositioningTarget = nullptr;
	}

	if (!ent)
		return;

	_cursorPositioningTarget = ent;
	_includePositioningTargetInTree = sceneHolder->IsEntityIncludedInTree(ent);

	if (_includePositioningTargetInTree)
	{
		if (!sceneHolder->ExcludeEntityFromTree(_cursorPositioningTarget))
		{
			ErrMsg("Failed to exclude positioning entity in tree!");
			return;
		}
	}
}

bool DebugPlayerBehaviour::RayCastFromCamera(RaycastOut &out)
{
	const XMFLOAT3A
		camPos = _currCameraPtr->GetTransform()->GetPosition(World),
		camDir = _currCameraPtr->GetTransform()->GetForward(World);

	return GetScene()->GetSceneHolder()->RaycastScene(
		{ camPos.x, camPos.y, camPos.z },
		{ camDir.x, camDir.y, camDir.z },
		out);
}
bool DebugPlayerBehaviour::RayCastFromCamera(RaycastOut &out, XMFLOAT3A &pos, XMFLOAT3A &dir)
{
	const XMFLOAT3A
		camPos = _currCameraPtr->GetTransform()->GetPosition(World),
		camDir = _currCameraPtr->GetTransform()->GetForward(World);
	dir = camDir;

	if (!GetScene()->GetSceneHolder()->RaycastScene(
		{ camPos.x, camPos.y, camPos.z },
		{ camDir.x, camDir.y, camDir.z },
		out))
	{
		return false;
	}

	pos = {
		camPos.x + (camDir.x * out.distance),
		camPos.y + (camDir.y * out.distance),
		camPos.z + (camDir.z * out.distance)
	};

	return true;
}
bool DebugPlayerBehaviour::RayCastFromMouse(RaycastOut &out, const Input &input)	// UNFINISHED
{
	// Get window width and height
	MouseState mouseState = input.GetMouse();
	WindowSize windowSize = input.GetActiveWindowSize();

	// Wiewport to NDC coordinates
	float xNDC = ((2.0f * mouseState.x) / (float)windowSize.width) - 1.0f;
	float yNDC = 1.0f - ((2.0f * mouseState.y) / (float)windowSize.height); // can also be - 1 depending on coord-system
	float zNDC = 1.0f;	// not really needed yet (specified anyways)
	XMFLOAT3A ray_clip = XMFLOAT3A(xNDC, yNDC, zNDC);

	// Wiew space -> clip space
	XMVECTOR rayClipVec = Load(ray_clip);
	XMMATRIX projMatrix = Load(_currCameraPtr->GetProjectionMatrix());
	XMVECTOR rayEyeVec = XMVector4Transform(rayClipVec, XMMatrixInverse(nullptr, projMatrix));

	// Set z and w to mean forwards and not a point
	rayEyeVec = XMVectorSet(XMVectorGetX(rayEyeVec), XMVectorGetY(rayEyeVec), 1, 0.0);

	// Clip space -> world space
	XMMATRIX viewMatrix = Load(_currCameraPtr->GetViewMatrix());
	XMVECTOR rayWorldVec = XMVector4Transform(rayEyeVec, XMMatrixInverse(nullptr, viewMatrix));

	rayWorldVec = XMVector4Normalize(rayWorldVec);	// Normalize
	XMFLOAT3A dir; Store(dir, rayWorldVec);

	// Camera 
	XMFLOAT3A camPos = _currCameraPtr->GetTransform()->GetPosition(World);

	// Perform raycast
	if (!GetScene()->GetSceneHolder()->RaycastScene(
		{ camPos.x, camPos.y, camPos.z },
		{ dir.x, dir.y, dir.z },
		out))
	{
		return false;
	}

	return true;
}
bool DebugPlayerBehaviour::RayCastFromMouse(RaycastOut &out, XMFLOAT3A &pos, XMFLOAT3A &dir, const Input &input)
{
	// Get window width and height
	MouseState mouseState = input.GetMouse();
	WindowSize windowSize = input.GetActiveWindowSize();

	// Wiewport to NDC coordinates
	float xNDC = ((2.0f * mouseState.x) / (float)windowSize.width) - 1.0f;
	float yNDC = 1.0f - ((2.0f * mouseState.y) / (float)windowSize.height); // can also be - 1 depending on coord-system
	float zNDC = 1.0f;	// not really needed yet (specified anyways)
	XMFLOAT3A ray_clip = XMFLOAT3A(xNDC, yNDC, zNDC);

	// Wiew space -> clip space
	XMVECTOR rayClipVec = Load(ray_clip);
	XMMATRIX projMatrix = Load(_currCameraPtr->GetProjectionMatrix());
	XMVECTOR rayEyeVec = XMVector4Transform(rayClipVec, XMMatrixInverse(nullptr, projMatrix));

	// Set z and w to mean forwards and not a point
	rayEyeVec = XMVectorSet(XMVectorGetX(rayEyeVec), XMVectorGetY(rayEyeVec), 1, 0.0);

	// Clip space -> world space
	XMMATRIX viewMatrix = Load(_currCameraPtr->GetViewMatrix());
	XMVECTOR rayWorldVec = XMVector4Transform(rayEyeVec, XMMatrixInverse(nullptr, viewMatrix));

	rayWorldVec = XMVector4Normalize(rayWorldVec);	// Normalize
	Store(dir, rayWorldVec);

	// Camera 
	XMFLOAT3A camPos = _currCameraPtr->GetTransform()->GetPosition(World);

	// Perform raycast
	if (!GetScene()->GetSceneHolder()->RaycastScene(
		{ camPos.x, camPos.y, camPos.z },
		{ dir.x, dir.y, dir.z },
		out))
	{
		return false;
	}

	pos = {
		camPos.x + (dir.x * out.distance),
		camPos.y + (dir.y * out.distance),
		camPos.z + (dir.z * out.distance)
	};

	return true;
}
#endif