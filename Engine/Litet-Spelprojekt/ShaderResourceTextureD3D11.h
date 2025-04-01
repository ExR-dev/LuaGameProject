#pragma once

#include <d3d11_4.h>
#include <wrl/client.h>

enum class TextureType
{
	NORMAL = 0,
	SPECULAR = 1,
	GLOSS = 2,
	HEIGHT = 3,
	OCCLUSION = 4,
};


class ShaderResourceTextureD3D11
{
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv = nullptr;

public:
	ShaderResourceTextureD3D11() = default;
	~ShaderResourceTextureD3D11();

	ShaderResourceTextureD3D11(const ShaderResourceTextureD3D11 &other) = delete;
	ShaderResourceTextureD3D11 &operator=(const ShaderResourceTextureD3D11 &other) = delete;
	ShaderResourceTextureD3D11(ShaderResourceTextureD3D11 &&other) = delete;
	ShaderResourceTextureD3D11 &operator=(ShaderResourceTextureD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, ID3D11DeviceContext* context, 
		UINT width, UINT height, const void *textureData, bool autoMipmaps);
	[[nodiscard]] bool Initialize(ID3D11Device *device, ID3D11DeviceContext* context, 
		const D3D11_TEXTURE2D_DESC &textureDesc, const D3D11_SUBRESOURCE_DATA *srData, bool autoMipmaps);

	[[nodiscard]] ID3D11Texture2D *GetTexture() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSRV() const;
};