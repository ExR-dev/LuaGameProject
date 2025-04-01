#pragma once

#include <Windows.h>
#include <d3d11.h>


[[nodiscard]] bool SetupD3D11(
	UINT width, UINT height, HWND window, 
	ID3D11Device *&device, 
	ID3D11DeviceContext *&immediateContext, 
	IDXGISwapChain *&swapChain, 
	ID3D11RenderTargetView *&rtv,
#ifdef USE_IMGUI
	ID3D11RenderTargetView *&imGuiRtv,
#endif
	ID3D11Texture2D *&dsTexture, 
	ID3D11DepthStencilView *&dsView,
	ID3D11UnorderedAccessView *&uav,
	ID3D11BlendState *&blendState,
	ID3D11DepthStencilState *&normalDepthStencilState,
	ID3D11DepthStencilState *&transparentDepthStencilState,
	D3D11_VIEWPORT &viewport);


void ReportLiveDeviceObjects(ID3D11Device *&device);