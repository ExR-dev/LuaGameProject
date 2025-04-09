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

	for (int _ = 0; _ < 100; _++)
		AddRoom({{(float)(Math::Random(2, 10)*_tileSize), (float)(Math::Random(2, 10)*_tileSize)}, 
				 {(unsigned char)(Math::Random01f()*255), (unsigned char)(Math::Random01f()*255), (unsigned char)(Math::Random01f()*255), 255}});
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

	while (foundIntersection)
	{
		foundIntersection = false;
		for (auto &room : _rooms)
		{
			for (auto &other : _rooms)
			{
				if (room != other && Intersecting(room, other))
				{
					foundIntersection = true;

					Vector2 delta = Vector2Subtract(room.pos, other.pos);

					// Decide axis of least penetration
					float overlapX = (room.size.x + other.size.x) / 2 - fabs(delta.x);
					float overlapY = (room.size.y + other.size.y) / 2 - fabs(delta.y);

					if (overlapX < overlapY)
					{
						float dir = (delta.x < 0) ? -1 : 1;
						room.pos.x += dir * _tileSize;
						other.pos.x -= dir * _tileSize;
					}
					else
					{
						float dir = (delta.y < 0) ? -1 : 1;
						room.pos.y += dir * _tileSize;
						other.pos.y -= dir * _tileSize;
					}

				}
			}
		}

		// Snap to tile grid
		for (auto &room : _rooms) {
			room.pos.x = roundf(room.pos.x / _tileSize) * _tileSize;
			room.pos.y = roundf(room.pos.y / _tileSize) * _tileSize;
		}
	}

	if (!foundIntersection)
		RoomSelection();

}

void DungeonGenerator::RoomSelection()
{
	if (_selectedRooms.size() > 0)
		_selectedRooms.clear();

	float totalArea = 0;
	for (const auto &room : _rooms)
		totalArea += (room.size.x * room.size.y);

	const float nRooms = _rooms.size();
	const float avgArea = totalArea / nRooms;

	const float selectionThreshold = 1.25f;

	for (int i = 0; i < _rooms.size(); i++)
		if ((_rooms[i].size.x * _rooms[i].size.y) > selectionThreshold * avgArea)
			_selectedRooms.push_back(i);
}

void DungeonGenerator::Draw()
{
	for (const auto &room : _rooms)
	{
		DrawRectangle(room.pos.x - room.size.x/2, room.pos.y - room.size.y/2, room.size.x, room.size.y, room.color);
		//DrawRectangle(room.pos.x, room.pos.y, room.size.x, room.size.y, room.color);
		DrawCircle(room.pos.x, room.pos.y, 3, { 255, 0, 0, 255 });
	}

	for (int i = 0; i < _selectedRooms.size(); i++)
	{
		Room room = _rooms[_selectedRooms[i]];
		DrawCircle(room.pos.x, room.pos.y, 3, { 0, 255, 0, 255 });
	}
}


Room::Room(raylib::Vector2 size, raylib::Color color) :
	_id(_ID++), pos({ 0, 0 }), size(size), color(color)
{
}
