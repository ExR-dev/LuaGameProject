#pragma once

#include <vector>

#include "../../raylib-cpp/raylib-cpp.hpp"

struct Room
{
	Room(raylib::Vector2 size, raylib::Color color);

	raylib::Vector2 pos;
	raylib::Vector2 size;
	raylib::Color color;

	bool operator==(const Room &other) 
	{
		return _id == other._id;
	};

	bool operator!=(const Room &other)
	{
		return _id != other._id;
	}

private:
	static int _ID;
	const int _id;
};

class DungeonGenerator
{
private:
	std::vector<Room> _rooms;

	const unsigned int _tileSize = 10;

	Vector2 _position;

	bool Intersecting(const Room &r1, const Room &r2);

public:
	DungeonGenerator(Vector2 pos);

	void Initialize();

	void AddRoom(const Room &room);

	void Generate(float radius);
	void SeperateRooms();

	void Draw();
};
