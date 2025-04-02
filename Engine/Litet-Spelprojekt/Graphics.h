#pragma once

#include <d3d11.h>
#include <array>
#include <wrl/client.h>
#include "Content.h"
#include "Time.h"
#include "RenderTargetD3D11.h"
#include "Window.h"
#include "SpotLightCollection.h"
#include "PointLightCollection.h"
#include "CameraBehaviour.h"
#include "DebugDrawer.h"

/// Maximum amount of lights for each type in each light tile.
/// Value should follow (n^2 - 1), where n is any non-zero natural number. 
/// If changing, remember to update MAX_LIGHTS in LightData.hlsli too.
constexpr UINT MAX_LIGHTS = 15; 
/// Resolution of the light tile grid. Total amount of light tiles is LIGHT_GRID_RES^2.
constexpr UINT LIGHT_GRID_RES = 8;

constexpr UINT RENDER_TYPES = 12;

enum RenderType
{
	DEFAULT,
	POSITION,
	NORMAL,
	AMBIENT,
	DIFFUSE,
	DEPTH,
	LIGHTING,
	SHADOW,
	SPECULAR,
	SPECULAR_STRENGTH,
	UV_COORDS,
	TRANSPARENCY,
};

enum LightType
{
	SPOTLIGHT,
	POINTLIGHT,
	SIMPLE_SPOTLIGHT,
	SIMPLE_POINTLIGHT,
};

struct LightTile
{
	UINT spotlights[MAX_LIGHTS];
	UINT pointlights[MAX_LIGHTS*6];
	UINT simpleSpotlights[MAX_LIGHTS];
	UINT simplePointlights[MAX_LIGHTS];

	UINT spotlightCount = 0;
	UINT pointlightCount = 0;
	UINT simpleSpotlightCount = 0;
	UINT simplePointlightCount = 0;
};

struct GeneralData
{
	float time = 0.0f;
	float deltaTime = 0.0f;
	int randInt = 0;
	float randNorm = 0.0f;
};

struct FogSettingsBuffer
{
	float thickness = 0.175f;
	float stepSize = 0.0f;
	UINT minSteps = 0;
	UINT maxSteps = 96;
};

struct DistortionSettingsBuffer
{
	DirectX::XMFLOAT3 distortionOrigin = { 0.0f, 0.0f, 0.0f };
	float distortionStrength = 0.0f;
};

/// Handles rendering of the scene and the GUI.
class Graphics
{
private:
	bool _isSetup		= false;
	bool _isRendering	= false;

	ID3D11Device *_device			= nullptr;
	ID3D11DeviceContext	*_context	= nullptr;
	Content	*_content				= nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain>				_swapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		_rtv = nullptr;
#ifdef USE_IMGUI
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      _imGuiRtv = nullptr;
#endif
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				_dsTexture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		_dsView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>	_uav = nullptr;
	Microsoft::WRL::ComPtr<ID3D11BlendState>			_tbs = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		_ndss = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		_tdss = nullptr;
	D3D11_VIEWPORT _viewport = { };
	D3D11_VIEWPORT _viewportBlur = { };
	D3D11_VIEWPORT _viewportFog = { };

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> _defaultRasterizer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> _wireframeRasterizer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> _shadowRasterizer = nullptr;

	bool _renderTransparency = true;
	bool _renderOverlay = true;
	bool _renderDebugDraw = true;
	bool _renderPostFX = true;
	bool _wireframe = false;
	bool _vSync = true;

	/// Scene is rendered to sceneRT, bright areas are rendered to sceneRT's alpha and depth is rendered to depthRT.
	/// sceneRT is downsampled and blurred horizontally to blurStageOneRT. Strength based on sceneRT's alpha.
	/// blurStageOneRT is blurred vertically to blurStageTwoRT.
	/// the depthRT is used to step through the scene and output total fog to fogRT.
	/// sceneRT is then combined with blurStageTwoRT and fogRT to produce the final image, output is sent to _uav.
	RenderTargetD3D11 _sceneRT; // RGBA
	RenderTargetD3D11 _depthRT; // R
	RenderTargetD3D11 _IntermediaryRT; // RGBA
	RenderTargetD3D11 _blurRT; // RGBA
	RenderTargetD3D11 _fogRT; // RGBA

	RenderType _renderOutput = DEFAULT;

	FogSettingsBuffer _fogSettings = { };
	int _blurIterations = 2;

	DistortionSettingsBuffer _distortionSettings = { };

	CameraBehaviour *_currViewCamera = nullptr;

	LightTile *_lightGrid;
	StructuredBufferD3D11 _lightGridBuffer;

