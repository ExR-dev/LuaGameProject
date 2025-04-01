#include "stdafx.h"
#include "PlayerMovementBehaviour.h"
#include "Entity.h"
#include "Scene.h"
#include "Intersections.h"
#include "MonsterBehaviour.h"
#include "InventoryBehaviour.h"
#include <cmath>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif
#include "RestrictedViewBehaviour.h"

using namespace DirectX;

bool PlayerMovementBehaviour::Start()
{
	if (_name == "")
		_name = "PlayerMovementBehaviour"; // For categorization in ImGui.

	//_spawnPosition = GetTransform()->GetPosition(World);
	//_spawnPosition = { 67.0f, -1.0f, 10.0f };
	//GetTransform()->SetPosition(_spawnPosition);

	_spawnPositions.push_back({ -119.0f, 5.5f, -283.0f });
	_spawnPositions.push_back({ -1.0f, 5.0f, -10.0f });
	_spawnPositions.push_back({ -200.0f, -4.0f, 74.0f });
	_spawnPositions.push_back({ -54.0f, 14.0f, 183.0f });
	_spawnPositions.push_back({ 131.0f, -3.0f, -17.0f });
	_spawnPositions.push_back({ -35.0f, -32.0f, -187.0f });

	// Initialize Audio
	_steps.push_back(new SoundBehaviour("footstep_concrete_000", SoundEffectInstance_Default));
	_steps.push_back(new SoundBehaviour("footstep_concrete_001", SoundEffectInstance_Default));
	_steps.push_back(new SoundBehaviour("footstep_concrete_002", SoundEffectInstance_Default));
	_steps.push_back(new SoundBehaviour("footstep_concrete_003", SoundEffectInstance_Default));
	_steps.push_back(new SoundBehaviour("footstep_concrete_004", SoundEffectInstance_Default));

	for (int i = 0; i < _steps.size(); i++)
	{
		if (!_steps.at(i)->Initialize(GetEntity()))
		{
			ErrMsg("Failed to initialize footstep sound!");
			return false;
		}
		_steps.at(i)->SetSerialization(false);
		_steps.at(i)->SetVolume(0.01f);
		_steps.at(i)->SetEnabled(false);
	}

	_collapseSound = new SoundBehaviour("BodyCollapse", SoundEffectInstance_ReverbUseFilters, false, 75.0f, 0.5f);
	if (!_collapseSound->Initialize(GetEntity()))
	{
		ErrMsg("Failed to initialize collapse sound!");
		return false;
	}
	_collapseSound->SetSerialization(false);
	_collapseSound->SetVolume(0.02f);

	// Breath sound initialization
	{
		std::vector<std::string> breathSounds = { "normal_recovery", "depleted_recovery" };

		for (auto& sound : breathSounds)
		{
			_breathSounds.insert({ sound, new SoundBehaviour("breath_" + sound, SoundEffectInstance_Default, false)});
			if (!_breathSounds[sound]->Initialize(GetEntity()))
			{
				ErrMsg("Failed to initialize breath sound " + sound + "!");
				return false;
			}
			_breathSounds[sound]->SetSerialization(false);
			_breathSounds[sound]->SetVolume(0.05f);
			_breathSounds[sound]->SetEnabled(false);
		}
	}

	GetTerrain();

	return true;
}

