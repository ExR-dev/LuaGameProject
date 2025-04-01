#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "Content.h"
#include "CameraBehaviour.h"
#include "SimpleMeshD3D11.h"


namespace DebugDraw
{
	struct LineSection
	{
		DirectX::XMFLOAT3 position;
		float size;
		DirectX::XMFLOAT4 color;
	};
	struct Line 
	{
		LineSection start;
		LineSection end;
	};
}

/// Allows for drawing debug shapes through simple function calls from anywhere.
class DebugDrawer
{
private:
	ID3D11Device *_device			= nullptr;
	ID3D11DeviceContext	*_context	= nullptr;
	Content	*_content				= nullptr;

	Microsoft::WRL::ComPtr<ID3D11Texture2D>			_overlayDST = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	_overlayDSV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	_overlayDSS = nullptr;
	Microsoft::WRL::ComPtr<ID3D11BlendState>		_blendState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	_defaultRasterizer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	_wireframeRasterizer = nullptr;

	std::vector<DebugDraw::Line> _sceneLineList;
	std::vector<DebugDraw::Line> _overlayLineList;
	SimpleMeshD3D11 _sceneMesh;
	SimpleMeshD3D11 _overlayMesh;

	CameraBehaviour *_camera = nullptr;

	[[nodiscard]] bool CreateOverlayDepthStencilTexture(const UINT width, const UINT height);
	[[nodiscard]] bool CreateOverlayDepthStencilState();
	[[nodiscard]] bool CreateBlendState();
	[[nodiscard]] bool CreateRasterizerStates();

	void SortLines(std::vector<DebugDraw::Line> *lineList, DirectX::XMFLOAT3A direction);

	[[nodiscard]] bool CreateMesh(std::vector<DebugDraw::Line> *lineList, SimpleMeshD3D11 *mesh);
	[[nodiscard]] bool RenderLines(std::vector<DebugDraw::Line> *lineList, SimpleMeshD3D11 *mesh);

	[[nodiscard]] bool HasSceneDraws() const;
	[[nodiscard]] bool HasOverlayDraws() const;

public:
	[[nodiscard]] bool Setup(const UINT width, const UINT height, 
		ID3D11Device *device, ID3D11DeviceContext *context, Content *content);

	[[nodiscard]] bool Render(ID3D11RenderTargetView *targetRTV, 
		ID3D11DepthStencilView *targetDSV, const D3D11_VIEWPORT *targetViewport);

	[[nodiscard]] static inline DebugDrawer *GetInstance() 
	{
		static DebugDrawer instance;
		return &instance;
	}

	void Clear();
	void SetCamera(CameraBehaviour *camera);

	/// Push a uniform line to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLine(const DirectX::XMFLOAT3 &start, const DirectX::XMFLOAT3 &end, const float &size, const DirectX::XMFLOAT4 &color, bool useDepth = true);
	/// Push a non-uniform line to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLine(const DebugDraw::Line &line, bool useDepth = true);
	/// Push a non-uniform line to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLine(const DebugDraw::LineSection &start, const DebugDraw::LineSection &end, bool useDepth = true);
	/// Push a sequence of non-uniform discontinuous lines to the debug draw list. They will be drawn on the next render call, then discarded.
	void DrawLines(const std::vector<DebugDraw::Line> &lines, bool useDepth = true);
	/// Push a non-uniform & continuous line strip to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineStrip(const std::vector<DebugDraw::LineSection> &lineStrip, bool useDepth = true);
	/// Push a uniform & continuous line strip to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineStrip(const std::vector<DirectX::XMFLOAT3> &points, const float &size, const DirectX::XMFLOAT4 &color, bool useDepth = true);
	/// Push a uniform & continuous line strip to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineStrip(const DirectX::XMFLOAT3 *points, const UINT length, const float &size, const DirectX::XMFLOAT4 &color, bool useDepth = true);

	/// Push a uniform line to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineThreadSafe(const DirectX::XMFLOAT3 &start, const DirectX::XMFLOAT3 &end, const float &size, const DirectX::XMFLOAT4 &color, bool useDepth = true);
	/// Push a non-uniform line to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineThreadSafe(const DebugDraw::Line &line, bool useDepth = true);
	/// Push a non-uniform line to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineThreadSafe(const DebugDraw::LineSection &start, const DebugDraw::LineSection &end, bool useDepth = true);
	/// Push a sequence of non-uniform discontinuous lines to the debug draw list. They will be drawn on the next render call, then discarded.
	void DrawLinesThreadSafe(const std::vector<DebugDraw::Line> &lines, bool useDepth = true);
	/// Push a non-uniform & continuous line strip to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineStripThreadSafe(const std::vector<DebugDraw::LineSection> &lineStrip, bool useDepth = true);
	/// Push a uniform & continuous line strip to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineStripThreadSafe(const std::vector<DirectX::XMFLOAT3> &points, const float &size, const DirectX::XMFLOAT4 &color, bool useDepth = true);
	/// Push a uniform & continuous line strip to the debug draw list. It will be drawn on the next render call, then discarded.
	void DrawLineStripThreadSafe(const DirectX::XMFLOAT3 *points, const UINT length, const float &size, const DirectX::XMFLOAT4 &color, bool useDepth = true);
};