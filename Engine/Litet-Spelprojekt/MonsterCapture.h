#pragma once

#include "MonsterState.h"

class MonsterCapture : public MonsterState
{
public:
	MonsterCapture() = default;
	~MonsterCapture() = default;

protected:
	bool OnEnter() override;
	bool OnUpdate(Time &time) override;
	bool OnExit() override;

private:
	Transform *_playerTransform;

	DirectX::XMFLOAT3A _playerPos;
	DirectX::XMFLOAT4A _playerRotation;
};
