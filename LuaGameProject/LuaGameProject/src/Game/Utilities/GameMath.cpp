#include "stdafx.h"
#include "GameMath.h"

ImVec2 GameMath::RayToImVec(const raylib::Vector2 &vec)
{
	return ImVec2(vec.x, vec.y);
}
ImVec4 GameMath::RayToImVec(const raylib::Vector4 &vec)
{
	return ImVec4(vec.x, vec.y, vec.z, vec.w);
}
ImVec4 GameMath::RayToImRect(const raylib::Rectangle &rect)
{
	return ImVec4(rect.x, rect.y, rect.width, rect.height);
}
ImVec4 GameMath::RayToImRect(const raylib::Vector2 &vec1, const raylib::Vector2 &vec2)
{
	return ImVec4(vec1.x, vec1.y, vec2.x, vec2.y);
}

raylib::Vector2 GameMath::ImToRayVec(const ImVec2 &vec)
{
	return raylib::Vector2(vec.x, vec.y);
}
raylib::Vector4 GameMath::ImToRayVec(const ImVec4 &vec)
{
	return raylib::Vector4(vec.x, vec.y, vec.z, vec.w);
}
raylib::Rectangle GameMath::ImToRayRect(const ImVec4 &rect)
{
	return raylib::Rectangle(rect.x, rect.y, rect.z, rect.w);
}
raylib::Rectangle GameMath::ImToRayRect(const ImVec2 &pos, const ImVec2 &size)
{
	return raylib::Rectangle(pos.x, pos.y, size.x, size.y);
}

