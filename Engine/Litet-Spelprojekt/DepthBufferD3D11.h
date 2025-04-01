#pragma once

#include <d3d11_4.h>
#include <wrl/client.h>
#include <vector>

class DepthBufferD3D11
{
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> _depthStencilViews;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv = nullptr;

public:
	DepthBufferD3D11() = default;
	DepthBufferD3D11(ID3D11Device *device, UINT width, UINT height, bool hasSRV = false);
	~DepthBufferD3D11();
	DepthBufferD3D11(const DepthBufferD3D11 &other) = delete;
	DepthBufferD3D11 &operator=(const DepthBufferD3D11 &other) = delete;
	DepthBufferD3D11(DepthBufferD3D11 &&other) = delete;
	DepthBufferD3D11 &operator=(DepthBufferD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT width, UINT height,
		bool hasSRV = false, UINT arraySize = 1);

	[[nodiscard]] ID3D11DepthStencilView *GetDSV(UINT arrayIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSRV() const;

	void Reset();
};