#include "stdafx.h"
#include "SoundBehaviour.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

SoundBehaviour::SoundBehaviour(std::string fileName, DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags, 
	bool loop, float distanceScaler, float reverbScaler)
{
	_fileName = fileName;
	_soundEffectFlag = flags;
	_loop = loop;
	_distanceScaler = distanceScaler;
	_reverbScaler = reverbScaler;
}

void SoundBehaviour::Play()
{
	if (_play)
		_soundSource.PlayAudio();
}

void SoundBehaviour::Pause()
{
	_soundSource.PauseAudio();
}

DirectX::XMFLOAT3 SoundBehaviour::GetListenerPosition() const
{
	return _listenerPos;
}

DirectX::XMFLOAT3 SoundBehaviour::GetEmitterPosition() const
{
	return _emitterPos;
}

void SoundBehaviour::SetListenerPosition(DirectX::XMFLOAT3 position)
{
	_listenerPos = position;
	_soundSource.SetListenerPosition(_listenerPos);
}

void SoundBehaviour::SetEmitterPosition(DirectX::XMFLOAT3 position)
{
	_emitterPos = position;
	_soundSource.SetEmitterPosition(_emitterPos);
}

float SoundBehaviour::GetSoundLength() const
{
	return _length;
}

void SoundBehaviour::SetVolume(float volume)
{
	_volume = volume;
	_soundSource.AdjustVolume(_volume);
}

void SoundBehaviour::SetSoundEffectFlag(DirectX::SOUND_EFFECT_INSTANCE_FLAGS flag)
{
	_soundEffectFlag = flag;
}

void SoundBehaviour::ResetSound()
{
	_soundSource.ResetSound();
	_duration = 0.0f;
	SetVolume(_volume);
}

bool SoundBehaviour::Start()
{
	if (_name == "")
		_name = "SoundBehaviour"; // For categorization in ImGui.

	if (!_soundSource.Initialize(GetScene()->GetSoundEngine()->GetAudioEngine(), 
		_soundEffectFlag, _fileName, _distanceScaler, _reverbScaler))
	{
		ErrMsg("Failed to initialize sound source " + _fileName);
		return false;
	}

	_soundSource.SetListenerPosition(_listenerPos);
	_soundSource.SetEmitterPosition(_emitterPos);

	SetVolume(_volume);

	_length = _soundSource.GetSoundLength() / 1000.0f;

	return true;
}

bool SoundBehaviour::Update(Time &time, const Input &input)
{
	SetEmitterPosition(GetEntity()->GetTransform()->GetPosition(World));

	Transform *viewCamTransform = GetScene()->GetViewCamera()->GetTransform();
	DirectX::XMFLOAT3A listenerPos = viewCamTransform->GetPosition(World);
	DirectX::XMFLOAT3A forwardVec = viewCamTransform->GetForward(World);
	DirectX::XMFLOAT3A upVec = viewCamTransform->GetUp(World);
	_soundSource.SetListenerPosition(listenerPos);
	_soundSource.SetListenerOrientation(forwardVec, upVec);

	if (!_loop)
	{
		if (_duration >= _length - 0.05f) // Reached end of sound
		{
			_duration = 0.0f;
			ResetSound();
			SetEnabled(false);
		}
		else if (_soundSource.GetSoundState() == DirectX::SoundState::PLAYING)
		{
			_duration += time.deltaTime;
		}
	}

	return true;
}

#ifdef USE_IMGUI
bool SoundBehaviour::RenderUI()
{
	Scene *scene = GetScene();
	Entity *soundSource = GetEntity();
	DirectX::XMFLOAT3A emitterPos = soundSource->GetTransform()->GetPosition(World);

	Transform *viewCamTransform = scene->GetViewCamera()->GetTransform();
	DirectX::XMFLOAT3A listenerPos = viewCamTransform->GetPosition(World);

	std::string listenerText = "ListenerPos: " + std::to_string(listenerPos.x) + ", " + std::to_string(listenerPos.y) + ", " + std::to_string(listenerPos.z);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), listenerText.c_str());

	std::string emitterText = "EmitterPos: " + std::to_string(emitterPos.x) + ", " + std::to_string(emitterPos.y) + ", " + std::to_string(emitterPos.z);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), emitterText.c_str());

	std::string durationText = "Duration: " + std::to_string(_duration) + "/" + std::to_string(_length);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), durationText.c_str());

	std::string volumeText = "Volume: " + std::to_string(_volume);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), volumeText.c_str());

	return true;
}
#endif

void SoundBehaviour::OnEnable()
{
	_play = true;
	Play();
}

void SoundBehaviour::OnDisable()
{
	_play = false;
	Pause(); 
}

void SoundBehaviour::OnDirty()
{
}

bool SoundBehaviour::Serialize(std::string *code) const
{
	std::string fileName = _fileName;
	std::replace(fileName.begin(), fileName.end(), ' ', '_');

	*code += _name + "("
		+ fileName + " " + std::to_string(_soundEffectFlag) + " " + std::to_string(_volume) + " "
		+ std::to_string(_loop) + " " + std::to_string(_play)
		+ std::to_string(_distanceScaler) + " " + std::to_string(_reverbScaler)
		+ " )";
	return true;
}

bool SoundBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	std::vector<float> attributes;
	std::istringstream stream(code);

	std::string value;
	stream >> value;
	_fileName = value;

	stream >> value;
	_soundEffectFlag = (DirectX::SOUND_EFFECT_INSTANCE_FLAGS)std::stoul(value);

	while (stream >> value) // Automatically handles spaces correctly
	{
		float attribute = std::stof(value);
		attributes.push_back(attribute);
	}

	_volume = attributes.at(0);
	_loop = attributes.at(1);
	_play = attributes.at(2);
	_distanceScaler = attributes.at(3);
	_reverbScaler = attributes.at(4);

	return true;
}

bool SoundBehaviour::OnSelect()
{
	return true;
}

bool SoundBehaviour::OnHover()
{
	return true;
}
