#pragma once
#include <string>
#include <vector>
#include <array>
#include <map>
#include "dep/raylib-cpp/raylib-cpp.hpp"

#define RESOURCE_PATH std::string("./res/")
#define TEXTURE_FOLDER std::string("Textures/")
#define FONT_FOLDER std::string("Fonts/")
#define SOUND_FOLDER std::string("Sounds/")

constexpr auto SOUND_POOL_SIZE = 8;

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

		operator T *()
		{
			return resource;
		}

	private:
		T *resource;
	};

	struct SoundPool
	{
	public:
		SoundPool() = default;
		~SoundPool() = default;

		bool Init(const std::string &name);

		raylib::Sound *Pop();

	private:
		std::array<raylib::Sound, SOUND_POOL_SIZE> m_sounds;
		int m_nextPop = 0;
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
	static void LoadResources();

	static void UnloadResources();

	// Load a texture
	static void LoadTextureResource(const std::string &name);

	// Load a ttf font
	static void LoadFontResource(const std::string &name);

	// Load a sound
	static void LoadSoundResource(const std::string &name);

	// Get a texture by name
	static const raylib::Texture2D *GetTextureResource(const std::string &name);
	static std::vector<std::string> GetTextureNames();

	// Get a font by name
	static raylib::Font *GetFontResource(const std::string &name);
	static std::vector<std::string> GetFontNames();

	// Get a sound by name
	static raylib::Sound *GetSoundResource(const std::string &name);

	static const std::string GetSoundResourcePath(const std::string &name);

private:
	std::map<std::string, Resource::ManagedResource<raylib::Texture2D>> m_textures;
	std::map<std::string, Resource::ManagedResource<raylib::Font>> m_fonts;
	std::map<std::string, Resource::ManagedResource<Resource::SoundPool>> m_sounds;

	template<typename T>
	static T *GetResourceFromMap(const std::string &name, std::map<std::string, Resource::ManagedResource<T>> &map);
};


template<typename T>
inline static T *ResourceManager::GetResourceFromMap(const std::string &name, std::map<std::string, Resource::ManagedResource<T>> &map)
{
	auto it = map.find(name);
	if (it == map.end())
		return nullptr;

	return (it->second);
}
