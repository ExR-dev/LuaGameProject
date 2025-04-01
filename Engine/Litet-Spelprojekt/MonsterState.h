#pragma once

#include "Time.h"
#include "MonsterBehaviour.h"

class MonsterState
{
public:
	MonsterState() = default;
	virtual ~MonsterState() = default;

	void Intitalize(MonsterBehaviour *monsterBehaviour);

	bool InitialOnEnter();
	bool InitialOnUpdate(Time &time);
	bool InitialOnExit();

protected:
	virtual bool OnEnter();
	virtual bool OnUpdate(Time &time);
	virtual bool OnExit();

	MonsterBehaviour *_mb = nullptr;
};
