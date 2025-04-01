#include "stdafx.h"
#include "AmbientSoundBehaviour.h"
#include "Scene.h"
#include "GameMath.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool AmbientSoundBehaviour::Start()
{
    if (_name == "")
		_name = "AmbientSoundBehaviour";

	_soundBehaviour = new SoundBehaviour(_fileName, _soundEffectFlag, _loop, _distanceScaler, _reverbScaler);
	if (!_soundBehaviour->Initialize(GetEntity()))
	{
		ErrMsg("Failed to Initialize sound behaviour!");
		return false;
	}
	_soundBehaviour->SetVolume(_volume);
	_soundBehaviour->SetSerialization(false);
	_soundBehaviour->SetEnabled(false);
	_soundBehaviour->SetEmitterPosition(GetEntity()->GetTransform()->GetPosition(World));

	_timer = RandomFloat(_delayMin, _delayMax);

#ifdef DEBUG_BUILD
	Material mat;
	mat.textureID = GetScene()->GetContent()->GetTextureID("Tex_SoundEmitter");
	mat.ambientID = GetScene()->GetContent()->GetTextureID("Tex_White");

	_billboardMeshBehaviour = new BillboardMeshBehaviour(mat, 0.0f, 0.0f, 1.0f, true, false, false, false);
	if (!_billboardMeshBehaviour->Initialize(GetEntity()))
	{
		ErrMsg("Failed to Initialize billboard mesh behaviour!");
		return false;
	}
	_billboardMeshBehaviour->SetSerialization(false);
	_billboardMeshBehaviour->SetEnabled(false);
#endif

    return true;
}

bool AmbientSoundBehaviour::Update(Time& time, const Input& input)
{
	if (!_soundBehaviour->IsEnabled())
	{
		if (_timer > 0)
		{
			_timer -= time.deltaTime;
		}
		else
		{
			_soundBehaviour->SetEnabled(true);
			_timer = RandomFloat(_delayMin, _delayMax);
		}
	}
	else
	{
		_soundBehaviour->Play();
	}

	return true;
}

#ifdef USE_IMGUI
bool AmbientSoundBehaviour::RenderUI()
{
	ImGui::Text("Play countdown: %.1f", _timer);
	if (ImGui::Button("Play", ImVec2(50, 20)))
	{
		TriggerSound();
	}

	const float pointOne = 0.1f;
	float vol = _volume;
	ImGui::InputScalar("Volume", ImGuiDataType_Float, &vol, &pointOne);
	if (vol != _volume)
	{
		_volume = round(vol * 1000) / 1000;
		_soundBehaviour->SetVolume(_volume);
	}

	ImGui::Checkbox("Loop", &_loop);
	ImGui::DragFloat("Min Delay", &_delayMin, 0.01f);
	ImGui::DragFloat("Max Delay", &_delayMax, 0.01f);
	ImGui::DragFloat("Distance scaler", &_distanceScaler, 0.01f);
	ImGui::DragFloat("Reverb scaler", &_reverbScaler, 0.01f);

	_delayMin = _delayMin < 0.0f ? 0.0f : round(_delayMin * 1000) / 1000;
	_delayMax = _delayMax < _delayMin ? _delayMin : round(_delayMax * 1000) / 1000;
	_distanceScaler = _distanceScaler < 0.0f ? 0.0f : round(_distanceScaler * 1000) / 1000;
	_reverbScaler = _reverbScaler < 0.0f ? 0.0f : round(_reverbScaler * 1000) / 1000;


	{
		static std::string soundName = "";
		static bool foundFile = false;

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
			const char* filterPatterns[] = { "*.wav" };
			const char* selectedFiles = tinyfd_openFileDialog(
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

			if (foundFile)
			{
				_fileName = soundName;
				_soundBehaviour = new SoundBehaviour(_fileName, _soundEffectFlag, _loop, _distanceScaler, _reverbScaler);
				if (!_soundBehaviour->Initialize(GetEntity()))
				{
					ErrMsg("Failed to Initialize sound behaviour after changing source!");
					return false;
				}
			}
			else
			{
				ErrMsg("Failed to load " + soundName + "! File does not exist.");
				ImGui::SetItemTooltip("File could not be found.");
			}
		}
	}

	return true;
}
#endif

AmbientSoundBehaviour::AmbientSoundBehaviour(std::string fileName, DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags, bool loop, float volume,
	float distanceScaler, float reverbScaler, float minimumDelay, float maximumDelay)
{
	_fileName = fileName;
	_soundEffectFlag = DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters;
	_loop = loop;
	_volume = volume;
	_distanceScaler = distanceScaler;
	_reverbScaler = reverbScaler;
	_delayMin = minimumDelay;
	_delayMax = maximumDelay;
}

void AmbientSoundBehaviour::TriggerSound()
{
	if (!_soundBehaviour->IsEnabled())
	{
		_soundBehaviour->SetEnabled(true);
		_timer = RandomFloat(_delayMin, _delayMax);
		_soundBehaviour->Play();
	}
}

bool AmbientSoundBehaviour::Serialize(std::string* code) const
{
	std::string fileName = _fileName;
	std::replace(fileName.begin(), fileName.end(), ' ', '_');

	*code += _name + "("
		+ fileName + " " + std::to_string(_soundEffectFlag) + " " + std::to_string(_volume) + " "
		+ std::to_string(_loop) + " " + std::to_string(_distanceScaler) + " " + std::to_string(_reverbScaler) + " "
		+ std::to_string(_delayMin) + " " + std::to_string(_delayMax) +
		" )";
	return true;
}

bool AmbientSoundBehaviour::Deserialize(const std::string& code)
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
	_distanceScaler = attributes.at(2);
	_reverbScaler = attributes.at(3);
	_delayMin = attributes.at(4);
	_delayMax = attributes.at(5);

	return true;
}
