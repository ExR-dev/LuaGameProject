#include "stdafx.h"
#include "ResourceManager.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

bool Resource::SoundPool::Init(const std::string &name)
{
	ZoneScopedC(RandomUniqueColor());

	const std::string filePath = RESOURCE_PATH + SOUND_FOLDER + name;

	m_sounds = std::array<raylib::Sound, SOUND_POOL_SIZE>();

	for (int i = 0; i < SOUND_POOL_SIZE; ++i)
	{
		raylib::Sound &soundRef = m_sounds[i];

		try
		{
			soundRef = raylib::Sound(filePath.c_str());
		}
		catch (const raylib::RaylibException &)
		{
			DbgMsg("Sound failed to load!");
			return false;
		}

		if (!soundRef.IsValid())
		{
			DbgMsg("Sound loaded but is invalid!");
			return false;
		}
	}

	return true;
}

raylib::Sound *Resource::SoundPool::Pop()
{
	if (m_nextPop >= SOUND_POOL_SIZE)
		m_nextPop = 0;

	return &m_sounds[m_nextPop++];
}


void ResourceManager::LoadResources()
{
	ZoneScopedC(RandomUniqueColor());

	m_textures.clear();
	m_sounds.clear();

	// Find all textures in the texture subfolder
	std::vector<std::string> textureFiles;
	std::string texturePath(RESOURCE_PATH + TEXTURE_FOLDER);

	for (const auto &entry : fs::directory_iterator(texturePath))
	{
		if (entry.is_regular_file())
		{
			textureFiles.push_back(entry.path().filename().string());
		}
	}

	// Load all textures
	for (const auto &file : textureFiles)
	{
		LoadTextureResource(file);
	}

	// Find all sounds in the sound subfolder
	std::vector<std::string> soundFiles;
	std::string soundPath(RESOURCE_PATH + SOUND_FOLDER);

	for (const auto &entry : fs::directory_iterator(soundPath))
	{
		if (entry.is_regular_file())
		{
			soundFiles.push_back(entry.path().filename().string());
		}
	}

	// Load all sounds
	for (const auto &file : soundFiles)
	{
		LoadSoundResource(file);
	}
}

void ResourceManager::UnloadResources()
{
	ZoneScopedC(RandomUniqueColor());

	m_textures.clear();
	m_sounds.clear();
}

void ResourceManager::LoadTextureResource(const std::string &name)
{
	std::string filePath = RESOURCE_PATH + TEXTURE_FOLDER + name;
	raylib::Texture2D *res = nullptr;
	try
	{
		res = new raylib::Texture2D(filePath.c_str());
	}
	catch (const raylib::RaylibException &)
	{
		DbgMsg("Texture failed to load!");

		if (res)
			delete res;
		return;
	}

	if (!res->IsValid())
	{
		delete res;
		return;
	}

	m_textures.emplace(name, res);
}

void ResourceManager::LoadSoundResource(const std::string &name)
{
	Resource::SoundPool *res = new Resource::SoundPool();

	if (!res->Init(name))
	{
		DbgMsg("Sound failed to load!");
		delete res;
		return;
	}

	m_sounds.emplace(name, res);
}

const raylib::Texture2D *ResourceManager::GetTextureResource(const std::string &name)
{
	return GetResourceFromMap(name, m_textures);
}

std::vector<std::string> ResourceManager::GetTextureNames()
{
	std::vector<std::string> values;
	values.reserve(m_textures.size());
	for (auto &[k, v] : m_textures)
		values.push_back(k);
	return values;
}

raylib::Sound *ResourceManager::GetSoundResource(const std::string &name)
{
	return GetResourceFromMap(name, m_sounds)->Pop();
}

const std::string ResourceManager::GetSoundResourcePath(const std::string &name) const
{
	return std::string(RESOURCE_PATH + SOUND_FOLDER + name);
}
