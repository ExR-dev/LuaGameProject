#include "stdafx.h"
#include "DebugDrawer.h"
#include <Windows.h>
#include <algorithm>
#include "GameMath.h"
#include "ErrMsg.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DebugDraw;

bool DebugDrawer::Setup(const UINT width, const UINT height, 
	ID3D11Device *device, ID3D11DeviceContext *context, Content *content)
{
#ifndef DEBUG_DRAW
	return true;
#endif

	_device = device;
	_context = context;
	_content = content;

	if (!CreateOverlayDepthStencilTexture(width, height))
	{
		ErrMsg("Failed to create depth stencil texture!");
		return false;
	}
	
	if (!CreateOverlayDepthStencilState())
	{
		ErrMsg("Failed to create depth stencil states!");
		return false;
	}
	
	if (!CreateBlendState())
	{
		ErrMsg("Failed to create blend state!");
		return false;
	}

	if (!CreateRasterizerStates())
	{
		ErrMsg("Failed to create rasterizer states!");
		return false;
	}

	return true;
}

bool DebugDrawer::CreateOverlayDepthStencilTexture(const UINT width, const UINT height)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D16_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (FAILED(_device->CreateTexture2D(&textureDesc, nullptr, _overlayDST.ReleaseAndGetAddressOf())))
	{
		ErrMsg("Failed to create depth stencil texture!");
		return false;
	}

	return SUCCEEDED(_device->CreateDepthStencilView(_overlayDST.Get(), nullptr, _overlayDSV.ReleaseAndGetAddressOf()));
}

bool DebugDrawer::CreateOverlayDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { };
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	if (FAILED(_device->CreateDepthStencilState(&depthStencilDesc, _overlayDSS.ReleaseAndGetAddressOf())))
	{
		ErrMsg("Failed to create overlay depth stencil state!");
		return false;
	}

	return true;
}

bool DebugDrawer::CreateBlendState()
{
	D3D11_BLEND_DESC blendDesc = { };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(_device->CreateBlendState(&blendDesc, _blendState.ReleaseAndGetAddressOf())))
	{
		ErrMsg("Failed to create blend state!");
		return false;
	}

	return true;
}

bool DebugDrawer::CreateRasterizerStates()
{
	D3D11_RASTERIZER_DESC rasterizerDesc = { };
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	if (FAILED(_device->CreateRasterizerState(&rasterizerDesc, &_defaultRasterizer)))
	{
		ErrMsg("Failed to create default rasterizer state!");
		return false;
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.AntialiasedLineEnable = true;

	if (FAILED(_device->CreateRasterizerState(&rasterizerDesc, &_wireframeRasterizer)))
	{
		ErrMsg("Failed to create wireframe rasterizer state!");
		return false;
	}

	return true;
}

bool DebugDrawer::HasSceneDraws() const
{
	return _sceneLineList.size() > 0;
}
bool DebugDrawer::HasOverlayDraws() const
{
	return _overlayLineList.size() > 0;
}

void DebugDrawer::Clear()
{
#ifndef DEBUG_DRAW
	return;
#endif

	UINT sceneLineCount = static_cast<UINT>(_sceneLineList.size());
	UINT overlayLineCount = static_cast<UINT>(_overlayLineList.size());

	_sceneLineList.clear();
	_overlayLineList.clear();

	_sceneLineList.reserve(sceneLineCount);
	_overlayLineList.reserve(overlayLineCount);
}

bool DebugDrawer::Render(ID3D11RenderTargetView *targetRTV, 
	ID3D11DepthStencilView *targetDSV, const D3D11_VIEWPORT *targetViewport)
{
#ifndef DEBUG_DRAW
	return true;
#endif

#ifdef PIX_TIMELINING
	PIXScopedEvent(911827178, "Debug Render");
#endif

	if (!_camera)
	{
		ErrMsg("Camera not set!");
		return false;
	}

	bool hasSceneDraws = HasSceneDraws();
	bool hasOverlayDraws = HasOverlayDraws();

	if (!hasSceneDraws && !hasOverlayDraws)
		return true;

	ID3D11BlendState *prevBlendState;
	UINT prevSampleMask = 0;
	FLOAT prevBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_context->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);

	constexpr float transparentBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_context->OMSetBlendState(_blendState.Get(), transparentBlendFactor, 0xffffffff);

	if (!_camera->BindDebugDrawBuffers())
	{
		ErrMsg("Failed to bind debug draw buffers!");
		return false;
	}

	if (hasSceneDraws)
	{ 
		// Draw with depth
		_context->OMSetRenderTargets(1, &targetRTV, targetDSV);
		_context->RSSetViewports(1, targetViewport);

		if (!RenderLines(&_sceneLineList, &_sceneMesh))
		{
			ErrMsg("Failed to render lines in scene!");
			return false;
		}
	}

	if (hasOverlayDraws)
	{ 
		// Draw Overlay
		_context->ClearDepthStencilView(_overlayDSV.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0);
		_context->OMSetRenderTargets(1, &targetRTV, _overlayDSV.Get());

		ID3D11DepthStencilState *prevDepthStencilState;
		_context->OMGetDepthStencilState(&prevDepthStencilState, 0);
		_context->OMSetDepthStencilState(_overlayDSS.Get(), 0);

		if (!RenderLines(&_overlayLineList, &_overlayMesh))
		{
			ErrMsg("Failed to render lines in scene!");
			return false;
		}

		// Reset depth stencil state
		_context->OMSetDepthStencilState(prevDepthStencilState, 0);
		prevDepthStencilState->Release();
	}

	// Reset blend state
	_context->OMSetBlendState(prevBlendState, prevBlendFactor, prevSampleMask);

	// Unbind render target
	static ID3D11RenderTargetView *const nullRTV = nullptr;
	_context->OMSetRenderTargets(1, &nullRTV, nullptr);

	return true;
}

