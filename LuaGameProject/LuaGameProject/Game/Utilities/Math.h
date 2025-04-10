#pragma once

#include "../../raylib-cpp/raylib-cpp.hpp"
#include <stdlib.h>
#include <array>

namespace Math
{ 

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct Point
{
	float x, y;

	Point(float x, float y) : x(x), y(y) {};
	Point(Vector2 p) : x(p.x), y(p.y) {};

	Point(const Point &) = default;
	Point(Point &&) = default;
	Point &operator=(const Point &) = default;
	Point &operator=(Point &&) = default;

	inline Vector2 ToVector() const { return Vector2(x, y); }

	inline bool operator==(const Point &other) const
	{
		return x == other.x && y == other.y;
	}
};

struct Line
{
	Point p1, p2;

	Line(Point p, Point q) : p1(p), p2(q) {};
	Line(Vector2 p, Vector2 q) : p1(p), p2(q) {};

	Line(const Line &) = default;
	Line(Line &&) = default;
	Line &operator=(const Line &) = default;
	Line &operator=(Line &&) = default;

	inline bool operator==(const Line &other) const
	{ 
		return (p1 == other.p1 && p2 == other.p2) ||
			   (p1 == other.p2 && p2 == other.p1);
	}

	inline bool operator!=(const Line &other) const
	{
		return !(*this == other);
	}

	float weight() const {
		return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
	}
};

struct Triangle
{
	Point p1, p2, p3;

	Triangle(Point p, Point q, Point r) : p1(p), p2(q), p3(r) {};
	Triangle(Vector2 p, Vector2 q, Vector2 r) : p1(p), p2(q), p3(r) {};

	Triangle(const Triangle &) = default;
	Triangle(Triangle &&) = default;
	Triangle &operator=(const Triangle &) = default;
	Triangle &operator=(Triangle &&) = default;

	inline std::array<Point, 3> GetPointList() const
	{
		return { p1, p2, p3 };
	}

	inline Line GetEdge(int index) const
	{
		if (index == 0)
			return { p1, p2 };
		if (index == 1)
			return { p2, p3 };
		return { p3, p1 };
	}

	inline bool operator==(const Triangle &other) const
	{
		std::array<Point, 3> a = GetPointList();
		std::array<Point, 3> b = other.GetPointList();

		// Check that every point in 'a' is in 'b'
		return std::all_of(a.begin(), a.end(), [&](const Point &pa) {
			return std::any_of(b.begin(), b.end(), [&](const Point &pb) {
				return pa == pb;
				});
			});
	}
};

float RoundM(float n, float m);

float Random01f();
float Randomf(float min, float max);
int Random(int min, int max);

Vector2 RandomPointCircle(float radius);
Vector2 RandomGridPointCircle(float radius, float tileSize);

bool IsTriangleCounterClockwise(const Triangle &t);
bool IsPointInsideTriangleCircumcircle(const Triangle &t, const Point &p);

}