bool PlayerMovementBehaviour::Update(Time &time, const Input &input)
{
	SceneHolder *sceneHolder = GetScene()->GetSceneHolder();

	if (!_flashlight)
		_flashlight = sceneHolder->GetEntityByName("Flashlight");

	if (!_playerCamera)
	{
		Entity *playerCamEnt = sceneHolder->GetEntityByName("playerCamera");
		CameraBehaviour *playerCam;

		if (!playerCamEnt->GetBehaviourByType<CameraBehaviour>(playerCam))
			return true;

		if (!SetPlayerCamera(playerCam))
		{
			ErrMsg("Failed to set player camera!");
			return false;
		}
	}

	if (!_headTrackerEntity)
	{
		_headTrackerEntity = sceneHolder->GetEntityByName("Player Head Tracker");

		if (!_headTrackerEntity)
			return true;
	}

	if (!_terrainFloor || !_terrainRoof || !_terrainWalls)
		if (!GetTerrain())
			return true;

	if (_isCaught)
	{
		_currentBreathSound = nullptr;
		PlayCaughtAnimnation(time);
	}
	else
	{
		// Keyboard input
		bool forward = BindingCollection::IsTriggered(InputBindings::InputAction::WalkForward);
		bool backward = BindingCollection::IsTriggered(InputBindings::InputAction::WalkBackward);
		bool left = BindingCollection::IsTriggered(InputBindings::InputAction::StrafeLeft);
		bool right = BindingCollection::IsTriggered(InputBindings::InputAction::StrafeRight);
		bool sneak = BindingCollection::IsTriggered(InputBindings::InputAction::Sneak);
		bool run = BindingCollection::IsTriggered(InputBindings::InputAction::Sprint);

		// Play the current breath sound
		if (_currentBreathSound)
		{
			if (!_currentBreathSound->IsEnabled())
				_currentBreathSound->SetEnabled(true);

			_currentBreathSound->Play();
		}

		//// Testing
		//bool forward = false;
		//bool backward = false;
		//bool left = false;
		//bool right = false;
		//bool sneak = false;
		//bool run = false;

		// State determination
		if (sneak) // Sneaking
		{
			_velocity = _sneakVelocity;
			_isSneaking = true;
			_isWalking = false;
			_isRunning = false; 

			//_headTrackerEntity->GetTransform()->SetPosition({ 0, _cameraHeight * 0.425f, 0 });
		}
		else 
		{
			if (run && _canRun && !backward) // Running
			{
				_velocity = _runVelocity;
				_isSneaking = false;
				_isWalking = false;
				_isRunning = true;

				// Stamina drain
				_stamina -= _staminaDrainRate * time.deltaTime;
				if (_stamina <= 0.0f) _canRun = false;
				
				// Calculating from a cosinus curve the headbobbing
				float height = abs(cos(_stepProgress));
				//_headTrackerEntity->GetTransform()->SetPosition({ 0, 0.95f * _cameraHeight -  0.3f * height, 0});
				_stepProgress += 3;
			}
			else // Walking
			{
				_velocity = _walkVelocity;
				_isSneaking = false;
				_isWalking = true;
				_isRunning = false;
				float height = abs(cos(_stepProgress));
				//_headTrackerEntity->GetTransform()->SetPosition({ 0, 0.90f*  _cameraHeight - 0.15f * height, 0 });
				_stepProgress += 2;
			}

			
		}

		if (_isHiding) {
			_headTrackerEntity->GetTransform()->SetPosition({ 0, _cameraHeight * 0.95f, 0 });
		}

		// Recover stamina when not running
		if (!_isRunning && _stamina < _staminaMax)
		{
			if (_canRun)
				_currentBreathSound = _breathSounds["normal_recovery"];
			else
				_currentBreathSound = _breathSounds["depleted_recovery"];

			_stamina += _staminaRecoveryRate * time.deltaTime;
			if (_stamina > _staminaMax)
			{
				_stamina = _staminaMax;
				_currentBreathSound = nullptr;
			}

			if (_stamina > _staminaMax * 0.4f)
			{
				_canRun = true;
			}
		}

		Transform *playerT = GetTransform();

		// Mouse Input
		if (_playerCamera && !_isHiding)
		{
			if (input.IsCursorLocked())
			{
				const MouseState mState = input.GetMouse();
				if (mState.dx != 0.0f)
				{
					XMFLOAT3A up = playerT->GetUp();
					float invert = SIGN(XMVectorGetX(XMVector3Dot(Load(up), { 0.0f, 1.0f, 0.0f, 0.0f })));
					playerT->RotateAxis(up, (static_cast<float>(mState.dx) / 360.0f) * invert, Local);
				}

				if (mState.dy != 0)
				{
					// Limit up and down movement to 90 degrees
					XMFLOAT3A camFwd = _headTrackerEntity->GetTransform()->GetForward();
					bool rotate = true;

					if (mState.dy < 0)	// up rotation
					{
						if (camFwd.y <= 0.99f) 
							rotate = true;
						else if (camFwd.y >= -0.99f) // down rotation
							rotate = true;
					}

					_headTrackerEntity->GetTransform()->RotatePitch(static_cast<float>(mState.dy) / 360.0f, Local);
					if (_headTrackerEntity->GetTransform()->GetUp().y < 0.01f)
						_headTrackerEntity->GetTransform()->RotatePitch(static_cast<float>(-mState.dy) / 360.0f, Local); // Rotate back if too far
				}
			}
		}

		// Calculate movement vector in entity local space
		XMFLOAT3A	localForward =		playerT->GetForward(Local);
		XMFLOAT3A	localRight =		playerT->GetRight(Local);
		XMVECTOR	localForwardVec =	XMVector3Normalize(Load(localForward));
		XMVECTOR	localRightVec =		XMVector3Normalize(Load(localRight));

		XMVECTOR moveVec = { 0.0f, 0.0f, 0.0f };
		if (forward)	moveVec = XMVectorAdd(moveVec, localForwardVec);
		if (backward)	moveVec = XMVectorAdd(moveVec, XMVectorScale(localForwardVec, -1.0f));
		if (left)		moveVec = XMVectorAdd(moveVec, XMVectorScale(localRightVec, -1.0f));
		if (right)		moveVec = XMVectorAdd(moveVec, localRightVec);

		if (forward || backward || left || right) // if there is movement input in any direction
		{

			_isStill = false;
			moveVec = XMVector3Normalize(moveVec); // Normalized movement direction in entity local space

			// Calculate velocity multiplier based on input direction in relation to local forward
			float dot; XMStoreFloat(&dot, XMVector3Dot(moveVec, localForwardVec));
			float dirMultiplier = 0;
			if (dot >= 0.0f)		dirMultiplier = _sidewaysMultiplier + dot * (_forwardsMultiplier - _sidewaysMultiplier);			// interpolate (sideways -> forwards)
			else					dirMultiplier = _backwardsMultiplier + (dot + 1.0f) * (_sidewaysMultiplier - _backwardsMultiplier);	// interpolate (sideways -> backwards)


			float moveScalar = (_velocity * time.deltaTime) * dirMultiplier;
			moveVec = XMVectorScale(XMVector3Normalize(moveVec), moveScalar);
			XMFLOAT3A move; Store(move, moveVec);

			// Move the entity
			if (!_isHiding)
			{
				playerT->Move(move, Local);
				_movementDir = move;
				_lastMoveDir = move;
				_lastMoveSpeed = moveScalar;
			}		
			
			float playerRadius = static_cast<const Collisions::Capsule *>(_playerCollider->GetCollider())->radius;
			Transform *playerT = GetTransform();
			XMFLOAT3 playerPos = playerT->GetPosition(World);

			Collisions::Circle c = { {playerPos.x, playerPos.z}, playerRadius/2 };
			XMFLOAT2 n1;
			float d;
			if (Collisions::CircleTerrainWallIntersection(c, *_terrainWalls, d, n1))
				playerT->Move({n1.x*d, 0, n1.y*d}, World);


					
			// Footstep audio
			if (_isRunning)			_stepInterval = 0.3f;
			else if (_isSneaking)	_stepInterval = 0.6f;
			else					_stepInterval = 0.45f;
			
			_stepTimer -= time.deltaTime;
			if (_stepTimer <= 0)
			{
				// Get a random index
				int soundIndex = rand() % _steps.size();

				// Play the sound
				_steps.at(soundIndex)->SetEnabled(true);
				_steps.at(soundIndex)->Play();

				// Reset the timer
				_stepTimer = 0;
				_stepTimer += _stepInterval;
			}

		}
		else
		{
			// Entity is not moving
			_velocity = 0.0f;
			_isStill = true;
			_isWalking = false;
			_isRunning = false;
			_movementDir = { 0.0f, 0.0f, 0.0f };
			if (!_isSneaking && !_isHiding) {
				// reset head bobbin when idle
				_headTrackerEntity->GetTransform()->SetPosition({ 0, _cameraHeight * 0.85f, 0 });
				_stepProgress = 0;

			}
		}

#ifdef DEBUG_BUILD
		if (input.GetKey(KeyCode::Space) == KeyState::Pressed)
			playerT->Move({ 0, 2.0f, 0 }, World); // "Jump"

		if (input.GetKey(KeyCode::M) == KeyState::Pressed)
			playerT->Move({ 0, 30.0f, 0 }, World); // "Super Jump"
#endif
	}


	if (_collapseSound->IsEnabled())
	{
		_collapseSound->SetEmitterPosition(GetTransform()->GetPosition(World));
		_collapseSound->Play();
	}

	return true;
}

