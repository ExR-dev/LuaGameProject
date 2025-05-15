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
		return m_id == other.m_id;
	};

	bool operator!=(const Room &other)
	{
		return m_id != other.m_id;
	}

private:
	static int m_ID;
	const int m_id;
};

#define MAX_ITERATIONS 1000

class DungeonGenerator
{
private:
	std::vector<Room> m_rooms;
	std::vector<int> m_selectedRooms;
	std::vector<Math::Line> m_graph;

	const unsigned int _tileSize = 10;

	Vector2 m_position;

	bool Intersecting(const Room &r1, const Room &r2);

public:
	DungeonGenerator();
	DungeonGenerator(Vector2 pos);
	DungeonGenerator(raylib::Vector2 pos);

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
