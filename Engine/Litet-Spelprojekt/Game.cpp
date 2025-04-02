#include "stdafx.h"
#include "Game.h"
#include "ErrMsg.h"
#include "D3D11Helper.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using namespace DirectX;


Game::Game()
{
	_activeSceneIndex = 0;
}

Game::~Game()
{
	_graphics.Shutdown();
	_content.Shutdown();
}

bool Game::CompileContent(
	const std::string &compiledContentFile,
	const std::vector<std::string> &meshNames)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(684262881, "Compile Content");
#endif

	std::ofstream writer;
	writer.open(compiledContentFile, std::ios::binary | std::ios::ate);
	if (!writer.is_open())
	{
		ErrMsg("Failed to open compiled content file!");
		return false;
	}

	for (int i = 0; i < meshNames.size(); i++)
	{
		const std::string &meshName = meshNames[i];

		CompiledData data = _content.GetMeshData(
			std::format("Mesh_{}", meshName), 
			std::format("Content\\Meshes\\{}.obj", meshName).c_str()
		);

		// Write name of file, size of contents, then contents
		writer.write((char *)(meshName + '\0').data(), meshName.size() + 1);
		writer.write((char *)&data.size, sizeof(size_t));
		writer.write(data.data, data.size);
		writer.flush();
	}

	writer.close();
	return true;
}

bool Game::DecompileContent(const std::string &compiledContentFile)
{
	std::ifstream reader(compiledContentFile, std::ios::binary | std::ios::in | std::ios::ate);
	if (!reader.is_open())
	{
		ErrMsg("Failed to open compiled content file!");
		return false;
	}

	size_t fileSize = reader.tellg();
	reader.seekg(0, std::ios::beg);

	while (reader.tellg() < fileSize)
	{
		std::string meshName = "";
		while (true)
		{
			char c = 0;
			reader.read(&c, 1);

			if (c == '\0')
				break;

			meshName += c;
		}

		size_t size = 0;
		reader.read((char *)&size, sizeof(size_t));

		std::vector<char> data;
		data.resize(size);
		reader.read(data.data(), size);

		MeshData meshData;
		size_t offset = 0;
		meshData.Decompile(data, offset);

		if (_content.AddMesh(_device.Get(), std::format("Mesh_{}", meshName), meshData) == CONTENT_NULL)
		{
			ErrMsg(std::format("Failed to add Mesh_{}!", meshName));
			reader.close();
			return false;
		}
	}

	reader.close();
	return true;
}

bool Game::LoadContent(
	const std::string &compiledContentFile,
	const std::vector<TextureData> &textureNames,
	const std::vector<TextureMapData> &textureMapNames,
	const std::vector<ShaderData>& shaderNames,
	const std::vector<HeightMapData>& heightMapNames)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(986841766, "Load Content");
#endif

	if (!DecompileContent(compiledContentFile))
	{
		ErrMsg("Failed to decompile content!");
		return false;
	}

#pragma warning(disable: 6993)
#pragma omp parallel num_threads(PARALLEL_THREADS)
	{
#pragma omp for nowait
		for (int i = 0; i < textureNames.size(); i++)
		{
			const TextureData &texture = textureNames[i];

			if (_content.AddTexture(_device.Get(), _immediateContext.Get(), std::format("Tex_{}", texture.name),
				std::format("Content\\Textures\\{}.png", texture.name).c_str(), texture.transparent) == CONTENT_NULL)
			{
				ErrMsg(std::format("Failed to add Tex_{}!", texture.name));
			}
		}

#pragma omp for nowait
		for (int i = 0; i < textureMapNames.size(); i++)
		{
			const TextureMapData &textureMap = textureMapNames[i];

			if (_content.AddTextureMap(_device.Get(), _immediateContext.Get(), std::format("TexMap_{}", textureMap.name),
				textureMap.type, std::format("Content\\Textures\\{}.png", textureMap.file).c_str()) == CONTENT_NULL)
			{
				ErrMsg(std::format("Failed to add TexMap_{}!", textureMap.name));
			}
		}

#pragma omp for nowait
		for (int i = 0; i < heightMapNames.size(); i++)
		{
			const HeightMapData &heightMap = heightMapNames[i];

			if (_content.AddHeightMap(std::format("HM_{}", heightMap.name), std::format("Content\\Textures\\{}.png", heightMap.name).c_str()) == CONTENT_NULL)
			{
				ErrMsg(std::format("Failed to add HM_{}!", heightMap.name));
			}
		}
	}
