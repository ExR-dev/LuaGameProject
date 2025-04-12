#pragma once

#include "Math.h"

#include <vector>

Math::Triangle SuperTriangle(const std::vector<Math::Point> &points);

std::vector<Math::Triangle> BowyerWatson(const std::vector<Math::Point> &points);

std::vector<int> Prim(const std::vector<std::vector<float>> &graph); // TODO: Validate

std::vector<Math::Line> Kruskal(const std::vector<Math::Line> graph);
