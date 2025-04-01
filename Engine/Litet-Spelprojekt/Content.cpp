#include "stdafx.h"
#include "Content.h"
#include "ContentLoader.h"

#include "ErrMsg.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

Content::Content()
{
	_materialVec.reserve(512);
}
Content::~Content()
{
	Shutdown();
}

void Content::Shutdown()
{
	if (_hasShutDown)
		return;

	for (const Mesh *mesh : _meshes)
		delete mesh;

	for (const Shader *shader : _shaders)
		delete shader;

	for (const Texture *texture : _textures)
		delete texture;

	for (const TextureMap *textureMap : _textureMaps)
		delete textureMap;

	for (const HeightTexture *heightTexture : _heightTextures)
		delete heightTexture;

	for (const Sampler *sampler : _samplers)
		delete sampler;

	for (const InputLayout *inputLayout : _inputLayouts)
		delete inputLayout;

	for (Material *material : _materialVec)
		delete material;

	_hasShutDown = true;
}


CompiledData Content::GetMeshData(const std::string &name, const char *path) const
{
	CompiledData data{};

	MeshData meshData{};
	if (!LoadMeshFromFile(path, meshData))
	{
		ErrMsg("Failed to load mesh from file!");
		return data;
	}

	std::vector<char> dataChars;
	meshData.Compile(dataChars);

	data.size = dataChars.size();
	data.data = new char[data.size];
	std::memcpy(data.data, dataChars.data(), data.size);
	return data;
}
CompiledData Content::GetShaderData(const std::string &name, const char *path, ShaderType shaderType) const
{
	CompiledData data{};

	std::string shaderFileData;
	std::ifstream reader;

	reader.open(path, std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		ErrMsg("Failed to open shader file!");
		return data;
	}

	reader.seekg(0, std::ios::end);
	shaderFileData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderFileData.assign(
		std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>()
	);

	shaderFileData.clear();
	reader.close();

	std::vector<char> dataChars;
	data.size = shaderFileData.length();
	data.data = new char[data.size];
	std::memcpy(data.data, shaderFileData.c_str(), data.size);
	return data;
}
CompiledData Content::GetTextureData(const std::string &name, const char *path, bool transparent) const
{
	CompiledData data{};
	return data;
}
CompiledData Content::GetTextureMapData(const std::string &name, const char *path, TextureType mapType) const
{
	CompiledData data{};
	return data;
}
CompiledData Content::GetHeightMapData(const std::string &name, const char *path) const
{
	CompiledData data{};
	return data;
}



UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const MeshData &meshData)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(674126468, std::format("Add Mesh '{}'", name).c_str());
#endif

	const UINT id = static_cast<UINT>(_meshes.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_meshes.at(i)->name == name)
			return i;
	}

	bool failed = false;
	Mesh *addedMesh = new Mesh(name, id);
#pragma warning(disable: 6993)
#pragma omp critical
	{
		if (!addedMesh->data.Initialize(device, meshData))
		{
			ErrMsg("Failed to initialize added mesh!");
			delete addedMesh;
			failed = true;
		}
		_meshes.push_back(addedMesh);
	}
#pragma warning(default: 6993)

	return failed ? CONTENT_NULL : id;
}
UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const char *path)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(764178624, std::format("Add Mesh '{}'", name).c_str());
#endif

	const UINT id = static_cast<UINT>(_meshes.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_meshes.at(i)->name == name)
			return i;
	}

	MeshData meshData = { };
	if (!LoadMeshFromFile(path, meshData))
	{
		ErrMsg("Failed to load mesh from file!");
		return CONTENT_NULL;
	}

	bool failed = false;
	Mesh *addedMesh = new Mesh(name, id);
#pragma warning(disable: 6993)
#pragma omp critical
	{
		if (!addedMesh->data.Initialize(device, meshData))
		{
			ErrMsg("Failed to initialize added mesh!");
			delete addedMesh;
			failed = true;
		}
		_meshes.push_back(addedMesh);
	}