	ConstantBufferD3D11 _globalLightBuffer;
	ConstantBufferD3D11 _generalDataBuffer;
	ConstantBufferD3D11 _fogSettingsBuffer;
	ConstantBufferD3D11 _distortionSettingsBuffer;
	SpotLightCollection *_currSpotLightCollection = nullptr;
	PointLightCollection *_currPointLightCollection = nullptr;

	UINT
		_currMeshID			= CONTENT_NULL,
		_currTexID			= CONTENT_NULL,
		_currNormalID		= CONTENT_NULL,
		_currSpecularID		= CONTENT_NULL,
		_currGlossinessID	= CONTENT_NULL,
		_currAmbientID		= CONTENT_NULL,
		_currLightID		= CONTENT_NULL,
		_currOcclusionID	= CONTENT_NULL,
		_currHeightID		= CONTENT_NULL,
		_currSamplerID		= CONTENT_NULL,
		_currVsID			= CONTENT_NULL,
		_currPsID			= CONTENT_NULL,
		_currInputLayoutID	= CONTENT_NULL;

	DirectX::XMFLOAT4A _ambientColor = { 0.06f, 0.06f, 0.06f, 0.0f };
	float _screenFadeAmount = 0.0f; /// Screen is black at 1.0f, normal at 0.0f. Use for transition fades.
	float _screenFadeRate = 0.0f;

#ifdef USE_IMGUI
	D3D11_RASTERIZER_DESC _shadowRasterizerDesc = { };
	D3D11_BLEND_DESC _transparentBlendDesc = { };
#endif


	/// Renders all queued entities to the specified target.
	[[nodiscard]] bool RenderToTarget(ID3D11RenderTargetView *targetRTV, ID3D11DepthStencilView *targetDSV, 
		const D3D11_VIEWPORT *targetViewport);

	[[nodiscard]] bool RenderSpotlights();
	[[nodiscard]] bool RenderPointlights();

	/// Renders all queued opaque entities to the depth buffers of all shadow-casting lights.
	[[nodiscard]] bool RenderShadowCasters();

	[[nodiscard]] bool RenderOpaque(ID3D11RenderTargetView *targetRTV, ID3D11RenderTargetView *targetDepthRTV, 
		ID3D11DepthStencilView *targetDSV, const D3D11_VIEWPORT *targetViewport, bool overlayStage = false);

	[[nodiscard]] bool RenderGeometry(bool overlayStage, bool skipPixelShader = false);

	[[nodiscard]] bool RenderCustom(ID3D11RenderTargetView *targetRTV, ID3D11DepthStencilView *targetDSV, 
		const D3D11_VIEWPORT *targetViewport, const std::string &pixelShader, bool overlayStage = false);

	[[nodiscard]] bool RenderTransparency(ID3D11RenderTargetView *targetRTV, ID3D11DepthStencilView *targetDSV,
		const D3D11_VIEWPORT *targetViewport);

	[[nodiscard]] bool RenderPostFX();

	[[nodiscard]] bool ResetRenderState();

public:
	~Graphics();

	[[nodiscard]] bool Setup(UINT width, UINT height, const Window *window,
		ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content);

	void Shutdown();

	[[nodiscard]] bool SetCamera(CameraBehaviour *viewCamera);
	[[nodiscard]] bool SetSpotlightCollection(SpotLightCollection *spotlights);
	[[nodiscard]] bool SetPointlightCollection(PointLightCollection *pointlights);

	[[nodiscard]] DebugDrawer *GetDebugDrawer();

	void ResetLightGrid();
	void AddLightToTile(UINT tileIndex, UINT lightIndex, LightType type);

	/// Begins a screen fade with the specified duration.
	/// Set to positive to fade to black.
	/// Set to negative to fade back from black.
	void BeginScreenFade(float duration);
	/// Manually sets the screen fade amount to a constant value.
	void SetScreenFadeManual(float amount);
	[[nodiscard]] float GetScreenFadeAmount() const;
	[[nodiscard]] float GetScreenFadeRate() const;

	void SetDistortionOrigin(DirectX::XMFLOAT3A origin);
	void SetDistortionStrength(float strength);

	void SetFogSettings(FogSettingsBuffer fogSettings);

	void SetAmbientColor(DirectX::XMFLOAT4A color);

	/// Begins scene rendering, enabling entities to be queued for rendering.
	[[nodiscard]] bool BeginSceneRender();

	/// Renders all queued entities to the window.
	[[nodiscard]] bool EndSceneRender(Time &time);

#ifdef USE_IMGUI
	[[nodiscard]] bool BeginUIRender() const;
	[[nodiscard]] bool RenderUI(Time &time);
	[[nodiscard]] bool EndUIRender() const;
#endif

	/// Resets variables and clears all render queues.
	[[nodiscard]] bool EndFrame();
};
