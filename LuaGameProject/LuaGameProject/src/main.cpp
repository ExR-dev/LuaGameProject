#include "stdafx.h"
#include "LuaConsole.h"
#include "Game/main2D.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
	srand(time(NULL));

	// TODO: Add intermediary menu scene instead of launching main2D.

	Main2D::Main2D main2D;
	main2D.Run();
}
