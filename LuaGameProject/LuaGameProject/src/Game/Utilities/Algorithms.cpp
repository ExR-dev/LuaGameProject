#include "stdafx.h"
#include "Algorithms.h"

using namespace Math;

Math::Triangle SuperTriangle(const std::vector<Point> &points)
{
    Point min = { INFINITY, INFINITY },
          max = { -INFINITY, -INFINITY };

    for (const auto &point : points)
    {
        min.x = MIN(min.x, point.x);
        min.y = MIN(min.y, point.y);
        max.x = MAX(max.x, point.x);
        max.y = MAX(max.y, point.y);
    }

    float dx = (max.x - min.x) * 10,
          dy = (max.y - min.y) * 10;

    return { Point(min.x - dx     , min.y - dy * 3),
             Point(min.x - dx     , max.y + dy    ),
             Point(max.x + dx * 3 , max.y + dy    ) };
}

std::vector<Triangle> BowyerWatson(const std::vector<Point> &points)
{
    std::vector<Triangle> triangles;
    std::vector<Triangle> badTriangles;
    std::vector<Line> polygon;

    // TODO: Add super triangle
    const Triangle super = SuperTriangle(points);
    triangles.push_back(super);

    bool shared;

    for (const auto &point : points)
    {
        badTriangles.clear();

        for (const auto &triangle : triangles)
            if (IsPointInsideTriangleCircumcircle(triangle, point))
                badTriangles.push_back(triangle);

        polygon.clear();

        for (const auto &triangle : badTriangles)
            for (int e = 0; e < 3; e++)
            {
                shared = false;
                const Line edge = triangle.GetEdge(e);
                for (const auto &other : badTriangles)
                    if (triangle != other)
                        for (int eo = 0; eo < 3; eo++)
                            if (edge == other.GetEdge(eo))
                                shared = true;
                if (!shared)
                    polygon.push_back(edge);
            }

        for (int i = badTriangles.size()-1; i >= 0; i--)
            triangles.erase(std::remove(triangles.begin(), triangles.end(), badTriangles[i]), triangles.end());

                        
        for (const auto &edge : polygon)
            triangles.push_back({ edge.p1, edge.p2, point });
    }

    // Clean up
    for (int i = triangles.size()-1; i >= 0 ; i--)
    {
        shared = false;
        for (const auto &point : triangles[i].GetPointList())
            for (const auto &sPoint : super.GetPointList())
                if (point == sPoint)
                    shared = true;
        if (shared)
            triangles.erase(std::remove(triangles.begin(), triangles.end(), triangles[i]), triangles.end());
    }


    return triangles;
}

std::vector<int> Prim(const std::vector<std::vector<float>> &graph)
{
    const unsigned int n = graph.size();
	std::vector<int> res;

	int no_edge = 0;
    std::vector<bool> selected(n, false);

	selected[0] = true;

	int x, y;
	while (no_edge < n - 1)
	{
		int min = INFINITY;
		x = 0;
		y = 0;

		for (int i = 0; i < n; i++)
			if (selected[i])
				for (int j = 0; j < n; j++)
					if (!selected[j])
						if (min > graph[i][j])
						{
							min = graph[i][j];
							x = i;
							y = j;
						}

		selected[y] = true;
		no_edge++;
	}

	for (int i = 0; i < n; i++)
		if (selected[i])
			res.push_back(i);

	return res;
}

