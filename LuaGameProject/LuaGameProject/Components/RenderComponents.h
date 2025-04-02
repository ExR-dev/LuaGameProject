#pragma once

#include "raylib-cpp/raylib-cpp.hpp"
#include "EnTT/entt.hpp"

namespace Component
{
	struct Render
	{
		raylib::Color color;
		bool visible;
	};

	struct Texture2D
	{
		raylib::Texture2D texture;

		operator const raylib::Texture2D& () const { return texture; }
		operator raylib::Texture2D& () { return texture; }
	};

	struct Model
	{
		raylib::Model model;

		operator const raylib::Model& () const { return model; }
		operator raylib::Model& () { return model; }
	};

	struct Sprite
	{
		raylib::Rectangle source;
	};

	struct Text
	{
		std::string text;
		raylib::Font font;
		int fontSize;
	};

	struct Cube
	{
		raylib::Vector3 position;
		raylib::Vector3 size;
	};

	struct Sphere
	{
		raylib::Vector3 position;
		float radius;
	};

	struct Cylinder
	{
		raylib::Vector3 position;
		float radius;
		float height;
	};
}
