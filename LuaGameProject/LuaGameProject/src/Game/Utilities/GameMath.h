#pragma once

// Forward declarations
namespace raylib
{
	struct Rectangle;
	struct Vector2;
	struct Vector4;
}

struct ImVec2;
struct ImVec4;


namespace GameMath
{
	ImVec2 RayToImVec(const raylib::Vector2 &vec);
	ImVec4 RayToImVec(const raylib::Vector4 &vec);
	ImVec4 RayToImRect(const raylib::Rectangle &rect);
	ImVec4 RayToImRect(const raylib::Vector2 &vec1, const raylib::Vector2 &vec2);

	raylib::Vector2 ImToRayVec(const ImVec2 &vec);
	raylib::Vector4 ImToRayVec(const ImVec4 &vec);
	raylib::Rectangle ImToRayRect(const ImVec4 &rect);
	raylib::Rectangle ImToRayRect(const ImVec2 &vec1, const ImVec2 &vec2);
}