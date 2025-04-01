#include "stdafx.h"
#include "CreditsBehaviour.h"
#include "Scene.h"
#include "Game.h"
#include "SoundBehaviour.h"

bool CreditsBehaviour::ChangeScreenTexture()
{
	_game->GetGraphics()->BeginScreenFade(-_fadeTime);

	Material mat = Material(*_creditsMesh->GetMaterial());
	mat.textureID = _slides.at(_screenIndex).textureID;
	if (!_creditsMesh->SetMaterial(&mat))
	{
		ErrMsg("Failed to set credits material!");
		return false;
	}

	if (_slides.at(_screenIndex).textureID == GetScene()->GetContent()->GetTextureID("Tex_Credit_Special_Texture"))
	{
		GetScene()->GetSceneHolder()->GetEntityByName("Maxwell")->Enable();
	}

	_screenIndex++;

	return true;
}

bool CreditsBehaviour::Start()
{
	if (_name == "")
		_name = "CreditsBehaviour"; // For categorization in ImGui.

	Scene *scene = GetScene();

	Entity* ent = scene->GetSceneHolder()->GetEntityByName("Credits Mesh");
	if (!ent->GetBehaviourByType<MeshBehaviour>(_creditsMesh))
		return false;

	_game = scene->GetGame();

	// Create main camera
	{
		ProjectionInfo projInfo = ProjectionInfo(65.0f * DEG_TO_RAD, 16.0f / 9.0f, { 0.2f, 100.0f });
		CameraBehaviour *camera = new CameraBehaviour(projInfo);

		if (!camera->Initialize(GetEntity()))
		{
			ErrMsg("Failed to bind MainCamera behaviour!");
			return false;
		}

		camera->SetSerialization(false);

		scene->SetViewCamera(camera);
	}

	Content *content = scene->GetContent();
	// 2.64
	Slide slide;
	slide.textureID = content->GetTextureID("Tex_Credit_Logo_Texture");
	slide.showLength = 12.0f; // 12.6
	_slides.push_back(slide);

	slide.textureID = content->GetTextureID("Tex_Credit_Devs_Texture");
	slide.showLength = 12.0f; // 25.2
	_slides.push_back(slide);

	slide.textureID = content->GetTextureID("Tex_Credit_Assets_Texture");
	slide.showLength = 17.0f; // 42.8
	_slides.push_back(slide);

	slide.textureID = content->GetTextureID("Tex_Credit_Testing_Texture");
	slide.showLength = 12.0f; // 55.4
	_slides.push_back(slide);

	slide.textureID = content->GetTextureID("Tex_Credit_BTH_Texture");
	slide.showLength = 10.0f; // 66.5
	_slides.push_back(slide);

	slide.textureID = content->GetTextureID("Tex_Credit_Story1_Texture");
	slide.showLength = 17.0f; // 83.7
	_slides.push_back(slide);

	slide.textureID = content->GetTextureID("Tex_Credit_Special_Texture");
	slide.showLength = 12.0f; // 96
	_slides.push_back(slide);

	SoundBehaviour *song = new SoundBehaviour("Credits", (DirectX::SOUND_EFFECT_INSTANCE_FLAGS)0U);
	if (!song->Initialize(GetEntity()))
	{
		ErrMsg("Failed to initialize sound!");
		return false;
	}
	song->SetSerialization(false);
	song->SetVolume(0.3f);
	song->SetEnabled(false);
	_song = song;

	return true;
}

bool CreditsBehaviour::Update(Time &time, const Input &input)
{
	_elapsed += time.deltaTime;
	static bool hasFadedOut = false;

	if (BindingCollection::IsTriggered(InputBindings::InputAction::Pause))
	{
		_screenIndex = 0;
		_elapsed = 0.0f;
		hasFadedOut = false;
		_game->GetGraphics()->BeginScreenFade(0.5f);
		GetScene()->GetSceneHolder()->GetEntityByName("Maxwell")->Disable();

		std::vector<SoundBehaviour *> sounds;
		GetScene()->GetGame()->GetScene(MENU_SCENE)->GetSceneHolder()->GetEntityByName("StartButton")->GetBehavioursByType<SoundBehaviour>(sounds);
		sounds.at(0)->ResetSound();
		sounds.at(0)->Play();

		SetEnabled(false);
		return true;
	}

	float showTime = _slides.at(_screenIndex == _slides.size() ? _slides.size() - 1 : _screenIndex).showLength;
	if (!hasFadedOut && _elapsed >= showTime && _elapsed <= showTime + _fadeTime) // Fade out
	{
		_game->GetGraphics()->BeginScreenFade(_fadeTime);
		hasFadedOut = true;
	}
	else if (hasFadedOut && _elapsed >= showTime + _fadeTime) // Switch credit screen texture / scene
	{
		if (_screenIndex >= _slides.size()) // Credit scene finished
		{
			Scene *currentScene = GetScene();
			currentScene->GetSceneHolder()->GetEntityByName("Maxwell")->Disable();

			if (!_game->SetScene(MENU_SCENE))
			{
				ErrMsg("Failed to set scene!");
				return false;
			}

			// Reset game

			std::ofstream file(std::format("Content/Saves/{}.txt", "GameSave"), std::ios::out);
			if (!file)
			{
				ErrMsg("Could not clear save file!");
				return false;
			}

			std::string code = "";

			file << code;
			file.close();

			Scene *gameScene = currentScene->GetGame()->GetScene(GAME_SCENE);
			gameScene->ResetScene();
			if (!gameScene->InitializeGame(currentScene->GetDevice(), currentScene->GetContext(), currentScene->GetGame(), currentScene->GetContent(),
				currentScene->GetGraphics(), 10.0f, "MapSave"))
			{
				ErrMsg("Failed to reset game scene!");
				return false;
			}

			std::vector<SoundBehaviour *> sounds;
			currentScene->GetGame()->GetScene(MENU_SCENE)->GetSceneHolder()->GetEntityByName("StartButton")->GetBehavioursByType<SoundBehaviour>(sounds);
			sounds.at(0)->ResetSound();
			sounds.at(0)->Play();

			_game->GetGraphics()->BeginScreenFade(-_fadeTime);
			SetEnabled(false);
			_screenIndex = 0;
		}
		else if (!ChangeScreenTexture()) // Next slide
		{
			return false;
		}
		hasFadedOut = false;
		_elapsed = 0.0f;
	}

	return true;
}

void CreditsBehaviour::OnEnable()
{
	_song->SetEnabled(true);
	if (ChangeScreenTexture())
		return;
}
