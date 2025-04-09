#include "../../stdafx.h"
#include "Math.h"

using namespace Math;

float Math::Random01()
{
    return (float)rand() / (float)RAND_MAX;
}

Vector2 Math::GetRandomPointInCircle(float radius)
{
    float t = 2 * PI * Random01();
    float u = Random01() + Random01();
    float r = u > 1 ? 2 - u : u;
    return { radius * r * cosf(t), radius * r * sinf(t) };
}

