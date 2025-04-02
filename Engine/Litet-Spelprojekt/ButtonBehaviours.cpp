#include "stdafx.h"
#include "ButtonBehaviours.h"
#include "Game.h"
#include "Scene.h"
#include "Input.h"
#include "CreditsBehaviour.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

#define HOVER_EFFECT DirectX::XMFLOAT3A({ 0.7f / 0.7f, 0.2f / 0.7f, 0.2f / 0.7f })

bool PlayButtonBehaviour::SwitchScene()
{
	_isSwitching = false;
	_timedSceneSwitch = 0.0f;

	Scene *scene = GetScene();
	Game *game = scene->GetGame();
	if (_playCutscene)
	{
		if (!game->SetScene(START_CUTSCENE))
		{
			ErrMsg("Failed to set scene!");
			return false;
		}
		_playCutscene = false;
	}
	else
	{
		if (!game->SetScene(GAME_SCENE))
		{
			ErrMsg("Failed to set scene!");
			return false;
		}
	}


	Window* window = game->GetWindow();
	Input::GetInstance()->ToggleLockCursor(window);

	FogSettingsBuffer fogSettings;
	game->GetGraphics()->SetFogSettings(fogSettings);
	game->GetGraphics()->BeginScreenFade(-1.0f);

	_reset = true;
	_time = 0.0f;
	_song->SetEnabled(false);

	return true;
}

bool PlayButtonBehaviour::Start()
{
	if (_name == "")
		_name = "PlayButtonBehaviour"; // For categorization in ImGui.

	_t = GetEntity()->GetTransform();

	SoundBehaviour *song = new SoundBehaviour("LurksBelowThemeSong", DirectX::SoundEffectInstance_Default);
	if (!song->Initialize(GetEntity()))
	{
		ErrMsg("Failed to initialize theme song!");
		return false;
	}
	song->SetVolume(0.1f);
	song->SetSerialization(false);
	_soundLength = song->GetSoundLength();
	_song = song;

	SoundBehaviour *buttonSound = new SoundBehaviour("footstep_concrete_000", DirectX::SoundEffectInstance_Default);
	if (!buttonSound->Initialize(GetEntity()))
	{
		ErrMsg("Failed to initialize theme song!");
		return false;
	}
	buttonSound->SetVolume(0.3f);
	buttonSound->SetEnabled(false);
	_buttonSound = buttonSound;

	return true;
}
bool PlayButtonBehaviour::Update(Time &time, const Input &input)
{
	GetScene()->GetGraphics()->SetDistortionStrength(0.0f);

	if (_time >= _timeToStart && _reset)
	{
		_song->SetEnabled(true);
		_song->ResetSound();
		_song->Play();
		_reset = false;
	}
	
	if (BindingCollection::IsTriggered(InputBindings::InputAction::Pause))
	{
		_reset = true;
		_time = 0.0f;
		_song->SetEnabled(false);
	}

	if (_time >= _soundLength)
	{
		_time = 0.0f;
		_reset = true;
	}

	_time += time.deltaTime;

	if (_isSwitching)
	{
		_timedSceneSwitch -= time.deltaTime;
		if (_timedSceneSwitch <= 0.0f)
		{
			if (!SwitchScene())
			{
				ErrMsg("Failed to switch scene!");
				return false;
			}
		}
	}

	return true;
}
bool PlayButtonBehaviour::OnSelect()
{
	_buttonSound->SetEnabled(true);

	_isSwitching = true;
	_timedSceneSwitch = 1.0f;

	Game *game = GetScene()->GetGame();
	game->GetGraphics()->BeginScreenFade(_timedSceneSwitch);
	return true;
}
bool PlayButtonBehaviour::OnHover()
{
	_t->SetScale(HOVER_EFFECT);
	return true;
}

void PlayButtonBehaviour::PlayButtonSound() const
{
	_buttonSound->SetEnabled(true);
}

#ifdef USE_IMGUI
bool PlayButtonBehaviour::RenderUI()
{
	std::string lengthText = "Length: " + std::to_string(_time) + "/" + std::to_string(_soundLength);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), lengthText.c_str());

	std::string durationText = "Duration: " + std::to_string(_time) + "/" + std::to_string(_timeToStart);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), durationText.c_str());

	return true;
}
#endif

// ---------------------------------------------------------------------

