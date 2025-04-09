#pragma once

#include <vector>

#include "../../raylib-cpp/raylib-cpp.hpp"

struct Room
{
	Vector2 pos;
	Vector2 size;
};

class DungeonGenerator
{
private:
	std::vector<Room> _rooms;

	const unsigned int _tileSize = 10;

public:
	DungeonGenerator();

	void AddRoom(const Room &room);
	void Generate();
	void Draw();
};
