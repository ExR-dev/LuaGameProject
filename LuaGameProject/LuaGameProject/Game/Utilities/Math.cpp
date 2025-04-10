#include "../../stdafx.h"
#include "Math.h"

using namespace Math;

float Math::RoundM(float n, float m)
{
    return floorf(((n + m - 1)/m))*m;
}

float Math::Random01f()
{
    return (float)rand() / (float)RAND_MAX;
}

float Math::Randomf(float min, float max)
{
    return min + Random01f() * (max - min);
}

int Math::Random(int min, int max)
{
    return min + rand() % (max - min);
}

Vector2 Math::RandomPointCircle(float radius)
{
    float t = 2 * PI * Random01f();
    float u = Random01f() + Random01f();
    float r = u > 1 ? 2 - u : u;
    return { radius * r * cosf(t), 
             radius * r * sinf(t) };
}

Vector2 Math::RandomGridPointCircle(float radius, float tileSize)
{
    float t = 2 * PI * Random01f();
    float u = Random01f() + Random01f();
    float r = u > 1 ? 2 - u : u;
    return { RoundM(radius * r * cosf(t), tileSize), 
             RoundM(radius * r * sinf(t), tileSize)};
}

bool Math::IsTriangleCounterClockwise(const Triangle &t)
{
    return (t.p2.x - t.p1.x) * (t.p3.y - t.p1.y) - (t.p3.x - t.p1.x) * (t.p2.y - t.p1.y) > 0;
}

bool Math::IsPointInsideTriangleCircumcircle(const Triangle &t, const Point &p)
{
    float ax = t.p1.x - p.x,
          ay = t.p1.y - p.y,
          bx = t.p2.x - p.x,
          by = t.p2.y - p.y,
          cx = t.p3.x - p.x,
          cy = t.p3.y - p.y;

    float det = ((ax*ax + ay*ay) * (bx*cy - cx*by) -
                 (bx*bx + by*by) * (ax*cy - cx*ay) +
                 (cx*cx + cy*cy) * (ax*by - bx*ay));

    return IsTriangleCounterClockwise(t) ? (det > 0) : (det < 0);
}
