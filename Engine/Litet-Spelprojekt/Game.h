#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Scene.h"
#include "Graphics.h"
#include "Content.h"
#include "Time.h"
#include "Input.h"
#include "Window.h"
#include "DebugDrawer.h"

/// Game handles loading content like textures and meshes, as well as managing the update and render steps of the main game loop.
class Game
{
private:
	Microsoft::WRL::ComPtr<ID3D11Device>		_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	_immediateContext;

	Graphics	_graphics;
	Content		_content;
	std::vector<Scene *> _scenes;
	UINT		_activeSceneIndex;
	Window		_window;

	struct TextureData		{ std::string name;	bool transparent; };
	struct TextureMapData	{ TextureType type;	std::string name;	std::string file; };
	struct ShaderData		{ ShaderType type;	std::string name;	std::string file; };
	struct HeightMapData	{ std::string name; std::string file; };

	[[nodiscard]] bool CompileContent(
		const std::string &compiledContentFile,
		const std::vector<std::string> &meshNames
	);
	[[nodiscard]] bool DecompileContent(
		const std::string &compiledContentFile
	);

	[[nodiscard]] bool LoadContent(
		const std::string &compiledContentFile,
		const std::vector<TextureData> &textureNames,
		const std::vector<TextureMapData> &textureMapNames,
		const std::vector<ShaderData> &shaderNames,
		const std::vector<HeightMapData> &heightMapNames
	);

	float _tickRate = 1.0f / 20.0f;
	float _tickTimer = 0.0f;

	float _gameVolume = 10.0f;

	bool _GameWon = false;

public:
	Game();
	~Game();

	[[nodiscard]] bool Setup(Time &time, UINT width, UINT height, Window window);

	/// Returns true if active scene exists and is initialized, Otherwise false.
	[[nodiscard]] bool ActiveSceneIsValid();

	/// Adds newScene to _scenes and initializes newScene if uninitialized, newScene is set as active scene if setActive is true.
	[[nodiscard]] bool AddScene(Scene *newScene, const bool setActive = false, const std::string &saveFile = "");

	/// Sets _activeSceneIndex to sceneIndex and initializes the now active scene if uninitialized.
	[[nodiscard]] bool SetScene(const UINT sceneIndex);
	[[nodiscard]] Scene *GetScene(const UINT sceneIndex);
	[[nodiscard]] const std::vector<Scene *> *GetScenes() const;
	[[nodiscard]] UINT GetActiveScene() const;

	[[nodiscard]] Graphics *GetGraphics();

	[[nodiscard]] Window GetWindow();

	[[nodiscard]] bool Update(Time &time, const Input &input);
	[[nodiscard]] bool Render(Time &time, const Input &input);

};