bool PlayerMovementBehaviour::LateUpdate(Time &time, const Input &input)
{
	GetTransform()->Move({ 0, -5.0f * std::clamp(time.deltaTime, 0.0f, 0.05f), 0}, World); // "Gravity"

	Transform *playerT = GetTransform();
	float playerHeight = static_cast<const Collisions::Capsule *>(_playerCollider->GetCollider())->height;

	if (_terrainRoof)
	{
		XMFLOAT3 playerPos = playerT->GetPosition(World),
			rayPos = playerPos;

		XMFLOAT3A n, floorPoint, roofPoint, newPos;
		float d;
		bool under;

		rayPos.y -= 200;
		Collisions::Ray rayUp = Collisions::Ray(rayPos, { 0, 1.0f, 0 }, 1000);
		bool collidingWithRoof = Collisions::TerrainRayIntersectionVertical(*_terrainRoof, rayUp, n, roofPoint, d, under);

		float newY = roofPoint.y - playerHeight;
		if (collidingWithRoof && playerPos.y > newY)
			playerT->SetPosition({ playerPos.x, newY, playerPos.z });
	}

	if (_terrainFloor)
	{
		XMFLOAT3 playerPos = playerT->GetPosition(World);
		XMFLOAT3 rayPos = playerPos;

		XMFLOAT3A n, floorPoint, roofPoint, newPos;
		float d;
		bool under;

		rayPos.y += 200;
		Collisions::Ray rayDown = Collisions::Ray(rayPos, { 0, -1.0f, 0 }, 1000);
		bool collidingWithFloor = Collisions::TerrainRayIntersectionVertical(*_terrainFloor, rayDown, n, floorPoint, d, under);
		
		if (collidingWithFloor && playerPos.y < floorPoint.y)
			playerT->SetPosition({playerPos.x, floorPoint.y, playerPos.z}, World);
	}

	return true;
}

