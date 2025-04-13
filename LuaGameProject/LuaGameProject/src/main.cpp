#include "stdafx.h"
#include "LuaConsole.h"
#include "Game/main2D.h"
#include "Game/main3D.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
	srand(time(NULL));

	std::cout << "Input game type (2/3): ";
	std::string gameType;
	std::cin >> gameType;

	if (gameType == "3")
	{
		std::cout << "Running 3D game..." << std::endl;
		Main3D::Run();
	}
	else if (gameType == "2")
	{
		std::cout << "Running 2D game..." << std::endl;
		Main2D::Run();
	}

    return 1;
}