#pragma warning(default: 6993)

	for (const ShaderData &shader : shaderNames)
	{
		if (_content.AddShader(_device.Get(), shader.name, shader.type, std::format("Content\\Shaders\\{}.cso", shader.file).c_str()) == CONTENT_NULL)
		{
			ErrMsg(std::format("Failed to add {} shader!", shader.name));
			return false;
		}
	}

	if (_content.AddSampler(_device.Get(), "SS_Fallback", D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER_ANISOTROPIC) == CONTENT_NULL)
	{
		ErrMsg("Failed to add fallback sampler!");
		return false;
	}

	if (_content.AddSampler(_device.Get(), "SS_Point", D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR) == CONTENT_NULL)
	{
		ErrMsg("Failed to add fallback sampler!");
		return false;
	}

	if (_content.AddSampler(_device.Get(), "SS_Clamp", D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER_ANISOTROPIC) == CONTENT_NULL)
	{
		ErrMsg("Failed to add clamp sampler!");
		return false;
	}

	if (_content.AddSampler(_device.Get(), "SS_Wrap", D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_ANISOTROPIC) == CONTENT_NULL)
	{
		ErrMsg("Failed to add clamp sampler!");
		return false;
	}

	if (_content.AddSampler(_device.Get(), "SS_Shadow", D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER_MIN_MAG_MIP_POINT) == CONTENT_NULL)
	{
		ErrMsg("Failed to add shadow sampler!");
		return false;
	}


	const std::vector<Semantic> fallbackInputLayout{
		{ "POSITION",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "NORMAL",		DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TANGENT",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TEXCOORD",	DXGI_FORMAT_R32G32_FLOAT	}
	};

	if (_content.AddInputLayout(_device.Get(), "IL_Fallback", fallbackInputLayout, _content.GetShaderID("VS_Geometry")) == CONTENT_NULL)
	{
		ErrMsg("Failed to add IL_Fallback!");
		return false;
	}

	const std::vector<Semantic> debugDrawInputLayout{
		{ "POSITION",	DXGI_FORMAT_R32G32B32_FLOAT		},
		{ "SIZE",		DXGI_FORMAT_R32_FLOAT			},
		{ "COLOR",		DXGI_FORMAT_R32G32B32A32_FLOAT	}
	};

	if (_content.AddInputLayout(_device.Get(), "IL_DebugDraw", debugDrawInputLayout, _content.GetShaderID("VS_DebugDraw")) == CONTENT_NULL)
	{
		ErrMsg("Failed to add IL_DebugDraw!");
		return false;
	}

	return true;
}

bool Game::Setup(Time& time, const UINT width, const UINT height, Window *window)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(866842434, "Game Setup");
#endif

	_window = window;

	if (!_graphics.Setup(width, height, window, *_device.ReleaseAndGetAddressOf(), *_immediateContext.ReleaseAndGetAddressOf(), &_content))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}

	std::vector<std::string> meshNames = {
		"Error",
		"Fallback",
		"Cube",
		"Character1",
		"Plane",
		"Sphere",
		"IsoSphereSmooth",
		"WireframeCube",
		"WireframeSphere",
		"WireframeCylinder",
		"WireframeCapsule",
		"HeightMapExample",
		"TranslationGizmo",
		"RotationGizmo",
		"ScalingGizmo",
		"Maxwell",
		"Whiskers",
		"GraniteRock",
		"DecimatedMan",
		"Cornell",
		"Support_Beam",
		"Camera",
		"FlashlightBody",
		"FlashlightLever",
		"Cave_Wall",
	};

	std::string line;

#ifdef COMPILE_CONTENT
	const char *meshPath = "Content/Meshes/_meshNames.txt";
	std::ifstream fileStream(meshPath);
	if (!fileStream)
	{
		ErrMsg("Could not load file for meshes");
		return false;
	}

	while (std::getline(fileStream, line))
	{
		auto comment = line.find("#");
		if (comment != std::string::npos)
			line = line.substr(0, comment);

		if (line.empty())
			continue;

		// See if the mesh is already in the list
		auto it = std::find(meshNames.begin(), meshNames.end(), line);
		if (it != meshNames.end())
			continue;

		meshNames.push_back(line);
	}
	fileStream.close();
