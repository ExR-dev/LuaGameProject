#include "stdafx.h"
#include "MonsterCapture.h"
#include "Scene.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool MonsterCapture::OnEnter()
{
	_mb->GetEntity()->Disable();

	_mb->_playerMover->Catch();
	_mb->SetState(IDLE);

    return true;
}

bool MonsterCapture::OnUpdate(Time &time)
{
    return true;
}

bool MonsterCapture::OnExit()
{
	_mb->ResetMonster();

	return true;
}
