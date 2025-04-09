#pragma once

#include <stdlib.h>

namespace Math
{ 

float RoundM(float n, float m);

float Random01();
float Random(float min, float max);

Vector2 RandomPointCircle(float radius);
Vector2 RandomGridPointCircle(float radius, float tileSize);

}