#endif


	std::vector<TextureData> textureNames = {
		// Opaque
		{ "Error",					false	},
		{ "Fallback",				false	},
		{ "texture1",				false	},
		{ "Character_Texture",		false	},
		{ "Sphere",					false	},
		{ "White",					false	},
		{ "Black",					false	},
		{ "Ambient",				false	},
		{ "AmbientBright",			false	},
		{ "Red",					false	},
		{ "Green",					false	},
		{ "Blue",					false	},
		{ "Fade",					false	},
		{ "Metal",					false	},
		{ "TransformGizmo",			false	},
		{ "Maxwell",				false	},
	    { "Gray",					false	},
		{ "Cornell",				false	},
		{ "Support_Beam",			false	},
		{ "Camera",					false	},
		{ "FlashlightBody",			false	},
		{ "FlashlightLever",		false	},
		{ "Cave_Wall_Texture",		false	},
		{ "Button_Start_Texture",	false	},
		{ "Button_Continue_Texture",false	},
		{ "Button_Load_Texture",	false	},
		{ "Button_Save_Texture",	false	},
		{ "Button_NewSave_Texture",	false	},
		{ "Button_Credits_Texture",	false	},
		{ "Button_Exit_Texture",	false	},
		{ "plastered_stone",		false	},
		{ "SoundEmitter",			false	},
		{ "HighlightColor",			false	},

		// Lightmaps
		{ "Cornell_Light",			false	},
	 
		// Transparent
		{ "Transparent",			true	},
		{ "Transparent2",			true	},
		{ "Flare",					true	},
		{ "Flare_Red",				true	},
		{ "Flare_Green",			true	},
		{ "Flare_Blue",				true	},
		{ "Particle",				true	},
		{ "Whiskers",				true	},
		{ "Noise",					true	},
		{ "Flare_Golden",			true	},
		{ "OldLamp_Glass",			true	},
	};

	const char *texturePath = "Content/Textures/_textures.txt";

	std::ifstream texStream(texturePath);
	if (texStream)
	{
		while (std::getline(texStream, line))
		{
			auto comment = line.find("#");
			if (comment != std::string::npos)
				line = line.substr(0, comment);

			if (line.empty())
				continue;

			// Check if the texture is already in the list
			bool found = false;
			UINT textureCount = static_cast<UINT>(textureNames.size());
			for (UINT i = 0; i < textureCount; i++)
			{
				if (textureNames.at(i).name == line)
				{
					found = true;
					break;
				}
			}

			if (found)
				continue;

			textureNames.insert(textureNames.begin(), { line, false });
		}
		texStream.close();
	}

	std::vector<TextureMapData> textureMapNames = {
		{ TextureType::NORMAL,		"Default_Normal",					"Default_Normal"					},
		{ TextureType::NORMAL,		"Metal_Normal",						"Metal_Normal"						},
		{ TextureType::NORMAL,		"Support_Beam_Normal",				"Support_Beam_Normal"				},
		{ TextureType::NORMAL,		"Camera_Normal",					"Camera_Normal"						},
		{ TextureType::NORMAL,		"FlashlightBody_Normal",			"FlashlightBody_Normal"				},
		{ TextureType::NORMAL,		"FlashlightLever_Normal",			"FlashlightLever_Normal"			},
		{ TextureType::NORMAL,		"Cave_Wall_Normal",					"Cave_Wall_Normal"					},

		{ TextureType::SPECULAR,	"Default_Specular",					"AmbientBright"						},
		{ TextureType::SPECULAR,	"Black_Specular",					"Black"								},
		{ TextureType::SPECULAR,	"DarkGray_Specular",				"DarkGray"							},
		{ TextureType::SPECULAR,	"Gray_Specular",					"Gray"								},
		{ TextureType::SPECULAR,	"White_Specular",					"White"								},
		{ TextureType::SPECULAR,	"Fade_Specular",					"Fade"								},
		{ TextureType::SPECULAR,	"Red_Specular",						"Red"								},
		{ TextureType::SPECULAR,	"Green_Specular",					"Green"								},
		{ TextureType::SPECULAR,	"Blue_Specular",					"Blue"								},
		{ TextureType::SPECULAR,	"Metal_Specular",					"Metal_Specular"					},
		{ TextureType::SPECULAR,	"Support_Beam_Specular",			"Support_Beam_Specular"				},
		{ TextureType::SPECULAR,	"Camera_Specular",					"Camera_Specular"					},
		{ TextureType::SPECULAR,	"FlashlightBody_Specular",			"FlashlightBody_Specular"			},
		{ TextureType::SPECULAR,	"FlashlightLever_Specular",			"FlashlightLever_Specular"			},

		{ TextureType::GLOSS,		"Default_Glossiness",				"Gray"								},
		{ TextureType::GLOSS,		"White_Glossiness",					"White"								},
		{ TextureType::GLOSS,		"Black_Glossiness",					"Black"								},
		{ TextureType::GLOSS,		"DarkGray_Glossiness",				"DarkGray"							},
		{ TextureType::GLOSS,		"Fade_Glossiness",					"Fade"								},
		{ TextureType::GLOSS,		"FlashlightBody_Glossiness",		"FlashlightBody_Glossiness"			},
		{ TextureType::GLOSS,		"FlashlightLever_Glossiness",		"FlashlightLever_Glossiness"		},

		{ TextureType::HEIGHT,		"Default_Height",					"Black"								},
		{ TextureType::HEIGHT,		"Metal_Height",						"Metal_Height"						},

		{ TextureType::OCCLUSION,	"Default_Occlusion",				"White"								},
		{ TextureType::OCCLUSION,	"Support_Beam_Occlusion",			"Support_Beam_Occlusion"			},
		{ TextureType::OCCLUSION,	"Camera_Occlusion",					"Camera_Occlusion"					},
	};

	const char *textureMapPath = "Content/Textures/_textureMaps.txt";

	std::ifstream texMapStream(textureMapPath);
	if (texMapStream)
	{
		while (std::getline(texMapStream, line))
		{
			auto comment = line.find("#");
			if (comment != std::string::npos)
				line = line.substr(0, comment);

			if (line.empty())
				continue;

			// Check if the texture map is already in the list
			bool found = false;
			UINT texMapCount = static_cast<UINT>(textureMapNames.size());
			for (UINT i = 0; i < texMapCount; i++)
			{
				if (textureMapNames.at(i).name == line)
				{
					found = true;
					break;
				}
			}

			if (found)
				continue;

			TextureType type = _content.GetTextureMapType(line);
			std::string suffix = _content.GetTextureMapTypeSuffix(type);

			UINT start = 0, end = 0;

			bool foundFirst = false;
			for (UINT i = 0; i < texMapCount; i++)
			{
				if (textureMapNames.at(i).name.ends_with(suffix))
				{
					if (foundFirst)
					{
						end = i;
					}
					else
					{
						start = end = i;
						foundFirst = true;
					}
				}
				else
				{
					if (foundFirst)
						break;
				}
			}

			if (!foundFirst)
			{
				end = texMapCount;
			}

			TextureMapData newTexMap = { type, line, line };

			if (!foundFirst || end >= texMapCount - 1)
				textureMapNames.push_back(newTexMap);
			else
				textureMapNames.insert(textureMapNames.begin() + (end - 1), newTexMap);
		}
		texMapStream.close();
	}


	std::vector<ShaderData> shaderNames = {
		{ ShaderType::VERTEX_SHADER,		"VS_Geometry",				"VS_Geometry"				},
		{ ShaderType::VERTEX_SHADER,		"VS_GeometryDistortion",	"VS_GeometryDistortion"		},
		{ ShaderType::VERTEX_SHADER,		"VS_Depth",					"VS_Depth"					},
		{ ShaderType::VERTEX_SHADER,		"VS_DepthDistortion",		"VS_DepthDistortion"		},
		{ ShaderType::VERTEX_SHADER,		"VS_Particle",				"VS_Particle"				},
		{ ShaderType::VERTEX_SHADER,		"VS_DebugDraw",				"VS_DebugDraw"				},

		{ ShaderType::HULL_SHADER,			"HS_LOD",					"HS_LOD"					},
		{ ShaderType::DOMAIN_SHADER,		"DS_LOD",					"DS_LOD"					},

		{ ShaderType::GEOMETRY_SHADER,		"GS_Billboard",				"GS_Billboard"				},
		{ ShaderType::GEOMETRY_SHADER,		"GS_DebugLine",				"GS_DebugLine"				},

		{ ShaderType::PIXEL_SHADER,			"PS_Geometry",				"PS_Geometry"				},
		{ ShaderType::PIXEL_SHADER,			"PS_TriPlanar",				"PS_TriPlanar"				},
		{ ShaderType::PIXEL_SHADER,			"PS_Transparent",			"PS_Transparent"			},
		{ ShaderType::PIXEL_SHADER,			"PS_Particle",				"PS_Particle"				},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugDraw",				"PS_DebugDraw"				},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewPosition",		"PS_DebugViewPosition"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewNormal",		"PS_DebugViewNormal"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewAmbient",		"PS_DebugViewAmbient"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewDiffuse",		"PS_DebugViewDiffuse"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewDepth",		"PS_DebugViewDepth"			},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewLighting",		"PS_DebugViewLighting"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewShadow",		"PS_DebugViewShadow"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewSpecular",		"PS_DebugViewSpecular"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewSpecStr",		"PS_DebugViewSpecStr"		},
		{ ShaderType::PIXEL_SHADER,			"PS_DebugViewUVCoords",		"PS_DebugViewUVCoords"		},

		{ ShaderType::COMPUTE_SHADER,		"CS_Particle",				"CS_Particle"				},
		{ ShaderType::COMPUTE_SHADER,		"CS_BlurHorizontalFX",		"CS_BlurHorizontalFX"		},
		{ ShaderType::COMPUTE_SHADER,		"CS_BlurVerticalFX",		"CS_BlurVerticalFX"			},
		{ ShaderType::COMPUTE_SHADER,		"CS_FogFX",					"CS_FogFX"					},
		{ ShaderType::COMPUTE_SHADER,		"CS_CombineFX",				"CS_CombineFX"				},
	};

	// Height Maps should be placed in the Texture folder
	//  { Reference Name,					File name						},
	std::vector<HeightMapData> heightMapNames = {
		{ "CaveHeightmap",					"CaveHeightmap"},
		{ "CaveRoofHeightmap",				"CaveRoofHeightmap"},
		{ "CaveWallsHeightmap",				"CaveWallsHeightmap"},
		{ "ExampleHeightMap",				"ExampleHeightMap"},
		{ "ExampleHeightMap2",				"ExampleHeightMap2"},
	};

