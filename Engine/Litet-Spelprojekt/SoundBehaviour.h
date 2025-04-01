#pragma once

#include <string>
#include "Behaviour.h"
#include "Content.h"
#include "SoundSource.h"

/*
How to use:
Create a SoundBehaviour object in Scene::Initialize() and initialize the object with Behaviour::Initialize() passing an Entity as argument.
Right after, run SoundBehaviour SoundBehaviour::AddSoundSource() and eventually set its position with SoundBehaviour::GetEmitterPosition().
In Scene::UpdateSound(), create a SoundBehaviour pointer to point at the SoundBehaviour just created. 
To do this, create an Entity pointer and point it at the Entity just created by using SceneHolder::GetEntityByName(). 
From this Entity pointer, get the SoundBehaviour by using Entity::GetBehaviourByName() or Entity::GetEntityByID().
Then run SoundBehaviour::Play() from the SoundBehaviour pointer.
*/

class SoundBehaviour : public Behaviour
{
private:
	SoundSource _soundSource;
	std::string _fileName = "";
	DirectX::SOUND_EFFECT_INSTANCE_FLAGS _soundEffectFlag = DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters;

	DirectX::XMFLOAT3 _listenerPos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 _emitterPos = { 0.0f, 0.0f, 0.0f };

	float _volume = 1.0f;
	float _distanceScaler = 75.0f;
	float _reverbScaler = 1.0f;
	bool _loop = false;
	bool _play = true;

	float _length = 0.0f;
	float _duration = 0.0f;


protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

	// OnEnable runs immediately after the behaviour is enabled.
	void OnEnable() override;

	// OnEnable runs immediately after the behaviour is disabled.
	void OnDisable() override;

	// OnDirty runs when the Entity's transform is modified.
	void OnDirty() override;

	[[nodiscard]] bool OnSelect() override;
	[[nodiscard]] bool OnHover() override;

public:
	SoundBehaviour() = default;
	SoundBehaviour(std::string fileName, 
		DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags = DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters, 
		bool loop = false, float distanceScaler = 75.0f, float reverbScaler = 1.0f);
	~SoundBehaviour() = default;

	void Play();
	void Pause();

	DirectX::XMFLOAT3 GetListenerPosition() const;
	DirectX::XMFLOAT3 GetEmitterPosition() const;

	void SetListenerPosition(DirectX::XMFLOAT3 position);
	void SetEmitterPosition(DirectX::XMFLOAT3 position);

	[[nodiscard]] float GetSoundLength() const;

	void SetVolume(float volume);
	void SetSoundEffectFlag(DirectX::SOUND_EFFECT_INSTANCE_FLAGS flag);

	void ResetSound();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};

