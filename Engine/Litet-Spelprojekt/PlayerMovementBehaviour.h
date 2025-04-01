#pragma once
#include "Behaviour.h"
#include "DirectXMath.h"
#include "ColliderBehaviour.h"
#include "SoundBehaviour.h"

class PlayerMovementBehaviour : public Behaviour
{
private:
	DirectX::XMFLOAT3A _spawnPosition = { 0, 0, 0 };
	std::vector<DirectX::XMFLOAT3A> _spawnPositions;

	float _velocity = 0.0f;	// Per second
	DirectX::XMFLOAT3A _movementDir = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3A _lastMoveDir = { 0.0f, 0.0f, 0.0f };
	float _lastMoveSpeed = 0.0f;

	bool _collidingWithWall = false;

	// States
	float _walkVelocity = 4.0f;
	float _runVelocity = 8.5f;
	float _sneakVelocity = 2.0f;
	float _solidFootingDot = 0.15f; // Maximum difference in dot product of the player's up vector and the terrain's normal, used for determining if the player is on solid footing

	bool _canRun = true;
	bool _isStill = true;
	bool _isSneaking = false; // crouch walk (not crouching alone)
	bool _isWalking = false;
	bool _isRunning = false;
	bool _isHiding = false;
	bool _isCaught = false;
	int _caughtStage = 0;
	float _caughtStageTimer = 0.0f;

	// Multipliers
	float _forwardsMultiplier = 1.0f;
	float _sidewaysMultiplier = 0.9f;
	float _backwardsMultiplier = 0.8f;

	// Stamina
	float _stamina = 100.0f;
	float _staminaMax = 100.0f;
	float _staminaDrainRate = 33.0f; // Per second
	float _staminaRecoveryRate = 10.0f; // Per second

	float _stepProgress = 0.0f; // Degrees on the cosinus curve 

	// Step sounds
	float _stepTimer = 0.0f;
	float _stepInterval = 0.5f;

	Entity *_flashlight = nullptr;
	CameraBehaviour *_playerCamera = nullptr;
	ColliderBehaviour *_playerCollider = nullptr;
	Collisions::Ray _rayCollider;
	const Collisions::Terrain *_terrainFloor = nullptr, 
							  *_terrainRoof = nullptr,
							  *_terrainWalls = nullptr;
	const ColliderBehaviour *_floorColliderBehaviour = nullptr,
							*_roofColliderBehaviour = nullptr,
							*_wallsColliderBehaviour = nullptr;

	Entity *_headTrackerEntity = nullptr;
	float _cameraHeight = 1.8f;

	// Audio
	std::vector<SoundBehaviour *> _steps;
	SoundBehaviour *_collapseSound = nullptr;
	std::unordered_map<std::string, SoundBehaviour*> _breathSounds;
	SoundBehaviour* _currentBreathSound = nullptr;

	void AdjustForCollision(const Collisions::CollisionData &data);
	bool GetTerrain();

protected:
	[[nodiscard]] bool Start() override;

	[[nodiscard]] bool Update(Time &time, const Input &input) override;
	[[nodiscard]] bool LateUpdate(Time &time, const Input &input) override;

#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI() override;
#endif

public:
	PlayerMovementBehaviour() = default;
	~PlayerMovementBehaviour() = default;

	bool SetPlayerCamera(CameraBehaviour *camera);
	void SetHeadTracker(Entity *ent);

	Transform *GetPlayerCameraDesiredPos() const;

	float GetCameraHeight() const;

	bool IsCaught();
	void Catch();
	void PlayCaughtAnimnation(Time &time);
	void RespawnPlayer();

	void SetHiding(bool value);
	void SetStill(bool value);
	void SetSneaking(bool value);
	void SetWalking(bool value);
	void SetRunning(bool value);

	bool IsHiding() const;
	bool IsStill() const;
	bool IsSneaking() const;
	bool IsWalking() const;
	bool IsRunning() const;

	bool MoveToPoint(Time &time, DirectX::XMFLOAT3 point, float velocity, float threshold2 = 0.5);


	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};