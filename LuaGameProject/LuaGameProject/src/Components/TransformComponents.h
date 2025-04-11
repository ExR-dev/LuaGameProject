#pragma once

#include "dep/raylib-cpp/raylib-cpp.hpp"
#include "dep/EnTT/entt.hpp"

namespace Component
{
	struct Transform
	{
		/*Vector3 position;
		Vector3 rotationAxis;
		float rotationAngle;
		Vector3 scale;*/

		raylib::Matrix transform;

		operator const raylib::Matrix& () const { return transform; }
		operator raylib::Matrix& () { return transform; }
	};

	struct Hierarchy
	{
		std::size_t children;
		entt::entity firstChild{entt::null};
		entt::entity parent{entt::null};
		entt::entity prevSibiling{entt::null};
		entt::entity nextSibiling{entt::null};
	};
}
