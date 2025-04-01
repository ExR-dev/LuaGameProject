#pragma once

#include <vector>
#include <string>
#include <set>

#include "InputLayoutD3D11.h"
#include "ShaderD3D11.h"
#include "MeshD3D11.h"
#include "SamplerD3D11.h"
#include "ShaderResourceTextureD3D11.h"
#include "Material.h"
#include "SoundSource.h"
#include "HeightMap.h"

inline constexpr UINT CONTENT_NULL = 0xFFFFFFFF;

struct MaterialProperties
{
	int sampleNormal;		// Use normal map if greater than zero.
	int sampleSpecular;		// Use specular map if greater than zero.
	int sampleGlossiness;	// Use glossiness map if greater than zero.
	int sampleLight;		// Use light map if greater than zero.
	int sampleAmbient;		// Use ambient map if greater than zero.
	int sampleOcclusion;	// Use occlusion map if greater than zero.

	float alphaCutoff = 0.5f;

	float padding[1];
};

struct Mesh
{
	std::string name;
	UINT id;
	MeshD3D11 data;

	Mesh(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Mesh() = default;
	Mesh(const Mesh &other) = delete;
	Mesh &operator=(const Mesh &other) = delete;
	Mesh(Mesh &&other) = delete;
	Mesh &operator=(Mesh &&other) = delete;
};

struct Shader
{
	std::string name;
	UINT id;
	ShaderD3D11 data;

	Shader(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Shader() = default;
	Shader(const Shader &other) = delete;
	Shader &operator=(const Shader &other) = delete;
	Shader(Shader &&other) = delete;
	Shader &operator=(Shader &&other) = delete;
};

struct Texture
{
	std::string name, path;
	UINT id;
	bool transparent = false;
	ShaderResourceTextureD3D11 data;

	Texture(std::string name, std::string path, const UINT id, bool transparent) : name(std::move(name)), path(std::move(path)), id(id), transparent(transparent) { }
	~Texture() = default;
	Texture(const Texture &other) = delete;
	Texture &operator=(const Texture &other) = delete;
	Texture(Texture &&other) = delete;
	Texture &operator=(Texture &&other) = delete;
};

struct TextureMap
{
	std::string name, path;
	UINT id;
	ShaderResourceTextureD3D11 data;

	TextureMap(std::string name, std::string path, const UINT id) : name(std::move(name)), path(std::move(path)), id(id) { }
	~TextureMap() = default;
	TextureMap(const TextureMap &other) = delete;
	TextureMap &operator=(const TextureMap &other) = delete;
	TextureMap(TextureMap &&other) = delete;
	TextureMap &operator=(TextureMap &&other) = delete;
};

struct Sampler
{
	std::string name;
	UINT id;
	SamplerD3D11 data;

	Sampler(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Sampler() = default;
	Sampler(const Sampler &other) = delete;
	Sampler &operator=(const Sampler &other) = delete;
	Sampler(Sampler &&other) = delete;
	Sampler &operator=(Sampler &&other) = delete;
};

struct InputLayout
{
	std::string name;
	UINT id;
	InputLayoutD3D11 data;

	InputLayout(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~InputLayout() = default;
	InputLayout(const InputLayout &other) = delete;
	InputLayout &operator=(const InputLayout &other) = delete;
	InputLayout(InputLayout &&other) = delete;
	InputLayout &operator=(InputLayout &&other) = delete;
};

struct Sound
{
	std::string name;
	UINT id;
	SoundSource data;

	Sound(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Sound() = default;
	Sound(const Sound& other) = delete;
	Sound& operator=(const Sound& other) = delete;
	Sound(Sound&& other) = delete;
	Sound& operator=(Sound&& other) = delete;
};

struct HeightTexture
{
	std::string name;
	UINT id;
	HeightMap data;

	HeightTexture(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~HeightTexture() = default;
	HeightTexture(const HeightTexture &other) = delete;
	HeightTexture &operator=(const HeightTexture &other) = delete;
	HeightTexture(HeightTexture &&other) = delete;
	HeightTexture &operator=(HeightTexture &&other) = delete;
};

struct CompiledData
{
	char *data = nullptr;
	size_t size = 0;

	CompiledData() = default;
	~CompiledData()
	{
		if (data)
		{
			delete[] data;
			data = nullptr;
		}
	}
};

static constexpr auto matPtrCmp = [](Material *a, Material *b) { return (*a) < (*b); };

/// Handles loading, storing and unloading of meshes, shaders, textures, texture maps, samplers and input layouts.
class Content
{
private:
	std::vector<Material*> _materialVec;
	std::set<Material*, decltype(matPtrCmp)> _materialSet;

	// Pointers are used to avoid calling move constructors on the contents when the vectors are resized
	std::vector<Mesh *> _meshes; 
	std::vector<Shader *> _shaders;
	std::vector<Texture *> _textures;
	std::vector<TextureMap *> _textureMaps;
	std::vector<Sampler *> _samplers;
	std::vector<InputLayout *> _inputLayouts;
	std::vector<Sound *> _sounds;
	std::vector<HeightTexture *> _heightTextures;

	bool _hasShutDown = false;

public:
	Content();
	~Content();
	Content(const Content &other) = delete;
	Content &operator=(const Content &other) = delete;
	Content(Content &&other) = delete;
	Content &operator=(Content &&other) = delete;

	void Shutdown();