#pragma warning(default: 6993)

	return failed ? CONTENT_NULL : id;
}

UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const void *dataPtr, const size_t dataSize)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(714866616, std::format("Add Shader '{}'", name).c_str());
#endif

	const UINT id = static_cast<UINT>(_shaders.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i)->name == name)
			return i;
	}

	bool failed = false;
#pragma warning(disable: 6993)
#pragma omp critical
	{
		Shader *addedShader = new Shader(name, id);
		if (!addedShader->data.Initialize(device, shaderType, dataPtr, dataSize))
		{
			ErrMsg("Failed to initialize added shader!");
			delete addedShader;
			failed = true;
		}
		_shaders.push_back(addedShader);
	}
#pragma warning(default: 6993)

	return failed ? CONTENT_NULL : id;
}
UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const char *path)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(681748532, std::format("Add Shader '{}'", name).c_str());
#endif

	const UINT id = static_cast<UINT>(_shaders.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i)->name == name)
			return i;
	}

	bool failed = false;
#pragma warning(disable: 6993)
#pragma omp critical
	{
		Shader *addedShader = new Shader(name, id);
		if (!addedShader->data.Initialize(device, shaderType, path))
		{
			ErrMsg("Failed to initialize added shader!");
			delete addedShader;
			failed = true;
		}
		_shaders.push_back(addedShader);
	}
#pragma warning(default: 6993)

	return failed ? CONTENT_NULL : id;
}

UINT Content::AddTexture(ID3D11Device *device, ID3D11DeviceContext *context, const std::string &name, const char *path, bool transparent)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(981462455, std::format("Add Texture '{}'", name).c_str());
#endif

	const UINT id = static_cast<UINT>(_textures.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_textures.at(i)->name == name)
			return i;
	}

	UINT width, height;
	std::vector<unsigned char> texData;

	if (!LoadTextureFromFile(path, width, height, texData))
	{
		ErrMsg("Failed to load texture from file!");
		return CONTENT_NULL;
	}

	bool failed = false;
#pragma warning(disable: 6993)
#pragma omp critical
	{
		Texture *addedTexture = new Texture(name, (std::string)path, id, transparent);
		if (!addedTexture->data.Initialize(device, context, width, height, texData.data(), true))
		{
			ErrMsg("Failed to initialize added texture!");
			delete addedTexture;
			failed = true;
		}
		_textures.push_back(addedTexture);
	}
#pragma warning(default: 6993)

	return failed ? CONTENT_NULL : id;
}
UINT Content::AddTextureMap(ID3D11Device *device, ID3D11DeviceContext *context, const std::string &name, const TextureType mapType, const char *path)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(86712688, std::format("Add Texture Map '{}'", name).c_str());
#endif

	bool autoMipmaps = true;
	if (context == nullptr)
		autoMipmaps = false;

	const UINT id = static_cast<UINT>(_textureMaps.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_textureMaps.at(i)->name == name)
			return i;
	}

	UINT width, height;
	std::vector<unsigned char> texData, texMapData;

	if (!LoadTextureFromFile(path, width, height, texData))
	{
		ErrMsg("Failed to load texture map from file!");
		return CONTENT_NULL;
	}

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = (autoMipmaps) ? 0u : 1u;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.SysMemSlicePitch = 0;

	switch (mapType)
	{
	case TextureType::HEIGHT:
	case TextureType::OCCLUSION:
	case TextureType::GLOSS:
		textureDesc.Format = DXGI_FORMAT_R8_UNORM;
		srData.SysMemPitch = width * sizeof(unsigned char);

		for (size_t i = 0; i < texData.size(); i += 4)
			texMapData.push_back(texData.at(i));
		break;

	default:
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srData.SysMemPitch = width * sizeof(unsigned char) * 4;

		texMapData.insert(texMapData.end(), texData.begin(), texData.end());
		break;
	}

	srData.pSysMem = texMapData.data();

	if (autoMipmaps)
	{
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	bool failed = false;
#pragma omp critical
	{
		TextureMap *addedTextureMap = new TextureMap(name, (std::string)path, id);
		if (!addedTextureMap->data.Initialize(device, context, textureDesc, &srData, autoMipmaps))
		{
			ErrMsg("Failed to initialize added texture map!");
			delete addedTextureMap;
			failed = true;
		}
		_textureMaps.push_back(addedTextureMap);
	}

	return failed ? CONTENT_NULL : id;
}

