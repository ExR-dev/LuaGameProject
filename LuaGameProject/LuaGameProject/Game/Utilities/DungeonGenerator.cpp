#include "../../stdafx.h"
#include "DungeonGenerator.h"
#include "Math.h"

int Room::_ID = 0;

bool DungeonGenerator::Intersecting(const Room &r1, const Room &r2)
{
	return (r1.pos.x + r1.size.x / 2) > (r2.pos.x - r2.size.x / 2) &&
		   (r1.pos.x - r1.size.x / 2) < (r2.pos.x + r2.size.x / 2) &&
		   (r1.pos.y + r1.size.y / 2) > (r2.pos.y - r2.size.y / 2) &&
		   (r1.pos.y - r1.size.y / 2) < (r2.pos.y + r2.size.y / 2);
}

DungeonGenerator::DungeonGenerator(Vector2 pos):
	_position(pos)
{
	Initialize();
}

void DungeonGenerator::Initialize()
{
	if (_rooms.size() > 0)
		_rooms.clear();

	for (int _ = 0; _ < 10; _++)
		AddRoom({{(float)((int)Math::Random(2, 10)*_tileSize), (float)((int)Math::Random(2, 10)*_tileSize)}, 
				 {(unsigned char)(Math::Random01()*255), (unsigned char)(Math::Random01()*255), (unsigned char)(Math::Random01()*255), 255}});
}

void DungeonGenerator::AddRoom(const Room &room)
{
	_rooms.push_back(room);
}

void DungeonGenerator::Generate(float radius)
{
	// Set room positions
	for (auto &room : _rooms)
		room.pos = Vector2Add(_position, Math::RandomGridPointCircle(radius, _tileSize));
}

void DungeonGenerator::SeperateRooms()
{
	bool foundIntersection = true;

	//while (foundIntersection)
	{
		foundIntersection = false;
		for (auto &room : _rooms)
			for (auto &other : _rooms)
				if (room != other)
					if (Intersecting(room, other))
					{
						foundIntersection = true;

						// Seperate rooms
						Vector2 dir = Vector2Normalize(Vector2Subtract(room.pos, other.pos));
						room.pos = Vector2Add(room.pos, Vector2Scale(dir, _tileSize));
						other.pos = Vector2Subtract(other.pos, Vector2Scale(dir, _tileSize));
					}
	}
}

void DungeonGenerator::Draw()
{
	for (const auto &room : _rooms)
	{
		DrawRectangle(room.pos.x - room.size.x/2, room.pos.y - room.size.y/2, room.size.x, room.size.y, room.color);
		//DrawRectangle(room.pos.x, room.pos.y, room.size.x, room.size.y, room.color);
		DrawCircle(room.pos.x, room.pos.y, 3, { 255, 0, 0, 255 });
	}
}


Room::Room(raylib::Vector2 size, raylib::Color color) :
	_id(_ID++), pos({ 0, 0 }), size(size), color(color)
{
}