bool SaveButtonBehaviour::Start()
{
	if (_name == "")
		_name = "SaveButtonBehaviour"; // For categorization in ImGui.

	_t = GetEntity()->GetTransform();

	return true;
}
bool SaveButtonBehaviour::OnSelect()
{
	PlayButtonBehaviour *start = nullptr;
	GetScene()->GetSceneHolder()->GetEntityByName("StartButton")->GetBehaviourByType<PlayButtonBehaviour>(start);
	start->PlayButtonSound();

	Scene *scenes = GetScene()->GetGame()->GetScene(GAME_SCENE);
	std::string code;
	if (!scenes->Serialize(&code))
	{
		ErrMsg("Failed to serialize scene!");
		return false;
	}

	return true;
}
bool SaveButtonBehaviour::OnHover()
{
	_t->SetScale(HOVER_EFFECT);
	return true;
}

// ---------------------------------------------------------------------

bool NewSaveButtonBehaviour::Start()
{
	if (_name == "")
		_name = "NewSaveButtonBehaviour"; // For categorization in ImGui.

	_t = GetEntity()->GetTransform();

	std::vector<SoundBehaviour *> sounds;
	if (GetScene()->GetSceneHolder()->GetEntityByName("StartButton")->GetBehavioursByType<SoundBehaviour>(sounds))
		_length = sounds.at(1)->GetSoundLength();

	return true;
}
bool NewSaveButtonBehaviour::Update(Time &time, const Input &input)
{
	if (_reset)
	{
		if (_time >= _length)
		{
			Scene *scene = GetScene()->GetGame()->GetScene(GAME_SCENE);

			std::ofstream file(std::format("Content/Saves/{}.txt", "GameSave"), std::ios::out);
			if (!file)
			{
				ErrMsg("Could not clear save file!");
				return false;
			}

			std::string code = "";

			file << code;
			file.close();

			Scene *currentScene = GetScene();
			scene->ResetScene();
			if (!scene->InitializeGame(currentScene->GetDevice(), currentScene->GetContext(), currentScene->GetGame(), currentScene->GetContent(),
				currentScene->GetGraphics(), 10.0f, "MapSave")) // Causes double sound
			{
				ErrMsg("Failed to reset game scene!");
				return false;
			}

			_reset = false;
			_time = 0.0f;
		}
		else
		{
			_time += time.deltaTime;
		}
	}
	return true;
}
bool NewSaveButtonBehaviour::OnSelect()
{
	PlayButtonBehaviour *start = nullptr;
	GetScene()->GetSceneHolder()->GetEntityByName("StartButton")->GetBehaviourByType<PlayButtonBehaviour>(start);
	start->PlayButtonSound();

	_reset = true;

	return true;
}
bool NewSaveButtonBehaviour::OnHover()
{
	_t->SetScale(HOVER_EFFECT);
	return true;
}

// ---------------------------------------------------------------------

bool CreditsButtonBehaviour::Start()
{
	if (_name == "")
		_name = "CreditsButtonBehaviour"; // For categorization in ImGui.

	_t = GetEntity()->GetTransform();

	return true;
}

bool CreditsButtonBehaviour::Update(Time &time, const Input &input)
{
	static float elapsed = 0.0f;
	if (_selected)
		elapsed += time.deltaTime;

	if (elapsed >= 1.0f)
	{
		if (!GetScene()->GetGame()->SetScene(CRED_SCENE))
		{
			ErrMsg("Failed to set credits scene!");
			return false;
		}

		CreditsBehaviour *credits = nullptr;
		GetScene()->GetGame()->GetScene(CRED_SCENE)->GetSceneHolder()->GetEntityByName("Credits Manager")->GetBehaviourByType<CreditsBehaviour>(credits);
		credits->SetEnabled(true);
		_selected = false;
		elapsed = 0.0f;
	}

	return true;
}

bool CreditsButtonBehaviour::OnSelect()
{
	_selected = true;
	GetScene()->GetGraphics()->BeginScreenFade(1.0f);

	return true;
}

bool CreditsButtonBehaviour::OnHover()
{
	_t->SetScale(HOVER_EFFECT);
	return true;
}

// ---------------------------------------------------------------------

bool ExitButtonBehaviour::Start()
{
	if (_name == "")
		_name = "ExitButtonBehaviour"; // For categorization in ImGui.

	_t = GetEntity()->GetTransform();

	return true;
}
bool ExitButtonBehaviour::OnSelect()
{
	ErrMsg("Game exited from exit button! (Not an error)");
	return false;
}
bool ExitButtonBehaviour::OnHover()
{
	_t->SetScale(HOVER_EFFECT);
	return true;
}