UINT Content::AddSampler(ID3D11Device *device, const std::string &name, 
	D3D11_TEXTURE_ADDRESS_MODE adressMode, D3D11_FILTER filter)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(2127824154, std::format("Add Sampler '{}'", name).c_str());
#endif

	const UINT id = static_cast<UINT>(_samplers.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_samplers.at(i)->name == name)
			return i;
	}

	Sampler* addedSampler = new Sampler(name, id);
	if (!addedSampler->data.Initialize(device, adressMode, filter))
	{
		ErrMsg("Failed to initialize added texture!");
		delete addedSampler;
		return CONTENT_NULL;
	}
	_samplers.push_back(addedSampler);

	return id;
}

UINT Content::AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics,
	const void *vsByteData, const size_t vsByteSize)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(386418685, std::format("Add Input Layout '{}'", name).c_str());
#endif

	const UINT id = static_cast<UINT>(_inputLayouts.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_inputLayouts.at(i)->name == name)
			return i;
	}

	InputLayout* addedInputLayout = new InputLayout(name, id);
	for (const Semantic& semantic : semantics)
	{
		if (!addedInputLayout->data.AddInputElement(semantic))
		{
			ErrMsg(std::format("Failed to add element \"{}\" to input layout!", semantic.name));
			delete addedInputLayout;
			return CONTENT_NULL;
		}
	}

	if (!addedInputLayout->data.FinalizeInputLayout(device, vsByteData, vsByteSize))
	{
		ErrMsg("Failed to finalize added input layout!");
		delete addedInputLayout;
		return CONTENT_NULL;
	}
	_inputLayouts.push_back(addedInputLayout);

	return id;
}
UINT Content::AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics, const UINT vShaderID)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(576146856, std::format("Add Input Layout '{}'", name).c_str());
#endif

	if (vShaderID == CONTENT_NULL)
	{
		ErrMsg("Failed to get vertex shader byte code, shader ID was CONTENT_NULL!");
		return CONTENT_NULL;
	}

	const ShaderD3D11* vShader = GetShader(vShaderID);
	if (vShader == nullptr)
	{
		ErrMsg("Failed to get vertex shader byte code, shader ID returned nullptr!");
		return CONTENT_NULL;
	}

	if (vShader->GetShaderType() != ShaderType::VERTEX_SHADER)
	{
		ErrMsg(std::format("Failed to get vertex shader byte code, shader ID returned invalid type ({})!", (UINT)vShader->GetShaderType()));
		return CONTENT_NULL;
	}

	return AddInputLayout(device, name, semantics, vShader->GetShaderByteData(), vShader->GetShaderByteSize());
}