#ifdef FAST_LOAD_MODE
	// Remove all but the most important content

	for (UINT i = 0; i < meshNames.size(); i++)
	{
		if (meshNames[i].find("Fallback") == std::string::npos &&
			meshNames[i].find("Error") == std::string::npos &&
			meshNames[i].find("Sphere") == std::string::npos &&
			meshNames[i].find("cave_map") == std::string::npos)
			meshNames.erase(meshNames.begin() + i--);
	}

	for (UINT i = 0; i < textureNames.size(); i++)
	{
		if (textureNames[i].name.find("Fallback") == std::string::npos && 
			textureNames[i].name.find("Error") == std::string::npos &&
			textureNames[i].name.find("Red") == std::string::npos &&
			textureNames[i].name.find("Green") == std::string::npos &&
			textureNames[i].name.find("Blue") == std::string::npos &&
			textureNames[i].name.find("Black") == std::string::npos &&
			textureNames[i].name.find("White") == std::string::npos &&
			textureNames[i].name.find("CaveSystem") == std::string::npos)
			textureNames.erase(textureNames.begin() + i--);
	}

	for (UINT i = 0; i < textureMapNames.size(); i++)
	{
		if (textureMapNames[i].name.find("Default") == std::string::npos)
			textureMapNames.erase(textureMapNames.begin() + i--);
	}

	heightMapNames.erase(heightMapNames.begin() + 3, heightMapNames.end());
