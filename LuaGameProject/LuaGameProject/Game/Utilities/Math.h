#pragma once

#include <stdlib.h>

namespace Math
{ 

float RoundM(float n, float m);

float Random01f();
float Randomf(float min, float max);
int Random(int min, int max);

Vector2 RandomPointCircle(float radius);
Vector2 RandomGridPointCircle(float radius, float tileSize);

}