bool BlurHeightMap(const HeightMap &hm, const char* path)
{
	/*
	std::vector<float> originalData(hm.GetHeightValues());  // Read-only in each iteration
	std::vector<float> temp(originalData); // Buffer for updates

	const UINT w = hm.GetWidth(), h = hm.GetHeight();
	const float threshold = 0.97f;

	for (UINT y = 0; y < h; y++)
	{
		for (UINT x = 0; x < w; x++)
		{
			if (originalData[y * w + x] >= threshold)
			{
				temp[y * w + x] = 0;
			}
		}
	}

	// Write final blurred heightmap
	if (!WriteTextureToFile(path, w, h, temp, 1, false))
	{
		ErrMsg("Failed to blur height map");
		return false;
	}
	*/

	/*
	std::vector<float> originalData(hm.GetHeightValues());  // Read-only in each iteration
	std::vector<float> temp(originalData); // Buffer for updates

	const UINT w = hm.GetWidth(), h = hm.GetHeight();
	const float threshold = 0.6f;

	for (UINT i = 0; i < 20; i++) 
	{
		for (UINT y = 0; y < h; y++)
		{
			for (UINT x = 0; x < w; x++)
			{
				if (originalData[y * w + x] >= threshold)
				{
					float avg = INFINITY;
					int avgCount = 0;

					for (int dy = -1; dy <= 1; dy++)
						for (int dx = -1; dx <= 1; dx++)
						{
							int nx = x + dx, 
								ny = y + dy;
							if (dy != dx && nx >= 0 && nx < (int)w && ny >= 0 && ny < (int)h)
								if (originalData[ny * w + nx] < threshold) 	
								{
									avg = std::min<float>(avg, originalData[ny * w + nx]);
									avgCount = 1;
								}
						}

					if (avgCount > 0)
						temp[y * w + x] = avg / avgCount;
				}
			}
		}

		for (int i = 0; i < w * h; i++)
			originalData[i] = temp[i];


		std::cout << "Iteration: " << i + 1 << "/20" << std::endl;
	}

	// Write final blurred heightmap
	if (!WriteTextureToFile(path, w, h, originalData, 1, false))
	{
		ErrMsg("Failed to blur height map");
		return false;
	}
	*/
	return true;
}
UINT Content::AddHeightMap(const std::string &name, const char *path)
{
	const UINT id = static_cast<UINT>(_heightTextures.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_heightTextures.at(i)->name == name)
			return i;
	}

	UINT width, height;
	std::vector<float> texData;

	if (!LoadTextureFromFile(path, width, height, texData, 1, true))
	{
		ErrMsg("Failed to load texture from file!");
		return CONTENT_NULL;
	}

	HeightTexture *ht = new HeightTexture(name, id);
	ht->data.Initialize(texData, width, height, name);

#pragma omp critical
	{
		_heightTextures.push_back(ht);
	}


#if(0)
	if (name == "HM_CaveHeightmap")
	{
		if (!BlurHeightMap(ht->data, path))
		{
			ErrMsg("Failed to add height map");
			return false;
		}
	}
#endif

	return id;
}

UINT Content::GetMeshCount() const
{
	return static_cast<UINT>(_meshes.size());
}
UINT Content::GetTextureCount() const
{
	return static_cast<UINT>(_textures.size());
}
UINT Content::GetTextureMapCount() const
{
	return static_cast<UINT>(_textureMaps.size());
}
UINT Content::GetSamplerCount() const
{
	return static_cast<UINT>(_samplers.size());
}

void Content::GetMeshNames(std::vector<std::string> *names) const
{
	names->clear();
	names->reserve(_meshes.size());
	for (const Mesh *mesh : _meshes)
		names->push_back(mesh->name);
}
void Content::GetShaderNames(std::vector<std::string> *names) const
{
	names->clear();
	names->reserve(_shaders.size());
	for (const Shader *shader : _shaders)
		names->push_back(shader->name);
}
void Content::GetTextureNames(std::vector<std::string> *names) const
{
	names->clear();
	names->reserve(_textures.size());
	for (const Texture *texture : _textures)
		names->push_back(texture->name);
}
void Content::GetTextureMapNames(std::vector<std::string> *names) const
{
	names->clear();
	names->reserve(_textureMaps.size());
	for (const TextureMap *textureMap : _textureMaps)
		names->push_back(textureMap->name);
}

