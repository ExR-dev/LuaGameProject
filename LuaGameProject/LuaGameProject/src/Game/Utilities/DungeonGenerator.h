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

	const unsigned int m_tileSize = 10;
	bool m_isInitialized = false;

	Vector2 m_position = { 0, 0 };

	bool Intersecting(const Room &r1, const Room &r2);

	DungeonGenerator() = default;
	~DungeonGenerator() = default;

public:
	static DungeonGenerator &Instance()
	{
		static DungeonGenerator instance;
		return instance;
	}

	void Initialize(Vector2 pos);

	void AddRoom(const Room &room);

	void Generate(float radius);
	void Reset();

	void SeparateRooms();
	bool GridSeparation();
	bool PhysicalSeparation();

	void RoomSelection();
	void GenerateGraph();

	void Draw();
};
