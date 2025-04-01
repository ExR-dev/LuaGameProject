#include "stdafx.h"
#include "MonsterHintBehaviour.h"
#include "GameMath.h"
#include "Scene.h"

DirectX::XMFLOAT3A VecToPointXZ(const DirectX::XMFLOAT3A& from, const DirectX::XMFLOAT3A& to)
{
	// Returns a XMFLOAT3A representing the vector starting at 'from' and ending at 'to' projected onto the XZ plane.
	// The Y component of the returned vector is equal to that of 'from'.
	return { to.x - from.x, from.y, to.z - from.z };
}

float Magnitude(const DirectX::XMFLOAT3A& vec)
{
	DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
	return DirectX::XMVectorGetX(DirectX::XMVector3Length(v));
}

DirectX::XMFLOAT3A Normalize(const DirectX::XMFLOAT3A& vec)
{
	DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&vec);
	v = DirectX::XMVector3Normalize(v);
	DirectX::XMFLOAT3A result;
	DirectX::XMStoreFloat3(&result, v);
	return result;
}

bool MonsterHintBehaviour::Start()
{
	if (_name == "")
		_name = "MonsterHintBehaviour";

    std::vector<std::string> fileNames = { "monster_hint_02", "DistantThud", "Echo_Cymbal_01", "Echo_Cymbal_02", "Echo_Cymbal_03", "monster_hint_01" };
	for (const auto& fileName : fileNames)
	{
		_sounds.push_back(new SoundBehaviour(fileName, DirectX::SoundEffectInstance_Use3D | DirectX::SoundEffectInstance_ReverbUseFilters, false, 400.0f, 0.0f));

		Entity* emitterEntity = nullptr;
		if (!GetScene()->CreateEntity(&emitterEntity, "Hint Emitter Entity", { {0,0,0}, {0.1f,0.1f,0.1f}, {0,0,0,1} }, false))
		{
			ErrMsg("Failed to initialize player!");
			return false;
		}

		if (!_sounds.back()->Initialize(emitterEntity))
		{
			ErrMsg("Failed to initialize sound!");
			return false;
		}
		_sounds.back()->SetSerialization(false);
		_sounds.back()->SetVolume(0.7f);
		_sounds.back()->SetEnabled(false);
	}

	_timer = RandomFloat(_minDelay, _maxDelay);

    return true;
}

bool MonsterHintBehaviour::Update(Time& time, const Input& input)
{
	if (_timer > 0)
	{
		_timer -= time.deltaTime;
	}
	else
	{
		// Only get the monster behaviour once
		if (!_monsterBehaviour)
		{
			_monsterBehaviour = GetScene()->GetMonster();
			if (!_monsterBehaviour)
			{
				ErrMsg("Failed to get monster behaviour!");
				return false;
			}
		}

		// Get a normalized vector between the player and monster
		DirectX::XMFLOAT3A playerPos = GetTransform()->GetPosition(World);
		DirectX::XMFLOAT3A monsterPos = _monsterBehaviour->GetTransform()->GetPosition(World);
		DirectX::XMFLOAT3A direction = VecToPointXZ(playerPos, monsterPos);

		// Get the distance between the player and monster
		float distance = Magnitude(direction);
		
		// Set the emitter position to 50 units away from the player, in the direction of the monster.
		if (distance > _minDistance)
		{
			direction = Normalize(direction);
			direction = DirectX::XMFLOAT3A{ direction.x * _minDistance, direction.y, direction.z * _minDistance };
			DirectX::XMFLOAT3A newEmitterPosition = DirectX::XMFLOAT3A{ playerPos.x + direction.x, playerPos.y + direction.y + 1.0f, playerPos.z + direction.z };



			int soundIndex = rand() % _sounds.size();

			// Play the sound
			_sounds.at(soundIndex)->GetTransform()->SetPosition(newEmitterPosition, World);
			_sounds.at(soundIndex)->SetEmitterPosition(newEmitterPosition);
			_sounds.at(soundIndex)->SetEnabled(true);

			// Reset the timer
			_timer = RandomFloat(_minDelay, _maxDelay);
		}

	}

	for (auto sound : _sounds)
	{
#ifdef USE_IMGUI
		if(_drawEmitters)
		{
			DirectX::XMFLOAT3A pos = GetTransform()->GetPosition();
			pos.y += .5f;
			DirectX::XMFLOAT3 emitterPos = sound->GetEmitterPosition();
			GetScene()->GetDebugDrawer()->DrawLine({ pos, .1f, { 1.0f, 0.0f, 0.0f, 1 } }, { emitterPos, .1f, { 0.0f, 0.0f, 1.0f, 1 } });
		}
#endif

		if (sound->IsEnabled())
		{
			sound->Play();
		}
	}
	return true;
}

#ifdef USE_IMGUI
bool MonsterHintBehaviour::RenderUI()
{
	ImGui::Checkbox("Draw emitter positions", &_drawEmitters);

	return true;
}
#endif