UINT Content::GetMeshID(const std::string &name) const
{
	std::string lookupName;

	if (name.find("Mesh_", 0) != 0)
		lookupName = "Mesh_" + name; // Add prefix if it is missing.
	else
		lookupName = name;

	const UINT count = static_cast<UINT>(_meshes.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_meshes.at(i)->name == lookupName)
			return i;
	}

	if (lookupName == "Mesh_Error")
	{
		ErrMsg("WARNING: No error mesh defined!");
		return CONTENT_NULL;
	}

	return GetMeshID("Mesh_Error");
}
std::string Content::GetMeshName(UINT id) const
{
	if (id >= _meshes.size())
		return "Uninitialized";
	return _meshes.at(id)->name;
}
MeshD3D11 *Content::GetMesh(const std::string &name) const
{
	UINT id = GetMeshID(name);
	return GetMesh(id);
}
MeshD3D11 *Content::GetMesh(const UINT id) const
{
	if (id == CONTENT_NULL)
	{
		ErrMsg(std::format("Failed to find mesh #{}! Returning default.", id));
		return &_meshes.at(0)->data;
	}

	if (id >= _meshes.size())
	{
		ErrMsg(std::format("Failed to find mesh #{}! Returning default.", id));
		return &_meshes.at(0)->data;
	}

	return &_meshes.at(id)->data;
}

UINT Content::GetShaderID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_shaders.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_shaders.at(i)->name == name)
			return i;
	}

	return CONTENT_NULL;
}
std::string Content::GetShaderName(UINT id) const
{
	if (id >= _shaders.size())
		return "Uninitialized";
	return _shaders.at(id)->name;
}
ShaderD3D11 *Content::GetShader(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_shaders.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_shaders.at(i)->name == name)
			return &_shaders.at(i)->data;
	}

	ErrMsg(std::format("Failed to find shader '{}'! Returning default.", name));
	return &_shaders.at(0)->data;
}
ShaderD3D11 *Content::GetShader(const UINT id) const
{
	if (id == CONTENT_NULL)
	{
		ErrMsg(std::format("Failed to find shader #{}! Returning default.", id));
		return &_shaders.at(0)->data;
	}

	if (id >= _shaders.size())
	{
		ErrMsg(std::format("Failed to find shader #{}! Returning default.", id));
		return &_shaders.at(0)->data;
	}

	return &_shaders.at(id)->data;
}

void Content::GetShaderTypeRange(std::string prefix, UINT &start, UINT &end) const
{
	start = end = 0;

	bool foundFirst = false;
	UINT shaderCount = static_cast<UINT>(_shaders.size());
	for (UINT i = 0; i < shaderCount; i++)
	{
		if (_shaders.at(i)->name.rfind(prefix, 0) == 0)
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
				return;
		}
	}
}

bool Content::HasTexture(const std::string & name) const
{
	std::string lookupName;

	if (name.find("Tex_", 0) != 0)
		lookupName = "Tex_" + name; // Add prefix if it is missing.
	else
		lookupName = name;

	const UINT count = static_cast<UINT>(_textures.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textures.at(i)->name == lookupName)
			return true;
	}

	return false;
}
UINT Content::GetTextureID(const std::string &name) const
{
	std::string lookupName;

	if (name.find("Tex_", 0) != 0)
		lookupName = "Tex_" + name; // Add prefix if it is missing.
	else
		lookupName = name;

	const UINT count = static_cast<UINT>(_textures.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textures.at(i)->name == lookupName)
			return i;
	}

	if (lookupName == "Tex_Fallback")
	{
		ErrMsg("WARNING: No fallback texture defined!");
		return CONTENT_NULL;
	}

	return GetTextureID("Tex_Fallback");
}
std::string Content::GetTextureName(UINT id) const
{
	if (id >= _textures.size())
		return "Uninitialized";
	return _textures.at(id)->name;
}
UINT Content::GetTextureIDByPath(const std::string &path) const
{
	const UINT count = static_cast<UINT>(_textures.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textures.at(i)->path == path)
			return i;
	}

	return CONTENT_NULL;
}
ShaderResourceTextureD3D11 *Content::GetTexture(const UINT id) const
{
	if (id >= _textures.size())
		return GetTexture("Tex_Fallback");

	return &_textures.at(id)->data;
}
ShaderResourceTextureD3D11 *Content::GetTexture(const std::string &name) const
{
	UINT id = GetTextureID(name);
	return GetTexture(id);
}

