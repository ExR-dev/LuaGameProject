#pragma once
#include "Behaviour.h"
#include "SoundBehaviour.h"
#include "BillboardMeshBehaviour.h"

class AmbientSoundBehaviour final : public Behaviour
{
private:
	SoundBehaviour* _soundBehaviour = nullptr;
#ifdef DEBUG_BUILD
	BillboardMeshBehaviour* _billboardMeshBehaviour = nullptr;
#endif
	
	std::string _fileName = "";
	float _volume = 0.5f;
	DirectX::SOUND_EFFECT_INSTANCE_FLAGS _soundEffectFlag = DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters;
	bool _loop = false;
	float _distanceScaler = 75.0f;
	float _reverbScaler = 1.0f;

	float _timer = 0.0f;

	float _delayMin = 2.0f;
	float _delayMax = 10.0f;

protected:
	[[nodiscard]] bool Start() override;

	[[nodiscard]] bool Update(Time& time, const Input& input) override;

#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI() override;
#endif

public:
	AmbientSoundBehaviour() = default;
	AmbientSoundBehaviour(std::string fileName,
		DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags = DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters,
		bool loop = false, float volume = 0.5f, float distanceScaler = 75.0f, float reverbScaler = 1.0f,
		float minimumDelay = 2.0f, float maximumDelay = 10.0f);
	~AmbientSoundBehaviour() = default;

	void TriggerSound();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;
};