bool DebugDrawer::RenderLines(std::vector<Line> *lineList, SimpleMeshD3D11 *mesh)
{
	if (lineList->size() <= 0)
		return true;

	if (!CreateMesh(lineList, mesh))
	{
		ErrMsg("Failed to create mesh!");
		return false;
	}

	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	_context->RSSetState(_defaultRasterizer.Get());

	_context->IASetInputLayout(_content->GetInputLayout("IL_DebugDraw")->GetInputLayout());

	if (!_content->GetShader("VS_DebugDraw")->BindShader(_context))
	{
		ErrMsg("Failed to bind debug vertex shader!");
		return false;
	}

	if (!_content->GetShader("GS_DebugLine")->BindShader(_context))
	{
		ErrMsg("Failed to bind debug line geometry shader!");
		return false;
	}

	if (!_content->GetShader("PS_DebugDraw")->BindShader(_context))
	{
		ErrMsg("Failed to bind debug pixel shader!");
		return false;
	}

	mesh->BindMeshBuffers(_context);
	mesh->PerformDrawCall(_context);

	_context->GSSetShader(nullptr, nullptr, 0);

	return true;
}

inline static float ReturnClosest(const float &a, const float &b)
{
	return a < b ? a : b;
}
inline static float ReturnFarthest(const float &a, const float &b)
{
	return a < b ? b : a;
}
inline static float ReturnCenter(const float &a, const float &b)
{
	return (a + b) * 0.5f;
}
void DebugDrawer::SortLines(std::vector<Line> *lineList, XMFLOAT3A direction)
{
	return;
	XMVECTOR dir = XMLoadFloat3A(&direction);

	// Sort points based on their projection along the direction
	std::sort(lineList->begin(), lineList->end(),
		[&dir](const Line &a, const Line &b) { // Calculate dot products and compare scalar results

			float aDist = ReturnCenter(
				XMVectorGetX(XMVector3Dot(Load(a.start.position), dir)), 
				XMVectorGetX(XMVector3Dot(Load(a.end.position), dir))
			);

			float bDist = ReturnCenter(
				XMVectorGetX(XMVector3Dot(Load(b.start.position), dir)), 
				XMVectorGetX(XMVector3Dot(Load(b.end.position), dir))
			);

			return aDist > bDist;
		});
}

bool DebugDrawer::CreateMesh(std::vector<Line> *lineList, SimpleMeshD3D11 *mesh)
{
	SortLines(lineList, _camera->GetTransform()->GetForward(World));

	UINT vertCount = static_cast<UINT>(lineList->size() * 2);
	SimpleVertex *verticeData = new SimpleVertex[vertCount];

	std::memcpy(
		verticeData,
		lineList->data(),
		sizeof(SimpleVertex) * vertCount
	);

	SimpleMeshData simpleMeshData;
	simpleMeshData.vertexInfo.sizeOfVertex = sizeof(SimpleVertex);
	simpleMeshData.vertexInfo.nrOfVerticesInBuffer = vertCount;
	simpleMeshData.vertexInfo.vertexData = &(verticeData[0].x);

	if (!mesh->Initialize(_device, simpleMeshData))
	{
		ErrMsg("Failed to initialize simple mesh!");
		return false;
	}

	return true;
}

void DebugDrawer::SetCamera(CameraBehaviour *camera)
{
#ifndef DEBUG_DRAW
	return;
#endif

	_camera = camera;
}