	CompiledData GetMeshData(const std::string &name, const char *path) const;
	CompiledData GetShaderData(const std::string &name, const char *path, ShaderType shaderType) const;
	CompiledData GetTextureData(const std::string &name, const char *path, bool transparent) const;
	CompiledData GetTextureMapData(const std::string &name, const char *path, TextureType mapType) const;
	CompiledData GetHeightMapData(const std::string &name, const char *path) const;

	UINT AddMesh(ID3D11Device *device, const std::string &name, const MeshData &meshData);
	UINT AddMesh(ID3D11Device *device, const std::string &name, const char *path);

	UINT AddShader(ID3D11Device *device, const std::string &name, ShaderType shaderType, const void *dataPtr, size_t dataSize);
	UINT AddShader(ID3D11Device *device, const std::string &name, ShaderType shaderType, const char *path);

	UINT AddTexture(ID3D11Device *device, ID3D11DeviceContext* context, const std::string &name, const char *path, bool transparent);
	UINT AddTextureMap(ID3D11Device *device, ID3D11DeviceContext* context, const std::string &name, TextureType mapType, const char *path);
	UINT AddHeightMap(const std::string &name, const char *path);

	UINT AddSampler(ID3D11Device *device, const std::string &name, D3D11_TEXTURE_ADDRESS_MODE adressMode, D3D11_FILTER filter);
	UINT AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics, 
		const void *vsByteData, size_t vsByteSize);
	UINT AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics, UINT vShaderID);
	
	[[nodiscard]] UINT GetMeshCount() const;
	[[nodiscard]] UINT GetTextureCount() const;
	[[nodiscard]] UINT GetTextureMapCount() const;
	[[nodiscard]] UINT GetSamplerCount() const;

	void GetMeshNames(std::vector<std::string> *names) const;
	void GetShaderNames(std::vector<std::string> *names) const;
	void GetTextureNames(std::vector<std::string> *names) const;
	void GetTextureMapNames(std::vector<std::string> *names) const;

	[[nodiscard]] UINT GetMeshID(const std::string &name) const;
	[[nodiscard]] std::string GetMeshName(UINT id) const;
	[[nodiscard]] MeshD3D11 *GetMesh(const std::string &name) const;
	[[nodiscard]] MeshD3D11 *GetMesh(UINT id) const;

	[[nodiscard]] UINT GetShaderID(const std::string &name) const;
	[[nodiscard]] std::string GetShaderName(UINT id) const;
	[[nodiscard]] ShaderD3D11 *GetShader(const std::string &name) const;
	[[nodiscard]] ShaderD3D11 *GetShader(UINT id) const;

	/// Returns the first and last shader ID's of the specified prefix.
	void GetShaderTypeRange(std::string prefix, UINT &start, UINT &end) const;

	[[nodiscard]] bool HasTexture(const std::string &name) const;
	[[nodiscard]] UINT GetTextureID(const std::string &name) const;
	[[nodiscard]] std::string GetTextureName(UINT id) const;
	[[nodiscard]] UINT GetTextureIDByPath(const std::string &path) const;
	[[nodiscard]] ShaderResourceTextureD3D11 *GetTexture(const std::string &name) const;
	[[nodiscard]] ShaderResourceTextureD3D11 *GetTexture(UINT id) const;

	[[nodiscard]] UINT GetTextureMapID(const std::string &name) const;
	[[nodiscard]] std::string GetTextureMapName(UINT id) const;
	[[nodiscard]] UINT GetTextureMapIDByPath(const std::string &path, TextureType type) const;
	[[nodiscard]] ShaderResourceTextureD3D11 *GetTextureMap(const std::string &name) const;
	[[nodiscard]] ShaderResourceTextureD3D11 *GetTextureMap(UINT id) const;

	[[nodiscard]] TextureType GetTextureMapType(const std::string &name) const;
	[[nodiscard]] std::string GetTextureMapTypeSuffix(TextureType type) const;

	/// Returns the first and last texture map ID's of the specified type.
	void GetTextureMapTypeRange(TextureType type, UINT &start, UINT &end) const;

	[[nodiscard]] UINT GetSamplerID(const std::string &name) const;
	[[nodiscard]] std::string GetSamplerName(UINT id) const;
	[[nodiscard]] SamplerD3D11 *GetSampler(const std::string &name) const;
	[[nodiscard]] SamplerD3D11 *GetSampler(UINT id) const;

	[[nodiscard]] UINT GetInputLayoutID(const std::string &name) const;
	[[nodiscard]] InputLayoutD3D11 *GetInputLayout(const std::string &name) const;
	[[nodiscard]] InputLayoutD3D11 *GetInputLayout(UINT id) const;

	[[nodiscard]] UINT GetSoundID(const std::string &name) const;
	[[nodiscard]] SoundSource *GetSound(const std::string &name) const;
	[[nodiscard]] SoundSource *GetSound(UINT id) const;

	[[nodiscard]] UINT GetHeightMapID(const std::string &name) const;
	[[nodiscard]] HeightMap *GetHeightMap(const std::string &name) const;
	[[nodiscard]] HeightMap *GetHeightMap(UINT id) const;


	template <typename... Args>
	[[nodiscard]] const Material *GetOrAddMaterial(Args&&... args);
	const Material *GetOrAddMaterial(Material mat);
	[[nodiscard]] const Material *GetDefaultMaterial();
	[[nodiscard]] const Material *GetErrorMaterial();
};
