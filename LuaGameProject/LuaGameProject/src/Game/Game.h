#pragma once
#include "box2d/box2D.h"
#include "lua.hpp"

namespace Game
{
	enum SceneState
	{
		None,
		InMenu,
		InGame,
		InEditor,
		ReloadGame,
		ReloadEditor,
		Quitting,

		Count
	};

	static bool IsQuitting = false;

	class Game
	{
	public:
		float TimeScale = 1.0f;

	#ifdef LUA_DEBUG
		bool CmdStepMode = false;
		int CmdTakeSteps = 0;
	#endif

		static Game &Instance()
		{
			static Game instance;
			return instance;
		}

	private:

	};
}
