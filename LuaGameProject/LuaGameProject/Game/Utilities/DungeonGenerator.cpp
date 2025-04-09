#include "../../stdafx.h"
#include "DungeonGenerator.h"
#include "Math.h"

DungeonGenerator::DungeonGenerator()
{
	for (int _ = 0; _ < 10; _++)
		AddRoom({ {0, 0}, {20, 20} });
}

void DungeonGenerator::AddRoom(const Room& room)
{
	_rooms.push_back(room);
}

void DungeonGenerator::Generate()
{
	//for (auto room : _rooms)
		//room.pos = Math::GetRandomPointInCircle(200);
}

void DungeonGenerator::Draw()
{
	for (auto room : _rooms)
		DrawRectangle(room.pos.x, room.pos.y, room.size.x, room.size.y, 
			{(unsigned char)(Math::Random01()*255), (unsigned char)(Math::Random01()*255), (unsigned char)(Math::Random01()*255), 255});
}