UINT Content::GetTextureMapID(const std::string &name) const
{
	std::string lookupName;

	if (name.find("TexMap_", 0) != 0)
		lookupName = "TexMap_" + name; // Add prefix if it is missing.
	else
		lookupName = name;

	const UINT count = static_cast<UINT>(_textureMaps.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textureMaps.at(i)->name == lookupName)
			return i;
	}

	return CONTENT_NULL;
}
std::string Content::GetTextureMapName(UINT id) const
{
	if (id >= _textureMaps.size())
		return "Uninitialized";
	return _textureMaps.at(id)->name;
}
UINT Content::GetTextureMapIDByPath(const std::string &path, const TextureType type) const
{
	const UINT count = static_cast<UINT>(_textureMaps.size());

	std::string typeStr = GetTextureMapTypeSuffix(type);

	for (UINT i = 0; i < count; i++)
		if (_textureMaps.at(i)->path == path && _textureMaps.at(i)->name.find(typeStr, 0) != std::string::npos)
			return i;

	return CONTENT_NULL;
}
ShaderResourceTextureD3D11 *Content::GetTextureMap(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_textureMaps.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textureMaps.at(i)->name == name)
			return &_textureMaps.at(i)->data;
	}

	//ErrMsg(std::format("Failed to find texture map '{}'! Returning default.", name));
	return &_textureMaps.at(0)->data;
}
ShaderResourceTextureD3D11 *Content::GetTextureMap(const UINT id) const
{
	if (id == CONTENT_NULL)
	{
		//ErrMsg(std::format("Failed to find texture map #{}! Returning default.", id));
		return &_textureMaps.at(0)->data;
	}

	if (id >= _textureMaps.size())
	{
		//ErrMsg(std::format("Failed to find texture map #{}! Returning default.", id));
		return &_textureMaps.at(0)->data;
	}

	return &_textureMaps.at(id)->data;
}

TextureType Content::GetTextureMapType(const std::string &name) const
{
	if (name.ends_with("_Normal"))
	{
		return TextureType::NORMAL;
	}
	else if (name.ends_with("_Specular"))
	{
		return TextureType::SPECULAR;
	}
	else if (name.ends_with("_Glossiness"))
	{
		return TextureType::GLOSS;
	}
	else if (name.ends_with("_Height"))
	{
		return TextureType::HEIGHT;
	}
	else if (name.ends_with("_Occlusion"))
	{
		return TextureType::OCCLUSION;
	}

	return (TextureType)-1; // Invalid
}
std::string Content::GetTextureMapTypeSuffix(TextureType type) const
{
	switch (type)
	{
	case TextureType::NORMAL:
		return "_Normal";

	case TextureType::SPECULAR:
		return "_Specular";

	case TextureType::GLOSS:
		return "_Glossiness";

	case TextureType::HEIGHT:
		return "_Height";

	case TextureType::OCCLUSION:
		return "_Occlusion";
	}

	return "";
}

void Content::GetTextureMapTypeRange(TextureType type, UINT &start, UINT &end) const
{
	start = end = 0;

	std::string suffix = GetTextureMapTypeSuffix(type);

	bool foundFirst = false;
	UINT texMapCount = static_cast<UINT>(_textureMaps.size());
	for (UINT i = 0; i < texMapCount; i++)
	{
		if (_textureMaps.at(i)->name.ends_with(suffix))
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
				return;
		}
	}

	if (!foundFirst)
		start = end = CONTENT_NULL;
}