#ifdef USE_IMGUI
bool PlayerMovementBehaviour::RenderUI()
{
	// Value modification
	ImGui::InputFloat("Walk Velocity", &_walkVelocity, 0.01f, 1.0f);
	ImGui::InputFloat("Run Velocity", &_runVelocity, 0.01f, 1.0f);
	ImGui::InputFloat("Sneak Velocity", &_sneakVelocity, 0.01f, 1.0f);
	ImGui::InputFloat("Stamina Drain Rate", &_staminaDrainRate, 1.0f, 1.0f);
	ImGui::InputFloat("Stamina Recovery Rate", &_staminaRecoveryRate, 1.0f, 1.0f);

	ImGui::InputFloat("Forwards Multiplier", &_forwardsMultiplier, 0.01f, 1.0f);
	ImGui::InputFloat("Sideways Multiplier", &_sidewaysMultiplier, 0.01f, 1.0f);
	ImGui::InputFloat("Backwards Multiplier", &_backwardsMultiplier, 0.01f, 1.0f);

	ImGui::SliderFloat("Solid Footing Dot", &_solidFootingDot, 0.0f, 1.0f);

	// Value display
	//std::string testText = "Test: " + std::to_string(test);
	//ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), testText.c_str());

	std::string staminaText = "Stamina: " + std::to_string(_stamina);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), staminaText.c_str());

	std::string speedText = "Velocity: " + std::to_string(_velocity);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), speedText.c_str());

	std::string directionText = "Direction: " + std::to_string(_movementDir.x) + ", " + std::to_string(_movementDir.y) + ", " + std::to_string(_movementDir.z);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), directionText.c_str());

	Transform *playerT = GetEntity()->GetTransform();
	XMFLOAT3A	localForward = playerT->GetForward();
	std::string forwardText = "Forward: " + std::to_string(localForward.x) + ", " + std::to_string(localForward.y) + ", " + std::to_string(localForward.z);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), forwardText.c_str());

	if (ImGui::Button("Reset Player"))
	{
		XMFLOAT3A point;
		GetScene()->GetGraphManager()->GetClosestPoint(playerT->GetPosition(World), point);
		GetTransform()->SetPosition(point, World);
	}
	
	if (ImGui::Button("Simulated Catch"))
	{
		Catch();
	}


	return true;
}
#endif

