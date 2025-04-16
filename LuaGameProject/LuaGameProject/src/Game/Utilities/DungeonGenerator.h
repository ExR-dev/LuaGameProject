#pragma once

#include <vector>

#include "dep/raylib-cpp/raylib-cpp.hpp"

#include "Math.h"

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

#define MAX_ITERATIONS 1000

class DungeonGenerator
{
private:
	std::vector<Room> _rooms;
	std::vector<int> _selectedRooms;
	std::vector<Math::Line> _graph;

	const unsigned int _tileSize = 10;

	Vector2 _position;

	bool Intersecting(const Room &r1, const Room &r2);

public:
	DungeonGenerator(Vector2 pos);

	void Initialize();

	void AddRoom(const Room &room);

	void Generate(float radius);

	void SeparateRooms();
	bool GridSeparation();
	bool PhysicalSeparation();

	void RoomSelection();
	void GenerateGraph();

	void Draw();
};