UINT Content::GetSamplerID(const std::string &name) const
{
	std::string lookupName;

	if (name.find("SS_", 0) != 0)
		lookupName = "SS_" + name; // Add prefix if it is missing.
	else
		lookupName = name;

	const UINT count = static_cast<UINT>(_samplers.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_samplers.at(i)->name == lookupName)
			return i;
	}

	return CONTENT_NULL;
}
std::string Content::GetSamplerName(UINT id) const
{
	if (id >= _samplers.size())
		return "Uninitialized";
	return _samplers.at(id)->name;
}
SamplerD3D11 *Content::GetSampler(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_samplers.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_samplers.at(i)->name == name)
			return &_samplers.at(i)->data;
	}

	ErrMsg(std::format("Failed to find sampler '{}'! Returning default.", name));
	return &_samplers.at(0)->data;
}
SamplerD3D11 *Content::GetSampler(const UINT id) const
{
	if (id == CONTENT_NULL)
	{
		ErrMsg(std::format("Failed to find sampler #{}! Returning default.", id));
		return &_samplers.at(0)->data;
	}

	if (id >= _samplers.size())
	{
		ErrMsg(std::format("Failed to find sampler #{}! Returning default.", id));
		return &_samplers.at(0)->data;
	}

	return &_samplers.at(id)->data;
}

UINT Content::GetInputLayoutID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_inputLayouts.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_inputLayouts.at(i)->name == name)
			return i;
	}

	return CONTENT_NULL;
}
InputLayoutD3D11 *Content::GetInputLayout(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_inputLayouts.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_inputLayouts.at(i)->name == name)
			return &_inputLayouts.at(i)->data;
	}

	ErrMsg(std::format("Failed to find input layout '{}'! Returning default.", name));
	return &_inputLayouts.at(0)->data;
}
InputLayoutD3D11 *Content::GetInputLayout(const UINT id) const
{
	if (id == CONTENT_NULL)
	{
		ErrMsg(std::format("Failed to find input layout #{}! Returning default.", id));
		return &_inputLayouts.at(0)->data;
	}

	if (id >= _inputLayouts.size())
	{
		ErrMsg(std::format("Failed to find input layout #{}! Returning default.", id));
		return &_inputLayouts.at(0)->data;
	}

	return &_inputLayouts.at(id)->data;
}

UINT Content::GetSoundID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_sounds.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_sounds.at(i)->name == name)
			return i;
	}

	return CONTENT_NULL;
}
SoundSource *Content::GetSound(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_sounds.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_sounds.at(i)->name == name)
			return &_sounds.at(i)->data;
	}

	return nullptr;
}
SoundSource *Content::GetSound(UINT id) const
{
	if (id == CONTENT_NULL)
		return nullptr;
	if (id >= _sounds.size())
		return nullptr;

	return &_sounds.at(id)->data;
}

UINT Content::GetHeightMapID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_heightTextures.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_heightTextures.at(i)->name == name)
			return i;
	}

	return CONTENT_NULL;
}

HeightMap *Content::GetHeightMap(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_heightTextures.size());

	for (UINT i = 0; i < count; i++)
		if (_heightTextures.at(i)->name == name)
			return &_heightTextures.at(i)->data;
	return nullptr;
}

HeightMap *Content::GetHeightMap(UINT id) const
{
	if (id == CONTENT_NULL)
		return nullptr;
	if (id >= _heightTextures.size())
		return nullptr;

	return &_heightTextures.at(id)->data;
}

// Looks for material with the same properties as the arguments.
// If one exists, that one is returned. Otherwise it is created and returned.
const Material *Content::GetOrAddMaterial(Material mat)
{
	auto it = _materialSet.find(&mat);
	if (it != _materialSet.end())
		return (*it);

	Material *newMat = new Material(mat);
	_materialVec.push_back(newMat);
	_materialSet.insert(newMat);

	return newMat;
}
const Material *Content::GetDefaultMaterial()
{
	return GetOrAddMaterial(Material::MakeMat(GetTextureID("Tex_Fallback")));
}
const Material *Content::GetErrorMaterial()
{
	Material mat;
	mat.textureID = GetTextureID("Tex_Error");
	mat.ambientID = GetTextureID("Tex_Red");

	return GetOrAddMaterial(mat);
}