bool PlayerMovementBehaviour::Serialize(std::string *code) const
{
	*code += _name + "(" 
		+ std::to_string(_stamina) + " " + std::to_string(_isHiding) +
		" )";
	return true;
}

bool PlayerMovementBehaviour::Deserialize(const std::string &code)
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

	_stamina = attributes.at(0);
	_isHiding = attributes.at(1);
	return true;
}


void PlayerMovementBehaviour::AdjustForCollision(const Collisions::CollisionData &data)
{
	using namespace DirectX;

	bool hasSolidFooting = data.normal.y > 1.0f - _solidFootingDot;
	//hasSolidFooting = false;
	float div = 1.0f;
	//float div = 0.5f;
	//float div = 1.2f;

	XMFLOAT3A move = {0, 0, 0};
	if (hasSolidFooting) // Prevent the player from sliding by converting horizontal movement to additional vertical movement
		move.y = data.depth * div;
	else
		Store(move, XMVectorScale(XMVector3Normalize(XMLoadFloat3(&data.normal)), data.depth * div));

	Transform *t = GetEntity()->GetTransform();
	t->Move(move, World);
}

bool PlayerMovementBehaviour::GetTerrain()
{
	Entity *ent = GetScene()->GetSceneHolder()->GetEntityByName("Terrain Floor");
	if (ent)
		if (ent->GetBehaviourByType<const ColliderBehaviour>(_floorColliderBehaviour))
			 _terrainFloor = dynamic_cast<const Collisions::Terrain *>(_floorColliderBehaviour->GetCollider());

	ent = GetScene()->GetSceneHolder()->GetEntityByName("Terrain Roof");
	if (ent)
		if (ent->GetBehaviourByType<const ColliderBehaviour>(_roofColliderBehaviour))
			 _terrainRoof = dynamic_cast<const Collisions::Terrain *>(_roofColliderBehaviour->GetCollider());

	ent = GetScene()->GetSceneHolder()->GetEntityByName("Terrain Walls");
	if (ent)
		if (ent->GetBehaviourByType<const ColliderBehaviour>(_wallsColliderBehaviour))
			 _terrainWalls = dynamic_cast<const Collisions::Terrain *>(_wallsColliderBehaviour->GetCollider());

	return _terrainFloor != nullptr && _terrainRoof != nullptr && _terrainWalls != nullptr;
}

void PlayerMovementBehaviour::RespawnPlayer()
{
	if (!_spawnPositions.empty())
	{
		GetTransform()->SetPosition(_spawnPositions.at(rand() % _spawnPositions.size()), World);
	}
	else
	{
		GetTransform()->SetPosition(_spawnPosition);
	}

	_canRun = true;
	_isStill = true;
	_isSneaking = false;
	_isWalking = false;
	_isRunning = false;
	_isHiding = false;
}

void PlayerMovementBehaviour::SetHiding(bool value)
{
	_isHiding = value;
}

void PlayerMovementBehaviour::SetStill(bool value)
{
	_isStill = value;
}

void PlayerMovementBehaviour::SetSneaking(bool value)
{
	_isSneaking = value;
}

void PlayerMovementBehaviour::SetWalking(bool value)
{
	_isWalking = value;
}

void PlayerMovementBehaviour::SetRunning(bool value)
{
	_isRunning = value;
}

bool PlayerMovementBehaviour::IsHiding() const
{
	return _isHiding;
}

bool PlayerMovementBehaviour::IsStill() const
{
	return _isStill;
}

bool PlayerMovementBehaviour::IsSneaking() const
{
	return _isSneaking;
}

bool PlayerMovementBehaviour::IsWalking() const
{
	return _isWalking;
}

bool PlayerMovementBehaviour::IsRunning() const
{
	return _isRunning;
}

