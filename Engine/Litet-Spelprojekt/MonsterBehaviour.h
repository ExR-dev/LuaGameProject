#pragma once
#include <queue>

#include "Behaviour.h"
#include "MeshBehaviour.h"
#include "SoundBehaviour.h"
#include "PlayerMovementBehaviour.h"
#include "FlashLightBehaviour.h"
#include "GraphManager.h"

class MonsterState;
class MonsterIdle;
class MonsterHunt;
class MonsterHuntDone;
class MonsterExtendedHunt;
class MonsterCapture;

enum MonsterStatus
{
	IDLE,
	HUNT,
	HUNT_DONE,
	EXTENDED_HUNT,
	CAPTURE,

	MONSTER_STATE_COUNT,
};

// Idle State: Monster generates alertlevel based on distance
// Hunt State: Unused
enum class NoiseLevel : int
{
	Hiding = 0,
	Still = 0,
	Sneaking = 6,
	Walking = 15,
	Running = 30,
	Dancing = 100, // :)
};

// Idle State: Unused
// Hunt State: Monster can pinpoint player location if within range (meters)
enum class NoiseRange : int
{
	Hiding = 0,
	Still = 2,
	Sneaking = 8,
	Walking = 10,
	Running = 15,
};

enum MonsterSound
{
	HUNTING1,
	HUNTING2,
	HUNTING3,
	HUNTING_AMBIENT,
	HEART_SLOW,
	HEART_MEDIUM,
	HEART_FAST,

	SOUND_COUNT,
	NULL_SOUND
};

class MonsterBehaviour final : public Behaviour
{
friend class MonsterState; 
friend class MonsterIdle; 
friend class MonsterHunt;
friend class MonsterHuntDone;
friend class MonsterExtendedHunt;
friend class MonsterCapture;

private:
	// Alert
	float _alertLevel = 0.0f; // 0 = not alert, 100 = fully alert
	float _baseAlertLevel = 0.0f;
	float _huntTriggerAlertLevel = 80.0f;

	float _alertDecayRate = 0.0f;
	float _alertDecayRateIdle = 2.0f;
	float _alertDecayRateHunt = 4.0f;
	float _alertDecayRateHuntDone = 10.0f;

	float _alertGrowthRate = 10.0f;

	float _idleAlertFactor = 1.0f;
	float _huntDoneAlertFactor = 1.3f;
	float _huntAlertFactor = 1.5f; // Unused

	// Sight
	bool _playerInSight = false;
	float _sightRange = 50.0f;

	// Hearing
	float _hearingRange = 100.0f;
	int _playerNoiseRange = 0;
	int _playerNoiseLevel = 0;

	// Distance
	float _playerDistance = 0.0f;
	float _playerCaptureDistance = 2.0f;
	float _distanceEffect = 0.005f;
	float _distanceEffectHunt = 0.5;

	// Distortion
	float _distortionStrength = 0.0f;
	float _distortionAmplifier = 1.0f;
	float _distortionHuntRate = 0.75f;
	float _maxDistortion = 40.0f;

	// Movement
	float _baseMoveSpeed = 4.0f;
	float _baseHuntSpeed = 5.0f;
	float _moveSpeed = 0.0f;
	float _movementFactor = 1.0f;
	float _mineMovementFactor = 1.5f;
	float _caveMovementFactor = 0.8f;
	bool _nextNodeIsCave = false;
	bool _priorNodeWasCave = false;

	bool _isResetting = false;

	float _respawnTime = 1.5f;
	float _respawnTimer = 0.0f;
	DirectX::XMFLOAT3A _spawnPosition = { 0, 0, 0 };
	std::vector<DirectX::XMFLOAT3A> _spawnPositions;

	bool _sawPlayerHide = false; // TODO: Implement this

	MonsterStatus _state = MonsterStatus::IDLE;
	MonsterState *_monsterStates[MONSTER_STATE_COUNT] { nullptr };

	MonsterSound _currentSound = MonsterSound::HUNTING1;


	MeshBehaviour *_meshBehaviour = nullptr;

	SoundBehaviour *_soundBehaviours[SOUND_COUNT] { nullptr };
	float _soundVolumes[SOUND_COUNT] { 0 };
	float _soundVolumeIncreaseRate = 0.1f; // Should be < 1
	float _currentSoundVolume;

	Entity *_playerEntity = nullptr;
	PlayerMovementBehaviour *_playerMover = nullptr;
	FlashlightBehaviour *_playerFlashlight = nullptr;

	GraphManager *_graphManager = nullptr;

	bool _drawRadius = false;
	std::vector<DirectX::XMFLOAT3> _path;

	bool AnyStateUpdate(Time &time);

	void MoveToPoint(Time &time, const DirectX::XMFLOAT3A &point);
	float CalculateDistance(const DirectX::XMFLOAT3A &point1, const DirectX::XMFLOAT3A &point2);
	bool HasLOS(DirectX::XMFLOAT3A &point);

	bool SetState(const MonsterStatus& newState);

	bool DecayAlert(const float& deltaTime);
	bool GrowAlert(const float &deltaTime);

	void GetRandomizedNodePos(DirectX::XMFLOAT3A &pos, bool mineOnly=false); // Currently unused

	void UpdatePathToPlayer();
	void UpdatePathIdle();
	void PathFind(Time &time);

protected:
	[[nodiscard]] bool Start() override;
	[[nodiscard]] bool Update(Time& time, const Input& input) override;
	[[nodiscard]] bool FixedUpdate(const float& deltaTime, const Input& input) override;
#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI() override;
#endif

public:
	MonsterBehaviour() = default;
	~MonsterBehaviour();

	bool Alert(const float amount, std::optional<DirectX::XMFLOAT3A> soundOrigin = std::nullopt,
		std::optional<const float> distance = std::nullopt, std::optional<bool> scaleWithDist = std::nullopt, 
		std::optional<bool> requireLOS = std::nullopt);

	bool soundAlert(const float amount, std::optional<DirectX::XMFLOAT3A> soundOrigin = std::nullopt,
		std::optional<const float> distance = std::nullopt, std::optional<bool> scaleWithDist = std::nullopt);

	bool GetPath(DirectX::XMFLOAT3 &dest);
	bool GetPath(std::vector<DirectX::XMFLOAT3> *&path);

	void ResetMonster();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};;