#pragma once
#include <string>
#include <vector>
#include <map>
#include "dep/raylib-cpp/raylib-cpp.hpp"

#define RESOURCE_PATH std::string("./res/")
#define TEXTURE_FOLDER std::string("Textures/")
#define SOUND_FOLDER std::string("Sounds/")

namespace Resource
{
	template<typename T>
	struct ManagedResource
	{
	public:
		ManagedResource(T *resource) : resource(resource) {}
		~ManagedResource()
		{
			if (resource)
			{
				delete resource;
				resource = nullptr;
			}
		}

		// Move constructor
		ManagedResource(ManagedResource &&other) noexcept
		{
			if (resource)
			{
				delete resource;
				resource = nullptr;
			}

			resource = other.resource;
			other.resource = nullptr;
		}

		operator const T *() const
		{
			return resource;
		}

	private:
		const T *resource;
	};
}

class ResourceManager
{
public:
	ResourceManager() = default;
	~ResourceManager() = default;

	static ResourceManager &Instance() noexcept
	{
		static ResourceManager instance;
		return instance;
	}

	// Load all resources in the resource folder
	void LoadResources();

	// Load a texture
	void LoadTexture(const std::string &name);

	// Load a sound
	void LoadSound(const std::string &name);

	// Get a texture by name
	const raylib::Texture2D *GetTexture(const std::string &name) const;

	// Get a sound by name
	const raylib::Sound *GetSound(const std::string &name) const;

private:
	std::map<std::string, Resource::ManagedResource<raylib::Texture2D>> m_textures;
	std::map<std::string, Resource::ManagedResource<raylib::Sound>> m_sounds;

	template<typename T>
	static const T *GetResourceFromMap(const std::string &name, const std::map<std::string, Resource::ManagedResource<T>> &map);
};


template<typename T>
inline static const T *ResourceManager::GetResourceFromMap(const std::string &name, const std::map<std::string, Resource::ManagedResource<T>> &map)
{
	if (name == "")
		return nullptr;

	auto it = map.find(name);
	if (it == map.end())
		return nullptr;

	return (it->second);
}
