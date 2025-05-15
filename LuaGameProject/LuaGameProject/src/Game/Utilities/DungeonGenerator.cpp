#include "stdafx.h"
#include "DungeonGenerator.h"
#include "Math.h"
#include "Algorithms.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

int Room::m_ID = 0;

Room::Room(raylib::Vector2 size, raylib::Color color) :
	m_id(m_ID++), pos({ 0, 0 }), size(size), color(color)
{
}


using namespace Math;

bool DungeonGenerator::Intersecting(const Room &r1, const Room &r2)
{
	return (r1.pos.x + r1.size.x / 2) > (r2.pos.x - r2.size.x / 2) &&
		   (r1.pos.x - r1.size.x / 2) < (r2.pos.x + r2.size.x / 2) &&
		   (r1.pos.y + r1.size.y / 2) > (r2.pos.y - r2.size.y / 2) &&
		   (r1.pos.y - r1.size.y / 2) < (r2.pos.y + r2.size.y / 2);
}

DungeonGenerator::DungeonGenerator():
	m_position({0, 0})
{
	Initialize();
}

DungeonGenerator::DungeonGenerator(Vector2 pos):
	m_position(pos)
{
	Initialize();
}
DungeonGenerator::DungeonGenerator(raylib::Vector2 pos)
{
	m_position = Vector2(pos.x, pos.y);
	Initialize();
}

void DungeonGenerator::Initialize()
{
	ZoneScopedC(RandomUniqueColor());

	if (m_rooms.size() > 0)
	{
		m_rooms.clear();
		m_selectedRooms.clear();
		m_graph.clear();
	}

	for (int _ = 0; _ < 100; _++)
		AddRoom({{(float)(Math::Random(2, 10)*_tileSize), (float)(Math::Random(2, 10)*_tileSize)}, 
				 {(unsigned char)(Math::Random01f()*255), (unsigned char)(Math::Random01f()*255), (unsigned char)(Math::Random01f()*255), 255}});
}

void DungeonGenerator::AddRoom(const Room &room)
{
	m_rooms.push_back(room);
}

void DungeonGenerator::Generate(float radius)
{
	ZoneScopedC(RandomUniqueColor());

	// Set room positions
	for (auto &room : m_rooms)
		room.pos = Vector2Add(m_position, Math::RandomGridPointCircle(radius, _tileSize));
}

void DungeonGenerator::SeparateRooms()
{
	ZoneScopedC(RandomUniqueColor());

	//bool roomsSeparated = GridSeparation();
	bool roomsSeparated = PhysicalSeparation();
	
	if (roomsSeparated)
	{
		TRACELOG(LOG_ERROR, "Failed genrating dungeon: Intersections still exists!");
		return;
	}

	// TODO: Move to "Generate"
	RoomSelection();
	GenerateGraph();
}

bool DungeonGenerator::GridSeparation()
{
	ZoneScopedC(RandomUniqueColor());

	bool foundIntersection = true;

	// Separate all rooms
	for (int i = 0; i < MAX_ITERATIONS && foundIntersection; i++)
	{
		foundIntersection = false;
		for (auto &room : m_rooms)
		{
			for (auto &other : m_rooms)
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
		for (auto &room : m_rooms) {
			room.pos.x = roundf(room.pos.x / _tileSize) * _tileSize;
			room.pos.y = roundf(room.pos.y / _tileSize) * _tileSize;
		}
	}

	return foundIntersection;
}

bool DungeonGenerator::PhysicalSeparation()
{
	ZoneScopedC(RandomUniqueColor());

	bool foundIntersection = true;

	std::vector<std::pair<Room*, Vector2>> resolutions;
	resolutions.reserve(m_rooms.size());

	// Separate all rooms
	for (int i = 0; i < MAX_ITERATIONS && foundIntersection; i++)
	{
		foundIntersection = false;
		resolutions.clear();

		for (auto &room : m_rooms)
		{
			for (auto &other : m_rooms)
			{
				if (room != other && Intersecting(room, other))
				{
					foundIntersection = true;

					Vector2 normal = Vector2Normalize(Vector2Subtract(room.pos, other.pos));
					auto existing = std::find_if(resolutions.begin(), 
												 resolutions.end(), 
												 [&room](std::pair<Room *, Vector2> p) {return p.first == &room; });

					if (existing != resolutions.end())
						existing->second = Vector2Add(existing->second, normal); // Handle overlap with multiple
					else
						resolutions.push_back({ &room, normal });
				}
			}
		}

		for (auto &pairs : resolutions)
			pairs.first->pos = Vector2Add(pairs.first->pos, Vector2Add(pairs.second, Vector2(Math::Random01f()/100, Math::Random01f()/100)));
	}

	return foundIntersection;
}

void DungeonGenerator::RoomSelection()
{
	ZoneScopedC(RandomUniqueColor());

	if (m_selectedRooms.size() > 0)
		m_selectedRooms.clear();

	// Compute total area
	float totalArea = 0;
	for (const auto &room : m_rooms)
		totalArea += (room.size.x * room.size.y);

	const float nRooms = m_rooms.size();
	const float avgArea = totalArea / nRooms;

	const float selectionThreshold = 1.5f;

	// Select main-rooms based on area
	for (int i = 0; i < m_rooms.size(); i++)
		if ((m_rooms[i].size.x * m_rooms[i].size.y) > selectionThreshold * avgArea)
			m_selectedRooms.push_back(i);
}

void DungeonGenerator::GenerateGraph()
{
	ZoneScopedC(RandomUniqueColor());

	std::vector<Point> points;
	
	for (const auto &room : m_selectedRooms)
		points.push_back(m_rooms[room].pos);

	// Do Delaunay Triangulation
	std::vector<Triangle> triangles = BowyerWatson(points);

	// Get lines
	bool found;
	for (const auto &triangle : triangles)
		for (int e = 0; e < 3; e++)
		{
			found = false;
			Line edge = triangle.GetEdge(e);

			for (const auto &line : m_graph)
				found |= (line == edge);

			if (!found)
				m_graph.push_back(edge);
		}

	// Create MST
	std::vector<Line> oldGraph(m_graph);
	m_graph = Kruskal(m_graph);

	// Add some of the removed lines back
	const float addBackRate = 0.15f;
	for (int i = 0; i < oldGraph.size(); i++)
		if (std::find(m_graph.begin(), m_graph.end(), oldGraph[i]) == m_graph.end())
			if (Random01f() < addBackRate)
				m_graph.push_back(oldGraph[i]);

}

void DungeonGenerator::Draw()
{
	ZoneScopedC(RandomUniqueColor());

	const float padding = 4;
	for (const auto &room : m_rooms)
	{
		DrawRectangle(room.pos.x - room.size.x/2, room.pos.y - room.size.y/2, room.size.x, room.size.y, {255, 255, 255, 255});
		DrawRectangle(room.pos.x - (room.size.x-padding)/2, room.pos.y - (room.size.y-padding)/2, room.size.x - padding, room.size.y - padding, {0, 0, 255, 255});
		//DrawRectangle(room.pos.x, room.pos.y, room.size.x, room.size.y, room.color);
		DrawCircle(room.pos.x, room.pos.y, 3, { 255, 0, 0, 255 });
	}

	for (int i = 0; i < m_selectedRooms.size(); i++)
	{
		Room room = m_rooms[m_selectedRooms[i]];
		DrawCircle(room.pos.x, room.pos.y, 3, { 0, 255, 0, 255 });
	}

	for (const auto &edge : m_graph)
		DrawLine(edge.p1.x, edge.p1.y, edge.p2.x, edge.p2.y, { 0, 255, 0, 255 });
}