#endif

#ifdef COMPILE_CONTENT
	if (!CompileContent("Content\\CompiledContent", meshNames))
	{
		ErrMsg("Failed to compile content!");
		return false;
	}
#endif

	time.TakeSnapshot("LoadContent");
	if (!LoadContent("Content\\CompiledContent", textureNames, textureMapNames, shaderNames, heightMapNames))
	{
		ErrMsg("Failed to load game content!");
		return false;
	}
	time.TakeSnapshot("LoadContent");

	return true;
}

bool Game::ActiveSceneIsValid()
{
	if (_activeSceneIndex < _scenes.size())
	{
		if (_scenes.at(_activeSceneIndex) == nullptr)
		{
			return false;
		}

		if (!_scenes.at(_activeSceneIndex)->IsInitialized())
		{
			return false;
		}
	}

	return true;
}

bool Game::AddScene(Scene *newScene, const bool setActive, const std::string &saveFile)
{
	if (newScene == nullptr)
		return false;

	if (!newScene->IsInitialized())
	{
		if (saveFile == "MenuSave")
		{
			if (!newScene->InitializeMenu(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize menu scene!");
				return false;
			}
		}
		else if (saveFile == "MapSave")
		{
			if (!newScene->InitializeGame(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize scene!");
				return false;
			}
		}
		else if (saveFile == "CreditSave")
		{
			if (!newScene->InitializeCred(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize scene!");
				return false;
			}
		}
		else if (saveFile == "StartCutsceneSave")
		{
			if (!newScene->InitializeStartCutscene(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize scene!");
				return false;
			}
		}
		else
		{
			if (!newScene->Initialize(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize scene!");
				return false;
			}
		}

		_scenes.push_back(std::move(newScene));
	}

	if (setActive)
	{
		_activeSceneIndex = UINT(_scenes.size() - 1);
	}

	return true;
}

bool Game::SetScene(const UINT sceneIndex)
{
	if (sceneIndex >= _scenes.size())	// invalid index
	{
		ErrMsg("Invalid scene index");
		return false;
	}

	_scenes[_activeSceneIndex]->SuspendSceneSound();

	_activeSceneIndex = sceneIndex;

	_scenes[_activeSceneIndex]->ResumeSceneSound();

	Scene *scene = _scenes.at(_activeSceneIndex);
	std::string saveFile = scene->GetSaveFile();
	if (!scene->IsInitialized())
	{
		if (saveFile == "MenuSave")
		{
			if (!scene->InitializeMenu(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize menu scene!");
				return false;
			}
		}
		else if (saveFile == "MapSave")
		{
			if (!scene->InitializeGame(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize scene!");
				return false;
			}
		}
		else
		{
			if (!scene->Initialize(_device.Get(), _immediateContext.Get(), this, &_content, &_graphics, _gameVolume, saveFile))
			{
				ErrMsg("Failed to initialize scene!");
				return false;
			}
		}
	}

	return true;
}

Scene *Game::GetScene(const UINT sceneIndex)
{
	return _scenes.at(sceneIndex);
}
const std::vector<Scene *> *Game::GetScenes() const
{
	return &_scenes;
}
UINT Game::GetActiveScene() const
{
	return _activeSceneIndex;
}

Graphics *Game::GetGraphics()
{
	return &_graphics;
}

Window *Game::GetWindow()
{
	return _window;
}

bool Game::Update(Time& time, const Input& input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(386745480, "Game Update");
#endif

	// Update
	time.TakeSnapshot("SceneUpdateTime");
	if (ActiveSceneIsValid())
	{
		if (!_scenes.at(_activeSceneIndex)->Update(time, input))
		{
			ErrMsg("Failed to update scene!");
			return false;
		}
	}
	time.TakeSnapshot("SceneUpdateTime");

	// Fixed update
	static bool firstFixedUpdate = true;
	_tickTimer += time.deltaTime;
	while (_tickTimer >= _tickRate)
	{
		time.TakeSnapshot("SceneFixedUpdateTime");
		_tickTimer -= _tickRate;
		if (firstFixedUpdate)
		{
			firstFixedUpdate = false;
			_tickTimer = 0.0f;
		}

		if (ActiveSceneIsValid())
		{
			if (!_scenes.at(_activeSceneIndex)->FixedUpdate(_tickRate, input))
			{
				ErrMsg("Failed to update scene at fixed step!");
				return false;
			}
		}
		time.TakeSnapshot("SceneFixedUpdateTime");

#ifdef DEBUG_BUILD
		if (_tickTimer >= _tickRate * 4.0f)
			_tickTimer = _tickRate * 4.0f;
#endif
	}

	// Late update
	time.TakeSnapshot("SceneLateUpdateTime");
	if (ActiveSceneIsValid())
	{
		if (!_scenes.at(_activeSceneIndex)->LateUpdate(time, input))
		{
			ErrMsg("Failed to late update scene!");
			return false;
		}
	}
	time.TakeSnapshot("SceneLateUpdateTime");

	return true;
}

bool Game::Render(Time& time, const Input& input)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(582426884, "Game Render");
#endif

	if (!_graphics.BeginSceneRender())
	{
		ErrMsg("Failed to begin rendering!");
		return false;
	}

	time.TakeSnapshot("SceneRenderTime");
	/// v==========================================v ///
	/// v        Render scene here...              v ///
	/// v==========================================v ///

	if (ActiveSceneIsValid())
		if (!_scenes.at(_activeSceneIndex)->Render(time, input))
		{
			ErrMsg("Failed to render scene!");
			return false;
		}

	/// ^==========================================^ ///
	/// ^        Render scene here...              ^ ///
	/// ^==========================================^ ///
	time.TakeSnapshot("SceneRenderTime");

	if (!_graphics.EndSceneRender(time))
	{
		ErrMsg("Failed to end rendering!");
		return false;
	}

#ifdef USE_IMGUI
	if (!_graphics.BeginUIRender())
	{
		ErrMsg("Failed to begin UI rendering!");
		return false;
	}

	static float imGuiFontScale = 1.0f;
	ImGui::SetWindowFontScale(imGuiFontScale);

	/// v==========================================v ///
	/// v        Render UI here...                 v ///
	/// v==========================================v ///

	// For an in-depth manual of ImGui features and their usages see:
	// https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html

	if (ImGui::CollapsingHeader("General"))
	{
		ImGui::SliderFloat("Font Scale", &imGuiFontScale, 0.25f, 8.0f);

		if (ImGui::Button("Reset Font Scale"))
			imGuiFontScale = 1.0f;

		ImGui::SetWindowFontScale(imGuiFontScale);

		ImGui::DragFloat("Volume", &_gameVolume, 0.05f, 0.0f);

		_scenes[_activeSceneIndex]->SetSceneVolume(_gameVolume);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Performance"))
	{
		if (ImGui::TreeNode("FPS"))
		{
			constexpr size_t FPS_BUF_SIZE = 128;
			static size_t usedBufSize = FPS_BUF_SIZE;

			static float fpsBuf[FPS_BUF_SIZE]{};
			static size_t fpsBufIndex = 0;

			float currFps = 1.0f / time.deltaTime;
			fpsBuf[fpsBufIndex] = currFps;

			float avgFps = 0.0f;
			float dropFps = FLT_MAX;
			for (size_t i = 0; i < usedBufSize; i++)
			{
				avgFps += fpsBuf[i];

				if (dropFps > fpsBuf[i])
					dropFps = fpsBuf[i];
			}
			avgFps /= usedBufSize;

			static float minFPS = FLT_MAX;
			if (minFPS > currFps)
				minFPS = currFps;

			static UINT rebaseBufferSizeTimer = 0;
			if (++rebaseBufferSizeTimer >= 30)
			{
				size_t prevSize = usedBufSize;

				rebaseBufferSizeTimer = 0;
				if (avgFps > 240.f && usedBufSize != FPS_BUF_SIZE)
					usedBufSize = FPS_BUF_SIZE;
				else if (avgFps > 90.f && usedBufSize != (FPS_BUF_SIZE / 2))
					usedBufSize = FPS_BUF_SIZE / 2;
				else if (avgFps > 30.f && usedBufSize != (FPS_BUF_SIZE / 4))
					usedBufSize = FPS_BUF_SIZE / 4;
				else if (avgFps > 10.f && usedBufSize != (FPS_BUF_SIZE / 8))
					usedBufSize = FPS_BUF_SIZE / 8;

				if (prevSize != usedBufSize)
				{
					for (size_t i = 0; i < FPS_BUF_SIZE; i++)
						fpsBuf[i] = avgFps;
				}
			}
			
			(++fpsBufIndex) %= usedBufSize;

			char fps[8]{};
			snprintf(fps, sizeof(fps), "%.2f", currFps);
			ImGui::Text(std::format("FPS: {}", fps).c_str());

			snprintf(fps, sizeof(fps), "%.2f", avgFps);
			ImGui::Text(std::format("Avg: {}", fps).c_str());

			snprintf(fps, sizeof(fps), "%.2f", dropFps);
			ImGui::Text(std::format("Drop: {}", fps).c_str());

			snprintf(fps, sizeof(fps), "%.2f", minFPS);
			ImGui::Text(std::format("Min: {}", fps).c_str());

			static bool countLongAvg = false;
			static bool hasLongAvg = false;
			static float longAvgAccumulation = 0.0f;
			static int longAvgCount = 0;

			ImGui::Text("Long Exposure Avg: ");
			ImGui::SameLine();
			if (!countLongAvg)
			{
				if (ImGui::SmallButton("Start"))
				{
					countLongAvg = true;

					hasLongAvg = true;
					longAvgAccumulation = currFps;
					longAvgCount = 1;
				}
			}
			else
			{
				if (ImGui::SmallButton("Stop"))
					countLongAvg = false;

				longAvgAccumulation += currFps;
				longAvgCount++;
			}

			if (hasLongAvg)
			{
				ImGui::SameLine();
				ImGui::Text(std::format("Iter: {}", longAvgCount).c_str());

				if (longAvgCount > 0)
					ImGui::Text(std::format("Result: {}", (longAvgAccumulation / (float)longAvgCount)).c_str());
				else
					ImGui::Text("Result: NaN");
			}

			if (ImGui::Button("Reset"))
			{
				minFPS = 1.0f / time.deltaTime;

				for (size_t i = 0; i < FPS_BUF_SIZE; i++)
					fpsBuf[i] = 0.0f;

				countLongAvg = false;
				hasLongAvg = false;
				longAvgAccumulation = 0.0f;
				longAvgCount = 0;
			}
			ImGui::TreePop();
		}
		
		ImGui::PushID("Frame Time");
		if (ImGui::TreeNode("Frame Time"))
		{
			char timeStr[32]{};

			snprintf(timeStr, sizeof(timeStr), "%.6f", time.deltaTime);
			ImGui::Text(std::format("{} Frame", timeStr).c_str());

			ImGui::Spacing();

			if (ImGui::TreeNode("Scene"))
			{	
				if (ImGui::TreeNode("Update"))
				{	
					snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("SceneUpdateTime"));
					ImGui::Text(std::format("{} Scene Update", timeStr).c_str());
					
					snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("SceneLateUpdateTime"));
					ImGui::Text(std::format("{} Scene Late Update", timeStr).c_str());

					static float fixedUpdateTime = -1.0f;
					time.TryCompareSnapshots("SceneFixedUpdateTime", &fixedUpdateTime);
					snprintf(timeStr, sizeof(timeStr), "%.6f", fixedUpdateTime);
					ImGui::Text(std::format("{} Scene Fixed Update", timeStr).c_str());

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Render"))
				{	
					snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("SceneRenderTime"));
					ImGui::Text(std::format("{} Scene Render", timeStr).c_str());

					snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("CullingTotal"));
					ImGui::Text(std::format("{} Culling Total", timeStr).c_str());
			
					snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("CullingSetup"));
					ImGui::Text(std::format("{} Culling Setup", timeStr).c_str());

					snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("CullingCameras"));
					ImGui::Text(std::format("{} Culling Cameras", timeStr).c_str());
			
					snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("CullingCameraCubes"));
					ImGui::Text(std::format("{} Culling Camera Cubes", timeStr).c_str());

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
			
			ImGui::Spacing();

			if (ImGui::TreeNode("Graphics"))
			{
				ImGui::TreePop();
			}

			ImGui::Spacing();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Collisions"))
		{
			char timeStr[32]{};

			snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("CollisionChecks"));
			ImGui::Text(std::format("{} Collision Checks Total", timeStr).c_str());

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Graphics"))
	{
		if (!_graphics.RenderUI(time))
		{
			ErrMsg("Failed to render graphics UI!");
			return false;
		}
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Scene"))
	{
		if (ActiveSceneIsValid())
			if (!_scenes.at(_activeSceneIndex)->RenderUI())
			{
				ErrMsg("Failed to render scene UI!");
				return false;
			}
	}
 
	/// ^==========================================^ ///
	/// ^        Render UI here...                 ^ ///
	/// ^==========================================^ ///

	if (!_graphics.EndUIRender())
	{
		ErrMsg("Failed to end UI rendering!");
		return false;
	}
#endif

	if (!_graphics.EndFrame())
	{
		ErrMsg("Failed to end frame!");
		return false;
	}

	return true;
}