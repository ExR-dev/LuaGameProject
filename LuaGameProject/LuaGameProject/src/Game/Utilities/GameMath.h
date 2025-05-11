#pragma once

// Forward declarations
namespace raylib
{
	class Rectangle;
	class Vector2;
	class Vector4;
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


	// Variadic template function that checks if the 
	// first argument equals any of the remaining arguments
	template<typename T, typename... Args>
	bool EqualsAny(const T &a, Args... args) 
	{
		return ((a == args) || ...);
	}

	// Variadic template function that checks if the 
	// first argument equals all of the remaining arguments
	template<typename T, typename... Args>
	bool EqualsAll(const T &a, Args... args) 
	{
		return ((a == args) && ...);
	}

	// Variadic template function that checks if a comparison function 
	// returns true for the first argument and any of the remaining arguments
	template<typename T, typename Compare, typename... Args>
	bool CompareAny(Compare comp, const T &a, Args... args) 
	{
		return (comp(a, args) || ...);
	}

	// Variadic template function that checks if a comparison function 
	// returns true for the first argument and all of the remaining arguments
	template<typename T, typename Compare, typename... Args>
	bool CompareAll(Compare comp, const T &a, Args... args)
	{
		return (comp(a, args) && ...);
	}
}