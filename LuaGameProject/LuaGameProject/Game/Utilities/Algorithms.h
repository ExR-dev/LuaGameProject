#pragma once

#include "Math.h"

#include <vector>

Math::Triangle SuperTriangle(const std::vector<Math::Point> &points);

std::vector<Math::Triangle> BowyerWatson(const std::vector<Math::Point> &points);
