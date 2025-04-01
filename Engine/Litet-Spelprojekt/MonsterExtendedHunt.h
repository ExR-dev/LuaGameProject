#pragma once

#include "MonsterState.h"

class MonsterExtendedHunt : public MonsterState
{
public:
	MonsterExtendedHunt() = default;
	~MonsterExtendedHunt() = default;

protected:
	bool OnEnter() override;
	bool OnUpdate(Time &time) override;
	bool OnExit() override;
};
