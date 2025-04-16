#include "stdafx.h"
#include "LuaConsole.h"
#include "Game/main2D.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
	srand(time(NULL));

	Main2D::Run();
}
