#include "../../stdafx.h"
#include "Math.h"

using namespace Math;

float Math::RoundM(float n, float m)
{
    return floorf(((n + m - 1)/m))*m;
}

float Math::Random01()
{
    return (float)rand() / (float)RAND_MAX;
}

float Math::Random(float min, float max)
{
    return min + Random01() * (max - min);
}

Vector2 Math::RandomPointCircle(float radius)
{
    float t = 2 * PI * Random01();
    float u = Random01() + Random01();
    float r = u > 1 ? 2 - u : u;
    return { radius * r * cosf(t), 
             radius * r * sinf(t) };
}

Vector2 Math::RandomGridPointCircle(float radius, float tileSize)
{
    float t = 2 * PI * Random01();
    float u = Random01() + Random01();
    float r = u > 1 ? 2 - u : u;
    return { RoundM(radius * r * cosf(t), tileSize), 
             RoundM(radius * r * sinf(t), tileSize)};
}