bool PlayerMovementBehaviour::MoveToPoint(Time &time, DirectX::XMFLOAT3 point, float velocity, float threshold2)
{
	XMVECTOR moveVec = XMVectorSubtract(Load(point), Load(GetTransform()->GetPosition(World)));
	if (XMVectorGetX(XMVector3Dot(moveVec, moveVec)) < threshold2)
		return true;

	moveVec = XMVector3Normalize(moveVec);
	float moveScalar = (velocity * time.deltaTime);

	moveVec = XMVectorScale(XMVector3Normalize(moveVec), moveScalar);

	// Move the entity
	if (!_collidingWithWall && !_isHiding)
	{
		XMFLOAT3A move; Store(move, moveVec);
		GetTransform()->Move(move, Local);
		_movementDir = move;
		_lastMoveDir = move;
		_lastMoveSpeed = moveScalar;
	}

	if (_isRunning || velocity >= _runVelocity)
		_stepInterval = 0.3f;
	else if (_isSneaking || velocity <= _sneakVelocity)
		_stepInterval = 0.6f;
	else
		_stepInterval = 0.45f;
	
	_stepTimer -= time.deltaTime;
	if (_stepTimer <= 0)
	{
		// Get a random index
		int soundIndex = rand() % _steps.size();

		// Play the sound
		_steps.at(soundIndex)->SetEnabled(true);
		_steps.at(soundIndex)->Play();

		// Reset the timer
		_stepTimer = 0;
		_stepTimer += _stepInterval;
	}

	return false;
}

bool PlayerMovementBehaviour::SetPlayerCamera(CameraBehaviour *camera)
{
	if (!camera)
	{
		ErrMsg("Failed to set player camera, camera is nullptr");
		return false;
	}

	_playerCamera = camera;
	GetEntity()->GetBehaviourByType<ColliderBehaviour>(_playerCollider);

	if (_playerCollider)
		_playerCollider->AddOnIntersection([this](const Collisions::CollisionData &data) { AdjustForCollision(data); });

	return true;
}

void PlayerMovementBehaviour::SetHeadTracker(Entity *ent)
{
	_headTrackerEntity = ent;
}

Transform *PlayerMovementBehaviour::GetPlayerCameraDesiredPos() const
{
	return _headTrackerEntity->GetTransform();
}

float PlayerMovementBehaviour::GetCameraHeight() const
{
	return _cameraHeight;
}

bool PlayerMovementBehaviour::IsCaught()
{
	return _isCaught;
}

void PlayerMovementBehaviour::Catch()
{
	_caughtStage = 0;
	_isCaught = true;
	_caughtStageTimer = 0.25f;
	GetScene()->GetGraphics()->BeginScreenFade(_caughtStageTimer);
	_collapseSound->SetEnabled(true);
	_collapseSound->ResetSound();
	_collapseSound->Play();
}

