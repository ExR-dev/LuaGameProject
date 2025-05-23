#pragma once

#include <vector>

#include "dep/raylib-cpp/raylib-cpp.hpp"

#include "Math.h"

struct Room
{
	Room(raylib::Vector2 size, const std::string &name);

	raylib::Vector2 pos;
	raylib::Vector2 size;
	std::string p_name;

	inline int GetID() const { return m_id; }

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

	// Aguments: position (Vec2)
	// Returns: none
	static int lua_Initialize(lua_State *L);

	// Aguments: room (Room)
	// Returns: none
	static int lua_AddRoom(lua_State *L);

	// Aguments: none
	// Returns: rooms (table<Room>)
	static int lua_GetRooms(lua_State* L);

	// Aguments: radius (float)
	// Returns: none
	static int lua_Generate(lua_State *L);
	 
	// Aguments: selectionThreshold (float)
	// Returns: none
	static int lua_SeparateRooms(lua_State *L);

	// Aguments: none
	// Returns: none
	static int lua_Reset(lua_State *L);


public:
	static DungeonGenerator &Instance()
	{
		static DungeonGenerator instance;
		return instance;
	}

	void BindToLua(lua_State *L);

	void Initialize(Vector2 pos);

	void AddRoom(const Room &room);

	void Generate(float radius);
	void Reset();

	void SeparateRooms(float selectionThreshold = 1.5f);
	bool GridSeparation();
	bool PhysicalSeparation();

	void RoomSelection(float selectionThreshold = 1.5f);
	void GenerateGraph();

	void Draw();
};
