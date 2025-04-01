#include "stdafx.h"
#include "SoundSource.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool SoundSource::Initialize(DirectX::AudioEngine *audEngine, 
	DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags, std::string fileName, 
	float distanceScaler, float reverbScaler)
{
	fileName = "Content\\Sounds\\" + fileName + ".wav";
	struct stat buffer;   
	if (stat(fileName.c_str(), &buffer) != 0)
	{
		ErrMsg("Failed to load " + fileName + "! File does not exist.");
		return false;
	}

	std::wstring temp = std::wstring(fileName.begin(), fileName.end());
	_sound = std::make_unique<DirectX::SoundEffect>(audEngine, temp.c_str());
	_soundEffectFlag = flags;
	_effect = _sound->CreateInstance(flags);
	_emitter.ChannelCount = _sound->GetFormat()->nChannels;

	_listener.SetOrientation(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));


	X3DAUDIO_CONE listenerCone = {};

	// Default
	{
		listenerCone.InnerAngle = 0;
		listenerCone.OuterAngle = 0;

		listenerCone.InnerVolume = 1.0f / distanceScaler;
		listenerCone.OuterVolume = 1.0f / distanceScaler;

		listenerCone.InnerLPF = 0.0f;
		listenerCone.OuterLPF = 0.25f;

		listenerCone.InnerReverb = 1.0f * reverbScaler;
		listenerCone.OuterReverb = 0.1f * reverbScaler;
	}

	// DirectX example
	/*{
		listenerCone.InnerAngle = AudioPresets::ListenerCone.InnerAngle;
		listenerCone.OuterAngle = AudioPresets::ListenerCone.OuterAngle;

		listenerCone.InnerVolume = AudioPresets::ListenerCone.InnerVolume / distanceScaler;
		listenerCone.OuterVolume = AudioPresets::ListenerCone.OuterVolume / distanceScaler;

		listenerCone.InnerLPF = AudioPresets::ListenerCone.InnerLPF;
		listenerCone.OuterLPF = AudioPresets::ListenerCone.OuterLPF;

		listenerCone.InnerReverb = AudioPresets::ListenerCone.InnerReverb * reverbScaler;
		listenerCone.OuterReverb = AudioPresets::ListenerCone.OuterReverb * reverbScaler;
	}*/

	_listener.SetCone(listenerCone);
	
	// Default
	{
		_emitter.CurveDistanceScaler = distanceScaler;
		_emitter.pVolumeCurve = const_cast<X3DAUDIO_DISTANCE_CURVE *>(&AudioPresets::CustomCurve);
		_emitter.pLFECurve = const_cast<X3DAUDIO_DISTANCE_CURVE *>(&AudioPresets::LFECurve);
		_emitter.pReverbCurve = const_cast<X3DAUDIO_DISTANCE_CURVE *>(&AudioPresets::ReverbCurve);
	}

	// DirectX example
	/*{
		_emitter.pCone = const_cast<X3DAUDIO_CONE*>(&AudioPresets::EmitterCone);
	}*/

	if (!_effect)
	{
		ErrMsg("Failed to load sound effect " + fileName + "!");
		return false;
	}

	return true;
}

void SoundSource::PlayAudio()
{
	_effect->Play(true);
	if (_soundEffectFlag == (DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters))
		_effect->Apply3D(_listener, _emitter, false);
}

void SoundSource::AdjustVolume(float newVolume)
{
	_effect->SetVolume(newVolume);
}

void SoundSource::PauseAudio()
{
	_effect->Pause();
}

void SoundSource::ResumeAudio()
{
	_effect->Resume();
}

size_t SoundSource::GetSoundLength()
{
	return _sound->GetSampleDurationMS();
}

DirectX::SoundState SoundSource::GetSoundState()
{
	return _effect->GetState();
}

void SoundSource::SetListenerPosition(DirectX::XMFLOAT3 position)
{
	_listener.SetPosition(position);
}

void SoundSource::SetListenerOrientation(DirectX::XMFLOAT3 forwardVec, DirectX::XMFLOAT3 upVec)
{
	_listener.SetOrientation(forwardVec, upVec);
}

void SoundSource::SetEmitterPosition(DirectX::XMFLOAT3 position)
{
	_emitter.SetPosition(position);
}

void SoundSource::ResetSound()
{
	_effect.reset();
	_effect = _sound->CreateInstance(_soundEffectFlag);
}
