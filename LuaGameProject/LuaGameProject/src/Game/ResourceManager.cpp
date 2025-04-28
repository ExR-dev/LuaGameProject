#include "stdafx.h"
#include "ResourceManager.h"

using namespace Resource;

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
	std::string filePath = RESOURCE_PATH + SOUND_FOLDER + name;
	raylib::Sound *res = nullptr;
	try
	{
		res = new raylib::Sound(filePath.c_str());
	}
	catch (const raylib::RaylibException &)
	{
		DbgMsg("Sound failed to load!");

		if (res)
			delete res;
		return;
	}

	if (!res->IsValid())
	{
		delete res;
		return;
	}

	m_sounds.emplace(name, res);
}

const raylib::Texture2D *ResourceManager::GetTextureResource(const std::string &name)
{
	return GetResourceFromMap(name, m_textures);
}

raylib::Sound *ResourceManager::GetSoundResource(const std::string &name)
{
	return GetResourceFromMap(name, m_sounds);
}

const std::string ResourceManager::GetSoundResourcePath(const std::string &name) const
{
	return std::string(RESOURCE_PATH + SOUND_FOLDER + name);
}