void DebugDrawer::DrawLine(const Line &line, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	auto &lineList = useDepth ? _sceneLineList : _overlayLineList;

	lineList.push_back(line);
}
void DebugDrawer::DrawLine(const XMFLOAT3 &start, const XMFLOAT3 &end, const float &size, const XMFLOAT4 &color, bool useDepth)
{
	DrawLine({ {start, size, color}, {end, size, color} }, useDepth);
}
void DebugDrawer::DrawLine(const LineSection &start, const LineSection &end, bool useDepth)
{
	DrawLine({ start, end }, useDepth);
}
void DebugDrawer::DrawLines(const std::vector<Line> &lines, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	auto &lineList = useDepth ? _sceneLineList : _overlayLineList;

	lineList.insert(std::end(lineList), std::begin(lines), std::end(lines));
}
void DebugDrawer::DrawLineStrip(const std::vector<LineSection> &lineStrip, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	auto &lineList = useDepth ? _sceneLineList : _overlayLineList;

	if (lineStrip.size() <= 1)
		return;

	Line line;
	const LineSection *prevLine = &lineStrip[0];

	for (UINT i = 1; i < lineStrip.size(); i++)
	{
		line = { *prevLine, lineStrip[i] };
		prevLine = &lineStrip[i];
		lineList.push_back(line);
	}
}
void DebugDrawer::DrawLineStrip(const std::vector<XMFLOAT3> &points, const float &size, const XMFLOAT4 &color, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	std::vector<LineSection> lineStrip;
	lineStrip.reserve(points.size());

	for (UINT i = 0; i < points.size(); i++)
		lineStrip.push_back({ points[i], size, color });

	DrawLineStrip(lineStrip, useDepth);
}
void DebugDrawer::DrawLineStrip(const XMFLOAT3 *points, const UINT length, const float &size, const XMFLOAT4 &color, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	std::vector<LineSection> lineStrip;
	lineStrip.reserve(length);

	for (UINT i = 0; i < length; i++)
		lineStrip.push_back({ points[i], size, color });

	DrawLineStrip(lineStrip, useDepth);
}

void DebugDrawer::DrawLineThreadSafe(const Line &line, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	auto &lineList = useDepth ? _sceneLineList : _overlayLineList;

#pragma omp critical
	{
		lineList.push_back(line);
	}
}
void DebugDrawer::DrawLineThreadSafe(const XMFLOAT3 &start, const XMFLOAT3 &end, const float &size, const XMFLOAT4 &color, bool useDepth)
{
	DrawLineThreadSafe({ {start, size, color}, {end, size, color} }, useDepth);
}
void DebugDrawer::DrawLineThreadSafe(const LineSection &start, const LineSection &end, bool useDepth)
{
	DrawLineThreadSafe({ start, end }, useDepth);
}
void DebugDrawer::DrawLinesThreadSafe(const std::vector<Line> &lines, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	auto &lineList = useDepth ? _sceneLineList : _overlayLineList;

#pragma omp critical
	{
		lineList.insert(std::end(lineList), std::begin(lines), std::end(lines));
	}
}
void DebugDrawer::DrawLineStripThreadSafe(const std::vector<LineSection> &lineStrip, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	auto &lineList = useDepth ? _sceneLineList : _overlayLineList;

	if (lineStrip.size() <= 1)
		return;

	Line line;
	const LineSection *prevLine = &lineStrip[0];

#pragma omp critical
	{
		for (UINT i = 1; i < lineStrip.size(); i++)
		{
			line = { *prevLine, lineStrip[i] };
			prevLine = &lineStrip[i];

			lineList.push_back(line);
		}
	}
}
void DebugDrawer::DrawLineStripThreadSafe(const std::vector<XMFLOAT3> &points, const float &size, const XMFLOAT4 &color, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	std::vector<LineSection> lineStrip;
	lineStrip.reserve(points.size());

	for (UINT i = 0; i < points.size(); i++)
		lineStrip.push_back({ points[i], size, color });

	DrawLineStripThreadSafe(lineStrip, useDepth);
}
void DebugDrawer::DrawLineStripThreadSafe(const XMFLOAT3 *points, const UINT length, const float &size, const XMFLOAT4 &color, bool useDepth)
{
#ifndef DEBUG_DRAW
	return;
#endif

	std::vector<LineSection> lineStrip;
	lineStrip.reserve(length);

	for (UINT i = 0; i < length; i++)
		lineStrip.push_back({ points[i], size, color });

	DrawLineStripThreadSafe(lineStrip, useDepth);
}
