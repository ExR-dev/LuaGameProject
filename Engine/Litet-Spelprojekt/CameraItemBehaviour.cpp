#include "stdafx.h"
#include "CameraItemBehaviour.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;

bool CameraItemBehaviour::Start()
{
	if (_name == "")
		_name = "CameraItemBehaviour"; // For categorization in ImGui.

	Scene *scene = GetScene();
	SceneHolder *sceneHolder = scene->GetSceneHolder();
	Content *content = scene->GetContent();

	// Create camera flash entity
	Entity *ent = nullptr;
	if (!scene->CreateEntity(&ent, "Camera Flash", { {0,0,0},{.1f,.1f,.1f},{0,0,0,1} }, false))
	{
		ErrMsg("Failed to create Camera Flash entity!");
		return false;
	}
	ent->GetTransform()->SetParent(GetTransform());
	ent->GetTransform()->SetPosition({ 0.150064f, 0.217686f, 0.259081f }, Local);

	ProjectionInfo projInfo = ProjectionInfo(120.0f * DEG_TO_RAD, 1.0f, { 0.01f, 100.0f });
	_flashLightBehaviour = new SpotLightBehaviour(projInfo, { 200, 200, 200 }, 1.0f);

	if (!_flashLightBehaviour->Initialize(ent))
	{
		ErrMsg("Failed to bind light behaviour to camera flash!");
		return false;
	}

	ent->SetSerialization(false);
	_flashLightBehaviour->SetSerialization(false);
	_flashLightBehaviour->SetEnabled(false);

	_flashSoundBehaviour = new SoundBehaviour("Camera Flash");
	if (!_flashSoundBehaviour->Initialize(GetEntity()))
	{
		ErrMsg("Failed to Initialize sound behaviour!");
		return false;
	}
	_flashSoundBehaviour->SetEnabled(false);
	_flashSoundBehaviour->SetSerialization(false);

	return true;
}

bool CameraItemBehaviour::Update(Time &time, const Input &input)
{
	if (_cooldownTimer > 0.0f)
		_cooldownTimer -= time.deltaTime;

	if (!_firstFrameOfFlash && _flashTimer > 0.0f)
	{
		_flashTimer -= time.deltaTime;
		if (_flashTimer <= 0.0f)
			_flashLightBehaviour->SetEnabled(false);
	}
	_firstFrameOfFlash = false;

	return true;
}

#ifdef USE_IMGUI
bool CameraItemBehaviour::RenderUI()
{
	if (ImGui::Button("Take Picture"))
		TakePicture(0.03f);
	return true;
}
#endif

bool CameraItemBehaviour::OnSelect()
{
	TakePicture(0.03f);
	return true;
}

void CameraItemBehaviour::TakePicture(float flashTime)
{
	if (_cooldownTimer > 0.0f)
		return;

	_flashTimer = flashTime;
	_cooldownTimer = 0.5f;
	_firstFrameOfFlash = true;
	_flashLightBehaviour->SetEnabled(true);
	_flashSoundBehaviour->SetEnabled(true);
}

bool CameraItemBehaviour::Serialize(std::string *code) const
{
	// Standard code for Serialize
	*code += "CameraItemBehaviour( )";
		//+ std::to_string(_isOn) + " " + std::to_string(_battery) + " " + std::to_string(_isDead) + " " + std::to_string(_isCharging) +
		//" )";

	return true;
}

bool CameraItemBehaviour::Deserialize(const std::string &code)
{
	// Standard code for all behaviours deserialize
	//std::vector<float> attributes;
	//std::istringstream stream(code);

	//std::string value;
	//while (stream >> value) // Automatically handles spaces correctly
	//{
	//	float attribute = std::stof(value);
	//	attributes.push_back(attribute);
	//}

	return true;
}