void PlayerMovementBehaviour::PlayCaughtAnimnation(Time &time)
{
	switch (_caughtStage)
	{
	case 0: 
		// Wait for fade to black
		if (_caughtStageTimer <= 0)
		{
			// End of stage, prepare next stage
			_caughtStage++;
			_caughtStageTimer += 3.0f;

			if (_flashlight)
				_flashlight->Disable();

			InventoryBehaviour* inventory = nullptr;
			if (GetEntity()->GetBehaviourByType<InventoryBehaviour>(inventory))
			{
				inventory->SetHeldEnabled(false);
				inventory->SetEnabled(false);
			}


		}
		break;

	case 1: 
		// Sustain black screen
		if (_caughtStageTimer <= 0)
		{
			// End of stage, prepare next stage
			RespawnPlayer();
			GetScene()->GetGraphics()->BeginScreenFade(-2.0f);

			if (GetScene()->GetViewCamera() == GetScene()->GetPlayerCamera())
			{
				CameraBehaviour *animCam = GetScene()->GetAnimationCamera();
				GetScene()->SetViewCamera(animCam);

				animCam->GetTransform()->SetPosition({ -269.3f, -1.95f, -73.98f }, World);
				animCam->GetTransform()->SetEuler({ -12.4802f * DEG_TO_RAD, -70.137f * DEG_TO_RAD, 20.0f * DEG_TO_RAD }, World);

				RestrictedViewBehaviour *restrictedViewBehaviour;
				if (animCam->GetEntity()->GetBehaviourByType<RestrictedViewBehaviour>(restrictedViewBehaviour))
				{
					restrictedViewBehaviour->SetEnabled(true);
					restrictedViewBehaviour->SetAllowedOffset(15.0f);
					restrictedViewBehaviour->SetViewDirection(
						animCam->GetTransform()->GetEuler(World), 
						animCam->GetTransform()->GetForward(World), 
						animCam->GetTransform()->GetUp(World)
					);
				}

				SimplePointLightBehaviour *lightBehaviour;
				if (animCam->GetEntity()->GetBehaviourByType<SimplePointLightBehaviour>(lightBehaviour))
					lightBehaviour->SetEnabled(true);

				SoundBehaviour *dragSound;
				if (animCam->GetEntity()->GetBehaviourByType<SoundBehaviour>(dragSound))
				{
					dragSound->SetEnabled(true);
					dragSound->ResetSound();
					dragSound->Play();
				}
			}

			_caughtStage++;
			_caughtStageTimer += 7.5f;
		}
		break;

	case 2:
		// Dragging animation
		{
			CameraBehaviour *animCam = GetScene()->GetAnimationCamera();
			float moveSpeed = time.deltaTime * powf(
				std::clamp(
					(sinf((_caughtStageTimer - 0.5f) * 2.75f - 0.5f) + 0.9f) / 1.9f,
					0.0f, 1.0f),
				0.25f
			);

			animCam->GetTransform()->Move({ 1.25f * moveSpeed, 0, 0 }, World);

			SoundBehaviour *dragSound;
			if (animCam->GetEntity()->GetBehaviourByType<SoundBehaviour>(dragSound))
				dragSound->Play();

			XMFLOAT3A distortionOrigin = animCam->GetTransform()->GetPosition(World);
			distortionOrigin.y += 1.5f;
			distortionOrigin.x += 1.5f;

			GetScene()->GetGraphics()->SetDistortionOrigin(distortionOrigin);
			GetScene()->GetGraphics()->SetDistortionStrength(10.0f);

			if (_caughtStageTimer <= 2.0f)
				GetScene()->GetGraphics()->BeginScreenFade(2.0f);
		}

		if (_caughtStageTimer <= 0)
		{
			// End of stage, prepare next stage
			CameraBehaviour *animCam = GetScene()->GetAnimationCamera();

			RestrictedViewBehaviour *restrictedViewBehaviour;
			if (animCam->GetEntity()->GetBehaviourByType<RestrictedViewBehaviour>(restrictedViewBehaviour))
				restrictedViewBehaviour->SetEnabled(false);

			SimplePointLightBehaviour *lightBehaviour;
			if (animCam->GetEntity()->GetBehaviourByType<SimplePointLightBehaviour>(lightBehaviour))
				lightBehaviour->SetEnabled(false);

			SoundBehaviour *dragSound;
			if (animCam->GetEntity()->GetBehaviourByType<SoundBehaviour>(dragSound))
			{
				//dragSound->ResetSound();
				//dragSound->SetEnabled(false);
			}

			_caughtStage++;
			_caughtStageTimer += 2.0f;
		}
		break;

	case 3: 
		// Reset camera, wait
		if (_caughtStageTimer <= 0)
		{
			// End of stage, prepare next stage
			if (GetScene()->GetViewCamera() == GetScene()->GetAnimationCamera())
			{
				GetScene()->SetViewCamera(GetScene()->GetPlayerCamera());
			}

			_caughtStage++;
			_caughtStageTimer += 1.0f;
		}
		break;

	case 4: 
		// Fade in
		if (_caughtStageTimer <= 0)
		{
			GetScene()->GetGraphics()->BeginScreenFade(-2.5f);

			// End of stage, finish caught sequence
			_caughtStage = 0;
			_isCaught = false;
			_caughtStageTimer = 0.0f;

			if (_flashlight)
				_flashlight->Enable();

			InventoryBehaviour* inventory = nullptr;
			if (GetEntity()->GetBehaviourByType<InventoryBehaviour>(inventory))
			{
				inventory->SetHeldEnabled(true);
				inventory->SetEnabled(false);
			}


			GetScene()->GetMonster()->GetEntity()->Enable();
			GetScene()->GetMonster()->ResetMonster();
		}
		break;
	}

	_caughtStageTimer -= time.deltaTime;
}
