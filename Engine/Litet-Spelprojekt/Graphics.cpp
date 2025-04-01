#include "stdafx.h"
#include "Graphics.h"

#include "Entity.h"
#include "D3D11Helper.h"
#include "MeshBehaviour.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif

using Microsoft::WRL::ComPtr;

Graphics::~Graphics()
{
	delete[] _lightGrid;

	if (_isSetup)
		Shutdown();
}

bool Graphics::Setup(const UINT width, const UINT height, const Window window, 
	ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(746814486, "Graphics Setup");
#endif

#ifdef EDIT_MODE
	_ambientColor = { 0.5f, 0.5f, 0.5f, 0.0f };
#endif

	if (_isSetup)
	{
		ErrMsg("Failed to set up graphics, graphics has already been set up!");
		return false;
	}

	if (!SetupD3D11(width, height, window.GetHWND(), device, immediateContext, 
			*_swapChain.ReleaseAndGetAddressOf(),
			*_rtv.ReleaseAndGetAddressOf(), 
#ifdef USE_IMGUI
			*_imGuiRtv.ReleaseAndGetAddressOf(),
#endif
			*_dsTexture.ReleaseAndGetAddressOf(), 
			*_dsView.ReleaseAndGetAddressOf(), 
			*_uav.ReleaseAndGetAddressOf(), 
			*_tbs.ReleaseAndGetAddressOf(), 
			*_ndss.ReleaseAndGetAddressOf(), 
			*_tdss.ReleaseAndGetAddressOf(), 
			_viewport))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}

#ifdef USE_IMGUI
	_transparentBlendDesc = { };
	_transparentBlendDesc.AlphaToCoverageEnable = false;
	_transparentBlendDesc.IndependentBlendEnable = false;
	_transparentBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	_transparentBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	_transparentBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	_transparentBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	_transparentBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	_transparentBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	_transparentBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
	_transparentBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
#endif

	_viewportBlur = _viewport;
	_viewportBlur.Width *= 0.25f;
	_viewportBlur.Height *= 0.25f;

	_viewportFog = _viewport;
	_viewportFog.Width *= 0.25f;
	_viewportFog.Height *= 0.25f;

	// Render Targets
	{
		if (!_sceneRT.Initialize(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, true))
		{
			ErrMsg("Failed to initialize scene render target!");
			return false;
		}

		if (!_depthRT.Initialize(device, width, height, DXGI_FORMAT_R32_FLOAT, true))
		{
			ErrMsg("Failed to initialize depth render target!");
			return false;
		}

		if (!_IntermediaryRT.Initialize(device, static_cast<UINT>(_viewportBlur.Width), static_cast<UINT>(_viewportBlur.Height), DXGI_FORMAT_R16G16B16A16_FLOAT, true, true))
		{
			ErrMsg("Failed to initialize blur stage one render target!");
			return false;
		}

		if (!_blurRT.Initialize(device, static_cast<UINT>(_viewportBlur.Width), static_cast<UINT>(_viewportBlur.Height), DXGI_FORMAT_R16G16B16A16_FLOAT, true, true))
		{
			ErrMsg("Failed to initialize blur stage two render target!");
			return false;
		}

		if (!_fogRT.Initialize(device, static_cast<UINT>(_viewportFog.Width), static_cast<UINT>(_viewportFog.Height), DXGI_FORMAT_R16G16B16A16_FLOAT, true, true))
		{
			ErrMsg("Failed to initialize fog render target!");
			return false;
		}
	}

	if (!DebugDrawer::GetInstance()->Setup(width, height, device, immediateContext, content))
	{
		ErrMsg("Failed to setup debug drawer!");
		return false;
	}

	_lightGrid = new LightTile[LIGHT_GRID_RES * LIGHT_GRID_RES];

	ResetLightGrid();
	if (!_lightGridBuffer.Initialize(device, sizeof(LightTile), LIGHT_GRID_RES * LIGHT_GRID_RES,
		true, false, true, _lightGrid))
	{
		ErrMsg("Failed to initialize light tile buffer!");
		return false;
	}

	if (!_globalLightBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4A), &_ambientColor))
	{
		ErrMsg("Failed to initialize global light buffer!");
		return false;
	}
	
	GeneralData generalData{};
	if (!_generalDataBuffer.Initialize(device, sizeof(GeneralData), &generalData))
	{
		ErrMsg("Failed to initialize general data buffer!");
		return false;
	}
	
	if (!_fogSettingsBuffer.Initialize(device, sizeof(FogSettingsBuffer), &_fogSettings))
	{
		ErrMsg("Failed to initialize fog settings buffer!");
		return false;
	}
	
	if (!_distortionSettingsBuffer.Initialize(device, sizeof(DistortionSettingsBuffer), &_distortionSettings))
	{
		ErrMsg("Failed to initialize fog settings buffer!");
		return false;
	}

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

	if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &_defaultRasterizer)))
	{
		ErrMsg("Failed to create default rasterizer state!");
		return false;
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &_wireframeRasterizer)))
	{
		ErrMsg("Failed to create wireframe rasterizer state!");
		return false;
	}

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.DepthBias = -5;
	rasterizerDesc.DepthBiasClamp = -0.01f;
	rasterizerDesc.SlopeScaledDepthBias = -1.0f;
	rasterizerDesc.DepthClipEnable = false;

	if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &_shadowRasterizer)))
	{
		ErrMsg("Failed to create shadow rasterizer state!");
		return false;
	}

#ifdef DEBUG_BUILD
	std::memcpy(&_shadowRasterizerDesc, &rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
#endif

#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	//io.MouseDrawCursor = false;
	//io.ConfigFlags = 0;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigViewportsNoAutoMerge = true;

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplSDL3_InitForD3D(window.GetWindow());
	ImGui_ImplDX11_Init(device, immediateContext);
	ImGui::StyleColorsDark();
#endif

	_device = device;
	_context = immediateContext;
	_content = content;

	_isSetup = true;
	return true;
}

void Graphics::Shutdown()
{
#ifdef USE_IMGUI
	if (_isSetup)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}
#endif

	_isSetup = false;
}

bool Graphics::SetCamera(CameraBehaviour *viewCamera)
{
	if (viewCamera == nullptr)
	{
		ErrMsg("Failed to set camera, camera is nullptr!");
		return false;
	}

	_currViewCamera = viewCamera;
	return true;
}
bool Graphics::SetSpotlightCollection(SpotLightCollection *spotlights)
{
	if (spotlights == nullptr)
	{
		ErrMsg("Failed to set spotlight collection, collection is nullptr!");
		return false;
	}

	_currSpotLightCollection = spotlights;
	return true;
}
bool Graphics::SetPointlightCollection(PointLightCollection *pointlights)
{
	if (pointlights == nullptr)
	{
		ErrMsg("Failed to set pointlight collection, collection is nullptr!");
		return false;
	}

	_currPointLightCollection = pointlights;
	return true;
}

DebugDrawer *Graphics::GetDebugDrawer()
{
	return DebugDrawer::GetInstance();
}

void Graphics::ResetLightGrid()
{
	for (UINT i = 0; i < LIGHT_GRID_RES * LIGHT_GRID_RES; i++)
	{
		_lightGrid[i].spotlightCount = 0;
		_lightGrid[i].pointlightCount = 0;
		_lightGrid[i].simpleSpotlightCount = 0;
		_lightGrid[i].simplePointlightCount = 0;
	}
}
void Graphics::AddLightToTile(UINT tileIndex, UINT lightIndex, LightType type)
{
	LightTile &tile = _lightGrid[tileIndex];

	switch (type)
	{
	case SPOTLIGHT:
		if (tile.spotlightCount >= MAX_LIGHTS)
			return;
		
		tile.spotlights[tile.spotlightCount++] = lightIndex;
		break;

	case POINTLIGHT:
		if (tile.pointlightCount >= MAX_LIGHTS * 6)
			return;

		tile.pointlights[tile.pointlightCount++] = lightIndex;
		break;

	case SIMPLE_SPOTLIGHT:
		if (tile.simpleSpotlightCount >= MAX_LIGHTS)
			return;

		tile.simpleSpotlights[tile.simpleSpotlightCount++] = lightIndex;
		break;

	case SIMPLE_POINTLIGHT:
		if (tile.simplePointlightCount >= MAX_LIGHTS)
			return;

		tile.simplePointlights[tile.simplePointlightCount++] = lightIndex;
		break;
	}
}


void Graphics::BeginScreenFade(float duration)
{
	_screenFadeRate = 1.0f / duration;
}
void Graphics::SetScreenFadeManual(float amount)
{
	_screenFadeAmount = amount;
}
float Graphics::GetScreenFadeAmount() const
{
	return _screenFadeAmount;
}
float Graphics::GetScreenFadeRate() const
{
	return _screenFadeRate;
}

void Graphics::SetDistortionOrigin(DirectX::XMFLOAT3A origin)
{
	_distortionSettings.distortionOrigin = origin;
}
void Graphics::SetDistortionStrength(float strength)
{
	_distortionSettings.distortionStrength = strength;
}

void Graphics::SetFogSettings(FogSettingsBuffer fogSettings)
{
	_fogSettings = fogSettings;
}

void Graphics::SetAmbientColor(DirectX::XMFLOAT4A color)
{
	_ambientColor = color;
}

bool Graphics::BeginSceneRender()
{
	if (!_isSetup)
	{
		ErrMsg("Failed to begin rendering, graphics has not been set up!");
		return false;
	}

	if (_isRendering)
	{
		ErrMsg("Failed to begin rendering, rendering has already begun!");
		return false;
	}

	_isRendering = true;
	return true;
}
bool Graphics::EndSceneRender(Time &time)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(684416448, "Graphics Render");
#endif

	if (!_isRendering)
	{
		ErrMsg("Failed to end rendering, rendering has not begun!");
		return false;
	}

	if (!_lightGridBuffer.UpdateBuffer(_context, _lightGrid))
	{
		ErrMsg("Failed to update light grid buffer!");
		return false;
	}

	if (_screenFadeRate != 0.0f)
	{
		_screenFadeAmount += _screenFadeRate * time.deltaTime;

		if (_screenFadeAmount <= 0.0f || _screenFadeAmount >= 1.0f)
		{
			_screenFadeAmount = std::clamp(_screenFadeAmount, 0.0f, 1.0f);
			_screenFadeRate = 0.0f;
		}
	}

	_ambientColor.w = _screenFadeAmount;
	if (!_globalLightBuffer.UpdateBuffer(_context, &_ambientColor))
	{
		ErrMsg("Failed to update global light buffer!");
		return false;
	}

	GeneralData generalData{
		.time = time.time,
		.deltaTime = time.deltaTime,
		.randInt = rand(),
		.randNorm = static_cast<float>(rand()) / static_cast<float>(RAND_MAX)
	};

	if (!_generalDataBuffer.UpdateBuffer(_context, &generalData))
	{
		ErrMsg("Failed to update general data buffer!");
		return false;
	}
	
	if (!_fogSettingsBuffer.UpdateBuffer(_context, &_fogSettings))
	{
		ErrMsg("Failed to update fog settings buffer!");
		return false;
	}
	
	if (!_distortionSettingsBuffer.UpdateBuffer(_context, &_distortionSettings))
	{
		ErrMsg("Failed to update distortion settings buffer!");
		return false;
	}
	
	_context->OMSetDepthStencilState(_ndss.Get(), 0);
	

	// Bind vertex distortion settings
	ID3D11Buffer *const distortionSettings = _distortionSettingsBuffer.GetBuffer();
	_context->VSSetConstantBuffers(2, 1, &distortionSettings);

	// Bind general data for vertex shader
	ID3D11Buffer *const generalDataBuf = _generalDataBuffer.GetBuffer();
	_context->VSSetConstantBuffers(5, 1, &generalDataBuf);

	// Bind noise sampler and texture for vertex shader
	static UINT noiseSamplerID = _content->GetSamplerID("SS_Wrap");
	ID3D11SamplerState *const ss = _content->GetSampler(noiseSamplerID)->GetSamplerState();
	_context->VSSetSamplers(0, 1, &ss);

	static UINT noiseMapID = _content->GetTextureID("Tex_Noise");
	ID3D11ShaderResourceView *srv = _content->GetTexture(noiseMapID)->GetSRV();
	_context->VSSetShaderResources(10, 1, &srv);


	if (!RenderShadowCasters())
	{
		ErrMsg("Failed to render shadow casters!");
		return false;
	}

	// Render main camera to screen view
	_renderOutput = (RenderType)(_renderOutput % RENDER_TYPES);
	if (!RenderToTarget(_sceneRT.GetRTV(), nullptr, nullptr))
	{
		ErrMsg("Failed to render to screen view!");
		return false;
	}

	return true;
}

bool Graphics::RenderToTarget(
	ID3D11RenderTargetView *targetRTV,
	ID3D11DepthStencilView *targetDSV, 
	const D3D11_VIEWPORT *targetViewport)
{
	if (targetRTV == nullptr)		targetRTV = _rtv.Get();
	if (targetDSV == nullptr)		targetDSV = _dsView.Get();
	if (targetViewport == nullptr)	targetViewport = &_viewport;

	switch (_renderOutput)
	{
	case DEFAULT:
		if (!RenderOpaque(_sceneRT.GetRTV(), _depthRT.GetRTV(), targetDSV, targetViewport))
		{
			ErrMsg("Failed to render opaque!");
			return false;
		}
		
		if (_renderTransparency)
		{
			if (!RenderTransparency(_sceneRT.GetRTV(), targetDSV, targetViewport))
			{
				ErrMsg("Failed to render transparency!");
				return false;
			}
		}

		if (!RenderPostFX())
		{
			ErrMsg("Failed to render post fx!");
			return false;
		}

		if (_renderDebugDraw)
		{
			if (!DebugDrawer::GetInstance()->Render(_rtv.Get(), _dsView.Get(), &_viewport))
			{
				ErrMsg("Failed to render debug drawer!");
				return false;
			}
		}
		DebugDrawer::GetInstance()->Clear();

		// Overlay
		if (_renderOverlay)
			if (!RenderOpaque(_rtv.Get(), _depthRT.GetRTV(), targetDSV, targetViewport, true))
			{
				ErrMsg("Failed to render opaque!");
				return false;
			}
		break;

	case POSITION:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewPosition"))
		{
			ErrMsg("Failed to render position view!");
			return false;
		}

		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewPosition", true))
			{
				ErrMsg("Failed to render position view!");
				return false;
			}
		break;
	case NORMAL:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewNormal"))
		{
			ErrMsg("Failed to render normal view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewNormal", true))
			{
				ErrMsg("Failed to render normal view!");
				return false;
			}
		break;
	case AMBIENT:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewAmbient"))
		{
			ErrMsg("Failed to render ambient view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewAmbient", true))
				return false;
		break;
	case DIFFUSE:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewDiffuse"))
		{
			ErrMsg("Failed to render diffuse view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewDiffuse", true))
				return false;
		break;

	case DEPTH:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewDepth"))
		{
			ErrMsg("Failed to render depth view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewDepth", true))
				return false;
		break;

	case LIGHTING:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewLighting"))
		{
			ErrMsg("Failed to render lighting view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewLighting", true))
				return false;
		break;

	case SHADOW:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewShadow"))
		{
			ErrMsg("Failed to render shadow view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewShadow", true))
				return false;
		break;

	case SPECULAR:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewSpecular"))
		{
			ErrMsg("Failed to render specular view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewSpecular", true))
				return false;
		break;

	case SPECULAR_STRENGTH:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewSpecStr"))
		{
			ErrMsg("Failed to render specular strength view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewSpecStr", true))
				return false;
		break;

	case UV_COORDS:
		if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewUVCoords"))
		{
			ErrMsg("Failed to render UV coordinate view!");
			return false;
		}
		if (_renderOverlay)
			if (!RenderCustom(_rtv.Get(), targetDSV, targetViewport, "PS_DebugViewUVCoords", true))
				return false;
		break;

	case TRANSPARENCY:
		_context->ClearRenderTargetView(targetRTV, &_ambientColor.x);

		if (!RenderTransparency(targetRTV, targetDSV, targetViewport))
		{
			ErrMsg("Failed to render transparency view!");
			return false;
		}
		break;

	default:
		ErrMsg("Invalid render type!");
		return false;
	}

	_currInputLayoutID = CONTENT_NULL;
	_currMeshID = CONTENT_NULL;
	_currVsID = CONTENT_NULL;
	_currPsID = CONTENT_NULL;
	_currTexID = CONTENT_NULL;
	_currNormalID = CONTENT_NULL;
	_currSpecularID = CONTENT_NULL;
	_currGlossinessID = CONTENT_NULL;
	_currAmbientID = CONTENT_NULL;
	_currHeightID = CONTENT_NULL;
	_currSamplerID = CONTENT_NULL;

	return true;
}

bool Graphics::RenderSpotlights()
{
	if (!_currSpotLightCollection)
	{
		ErrMsg("Failed to render spotlights, current spotlight collection is nullptr!");
		return false;
	}

	// Used to compare if the mesh uses the distortion shader
	const UINT vsNoDistID = _content->GetShaderID("VS_Geometry");

	const UINT vsDepthID = _content->GetShaderID("VS_Depth");
	const UINT vsDepthDistID = _content->GetShaderID("VS_DepthDistortion");
	if (_currVsID != vsDepthDistID)
	{
		if (!_content->GetShader(vsDepthDistID)->BindShader(_context))
		{
			ErrMsg("Failed to bind depth-stage vertex shader!");
			return false;
		}
		_currVsID = vsDepthDistID;
	}

	_context->RSSetViewports(1, &_currSpotLightCollection->GetViewport());

	_currMeshID = CONTENT_NULL;
	const MeshD3D11 *loadedMesh = nullptr;

	const UINT spotLightCount = _currSpotLightCollection->GetNrOfLights();
	for (UINT spotlight_i = 0; spotlight_i < spotLightCount; spotlight_i++)
	{
		// Skip rendering if disabled
		if (!_currSpotLightCollection->GetLightEnabled(spotlight_i))
			continue;

		// Shadows don't need to update every frame
		if (!_currSpotLightCollection->GetLightBehaviour(spotlight_i)->DoUpdate())
			continue;

		ID3D11DepthStencilView *dsView = _currSpotLightCollection->GetShadowMapDSV(spotlight_i);
		_context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 0.0f, 0);
		_context->OMSetRenderTargets(0, nullptr, dsView);

		// Bind shadow-camera data
		CameraBehaviour *spotlightCamera = _currSpotLightCollection->GetLightBehaviour(spotlight_i)->GetShadowCamera();

		if (!spotlightCamera->BindShadowCasterBuffers())
		{
			ErrMsg(std::format("Failed to bind shadow-camera buffers for spotlight #{}!", spotlight_i));
			return false;
		}

		UINT entity_i = 0;
		for (auto &[resources, instance] : spotlightCamera->GetGeometryQueue())
		{
			MeshBehaviour *meshBehaviour = dynamic_cast<MeshBehaviour *>(instance.subject);

			if (!meshBehaviour)
			{
				ErrMsg(std::format("Skipping depth-rendering for non-mesh #{}!", entity_i));
				return false;
			}

			// Bind shared entity data, skip data irrelevant for shadow mapping
			if (_currMeshID != resources.meshID)
			{
				loadedMesh = _content->GetMesh(resources.meshID);
				if (!loadedMesh->BindMeshBuffers(_context))
				{
					ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", entity_i));
					return false;
				}
				_currMeshID = resources.meshID;
			}
			
			const UINT vsID = resources.material->vsID == vsNoDistID ? vsDepthID : vsDepthDistID;
			if (_currVsID != vsID)
			{
				ShaderD3D11 *vs = _content->GetShader(vsID);
				if (!vs)
				{
					ErrMsg(std::format("Failed to get vertex shader #{} for instance #{}!", vsID, entity_i));
					return false;
				}

				if (!vs->BindShader(_context))
				{
					ErrMsg(std::format("Failed to bind vertex shader #{} for instance #{}!", vsID, entity_i));
					return false;
				}
				_currVsID = vsID;
			}

			// Bind private entity data
			if (!meshBehaviour->InitialBindBuffers())
			{
				ErrMsg(std::format("Failed to bind private buffers for instance #{}!", entity_i));
				return false;
			}

			// Perform draw calls
			if (loadedMesh == nullptr)
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, loadedMesh is nullptr!", entity_i));
				return false;
			}

			const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
			for (UINT submesh_i = 0; submesh_i < subMeshCount; submesh_i++)
			{
				if (!loadedMesh->PerformSubMeshDrawCall(_context, submesh_i))
				{
					ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, submesh_i));
					return false;
				}
			}

			entity_i++;
		}
	}

	return true;
}
bool Graphics::RenderPointlights()
{
	if (!_currPointLightCollection)
	{
		ErrMsg("Failed to render pointlights, current pointlight collection is nullptr!");
		return false;
	}

	const UINT vsID = _content->GetShaderID("VS_Depth");
	const UINT vsDistID = _content->GetShaderID("VS_DepthDistortion");
	if (_currVsID != vsID)
	{
		if (!_content->GetShader(vsID)->BindShader(_context))
		{
			ErrMsg("Failed to bind depth-stage vertex shader!");
			return false;
		}
		_currVsID = vsID;
	}

	_context->RSSetViewports(1, &_currPointLightCollection->GetViewport());

	_currMeshID = CONTENT_NULL;
	const MeshD3D11 *loadedMesh = nullptr;

	const UINT pointlightCount = _currPointLightCollection->GetNrOfLights();
	for (UINT pointlight_i = 0; pointlight_i < pointlightCount; pointlight_i++)
	{
		// Shadows don't need to update every frame
		if (!_currPointLightCollection->GetLightBehaviour(pointlight_i)->DoUpdate())
			continue;

		for (UINT camera_i = 0; camera_i < 6; camera_i++)
		{
			// Skip rendering if disabled
			if (!_currPointLightCollection->GetLightEnabled(pointlight_i, camera_i))
				continue;

			ID3D11DepthStencilView *dsView = _currPointLightCollection->GetShadowMapDSV(pointlight_i, camera_i);
			_context->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 0.0f, 0);
			_context->OMSetRenderTargets(0, nullptr, dsView);

			// Bind shadow-camera data
			CameraCubeBehaviour *pointlightCamera = _currPointLightCollection->GetLightBehaviour(pointlight_i)->GetShadowCameraCube();

			if (!pointlightCamera->BindShadowCasterBuffers(camera_i))
			{
				ErrMsg(std::format("Failed to bind shadow-camera buffers for pointlight #{} camera #{}!", pointlight_i, camera_i));
				return false;
			}

			UINT entity_i = 0;
			for (auto &[resources, instance] : pointlightCamera->GetGeometryQueue(camera_i))
			{
				MeshBehaviour *meshBehaviour = dynamic_cast<MeshBehaviour *>(instance.subject);

				if (!meshBehaviour)
				{
					ErrMsg(std::format("Skipping depth-rendering for non-mesh #{}!", entity_i));
					return false;
				}

				// Bind shared entity data, skip data irrelevant for shadow mapping
				if (_currMeshID != resources.meshID)
				{
					loadedMesh = _content->GetMesh(resources.meshID);
					if (!loadedMesh->BindMeshBuffers(_context))
					{
						ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", entity_i));
						return false;
					}
					_currMeshID = resources.meshID;
				}

				// Bind private entity data
				if (!meshBehaviour->InitialBindBuffers())
				{
					ErrMsg(std::format("Failed to bind private buffers for instance #{}!", entity_i));
					return false;
				}

				// Perform draw calls
				if (loadedMesh == nullptr)
				{
					ErrMsg(std::format("Failed to perform draw call for instance #{}, loadedMesh is nullptr!", entity_i));
					return false;
				}

				const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
				for (UINT submesh_i = 0; submesh_i < subMeshCount; submesh_i++)
				{
					if (!loadedMesh->PerformSubMeshDrawCall(_context, submesh_i))
					{
						ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, submesh_i));
						return false;
					}
				}

				entity_i++;
			}
		}
	}

	return true;
}
bool Graphics::RenderShadowCasters()
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(PIX_COLOR(5, 10, 20), "Graphics Render Shadow Casters");
#endif

	// Bind depth stage resources
	const UINT ilID = _content->GetInputLayoutID("IL_Fallback");
	if (_currInputLayoutID != ilID)
	{
		_context->IASetInputLayout(_content->GetInputLayout(ilID)->GetInputLayout());
		_currInputLayoutID = ilID;
	}

	_context->PSSetShader(nullptr, nullptr, 0);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->RSSetState(_shadowRasterizer.Get());

	if (!RenderSpotlights())
	{
		ErrMsg("Failed to render spotlights!");
		return false;
	}

	if (!RenderPointlights())
	{
		ErrMsg("Failed to render pointlights!");
		return false;
	}

	// Unbind render target
	static constexpr ID3D11RenderTargetView* nullViews [] = { nullptr };
	_context->OMSetRenderTargets(1, nullViews, 0);

	_context->RSSetState(_defaultRasterizer.Get());

	return true;
}

bool Graphics::RenderOpaque(
	ID3D11RenderTargetView *targetSceneRTV,
	ID3D11RenderTargetView *targetDepthRTV,
	ID3D11DepthStencilView *targetDSV,
	const D3D11_VIEWPORT *targetViewport,
	bool overlayStage)
{
	ProjectionInfo proj = _currViewCamera->GetCurrProjectionInfo();
	float farDist = max(proj.planes.nearZ, proj.planes.farZ);

	// Clear & bind render targets
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float clearDepth[4] = { farDist, farDist, farDist, farDist };

	if (!overlayStage) // Skip clearing scene render target if on the overlay-stage
		_context->ClearRenderTargetView(targetSceneRTV, clearColor);

	_context->ClearRenderTargetView(targetDepthRTV, clearDepth);
	_context->ClearDepthStencilView(targetDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);

	ID3D11RenderTargetView *rtvs[2] = { targetSceneRTV, targetDepthRTV };
	_context->OMSetRenderTargets(2, rtvs, targetDSV);

	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	_context->RSSetViewports(1, targetViewport);
	_context->RSSetState(_wireframe ? _wireframeRasterizer.Get() : _defaultRasterizer.Get());

	// Bind camera data
	if (!_currViewCamera->BindViewBuffers())
	{
		ErrMsg("Failed to bind view camera buffers!");
		return false;
	}

	// Bind spotlight collection
	if (!_currSpotLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind spotlight buffers!");
		return false;
	}

	// Bind pointlight collection
	if (!_currPointLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind pointlight buffers!");
		return false;
	}

	// Bind light tile data
	ID3D11ShaderResourceView *const lightTileBuffer = _lightGridBuffer.GetSRV();
	_context->PSSetShaderResources(14, 1, &lightTileBuffer);

	// Bind global light data
	ID3D11Buffer *const globalLightBuffer = _globalLightBuffer.GetBuffer();
	_context->PSSetConstantBuffers(0, 1, &globalLightBuffer);

	// Bind general data
	ID3D11Buffer *const generalData = _generalDataBuffer.GetBuffer();
	_context->PSSetConstantBuffers(5, 1, &generalData);

	static ID3D11SamplerState *const ss = _content->GetSampler("SS_Clamp")->GetSamplerState();
	_context->PSSetSamplers(0, 1, &ss);

	static ID3D11SamplerState *const ssShadow = _content->GetSampler("SS_Shadow")->GetSamplerState();
	_context->PSSetSamplers(1, 1, &ssShadow);

	// Bind camera lighting data
	if (!_currViewCamera->BindPSLightingBuffers())
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	// Bind geometry stage resources
	static UINT geometryInputLayoutID = _content->GetInputLayoutID("IL_Fallback");
	_context->IASetInputLayout(_content->GetInputLayout(geometryInputLayoutID)->GetInputLayout());
	_currInputLayoutID = geometryInputLayoutID;

	if (!RenderGeometry(overlayStage))
	{
		ErrMsg("Failed to render geometry in RenderOpaque()!");
		return false;
	}

	// Unbind tesselation shaders
	_context->HSSetShader(nullptr, nullptr, 0);
	_context->DSSetShader(nullptr, nullptr, 0);

	// Unbind render targets
	static ID3D11RenderTargetView *const nullRTVS[2] = { };
	_context->OMSetRenderTargets(1, nullRTVS, nullptr);

	return true;
}

bool Graphics::RenderGeometry(bool overlayStage, bool skipPixelShader)
{
	static UINT hsID = _content->GetShaderID("HS_LOD");
	if (!_content->GetShader(hsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD hull shader!");
		return false;
	}

	static UINT dsID = _content->GetShaderID("DS_LOD");
	if (!_content->GetShader(dsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD domain shader!");
		return false;
	}

	static UINT defaultVsID = _content->GetShaderID("VS_GeometryDistortion");
	if (!_content->GetShader(defaultVsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind geometry vertex shader!");
		return false;
	}
	_currVsID = defaultVsID;

	static UINT defaultPsID = _content->GetShaderID("PS_Geometry");
	if (!skipPixelShader)
	{
		if (!_content->GetShader(defaultPsID)->BindShader(_context))
		{
			ErrMsg("Failed to bind geometry pixel shader!");
			return false;
		}
		_currPsID = defaultPsID;
	}

	static UINT defaultSamplerID = _content->GetSamplerID("SS_Fallback");
	if (_currSamplerID != defaultSamplerID)
	{
		ID3D11SamplerState *const ss = _content->GetSampler(defaultSamplerID)->GetSamplerState();
		_context->PSSetSamplers(0, 1, &ss);
		_context->DSSetSamplers(0, 1, &ss);
		_currSamplerID = defaultSamplerID;
	}

	const MeshD3D11 *loadedMesh = nullptr;
	_currMeshID = CONTENT_NULL;
	_currNormalID = CONTENT_NULL;
	_currSpecularID = CONTENT_NULL;
	_currGlossinessID = CONTENT_NULL;
	_currLightID = CONTENT_NULL;
	_currOcclusionID = CONTENT_NULL;

	ID3D11ShaderResourceView *srv;

	static UINT defaultAmbientID = _content->GetTextureID("Tex_Ambient");
	srv = _content->GetTexture(defaultAmbientID)->GetSRV();
	_context->PSSetShaderResources(4, 1, &srv);
	_currAmbientID = defaultAmbientID;

	static UINT defaultHeightID = _content->GetTextureMapID("TexMap_Default_Height");
	srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
	_context->DSSetShaderResources(0, 1, &srv);
	_currHeightID = defaultHeightID;

	auto &queue = overlayStage ? _currViewCamera->GetOverlayQueue() : _currViewCamera->GetGeometryQueue();

	UINT entity_i = 0;
	for (auto &[resources, instance] : queue)
	{
		MeshBehaviour *meshBehaviour = dynamic_cast<MeshBehaviour *>(instance.subject);

		if (!meshBehaviour)
		{
			ErrMsg(std::format("Skipping depth-rendering for non-mesh #{}!", entity_i));
			return false;
		}

		// Bind shared geometry resources
		if (_currMeshID != resources.meshID)
		{
			loadedMesh = _content->GetMesh(resources.meshID);
			if (!loadedMesh->BindMeshBuffers(_context))
			{
				ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", entity_i));
				return false;
			}
			_currMeshID = resources.meshID;
		}
		else if (loadedMesh == nullptr)
			loadedMesh = _content->GetMesh(resources.meshID);

		if (_currTexID != resources.material->textureID)
		{
			srv = _content->GetTexture(resources.material->textureID)->GetSRV();
			_context->PSSetShaderResources(0, 1, &srv);
			_currTexID = resources.material->textureID;
		}

		if (resources.material->normalID != CONTENT_NULL)
			if (_currNormalID != resources.material->normalID)
			{
				srv = _content->GetTextureMap(resources.material->normalID)->GetSRV();
				_context->PSSetShaderResources(1, 1, &srv);
				_currNormalID = resources.material->normalID;
			}

		if (resources.material->specularID != CONTENT_NULL)
			if (_currSpecularID != resources.material->specularID)
			{
				srv = _content->GetTextureMap(resources.material->specularID)->GetSRV();
				_context->PSSetShaderResources(2, 1, &srv);
				_currSpecularID = resources.material->specularID;
			}

		if (resources.material->glossinessID != CONTENT_NULL)
			if (_currGlossinessID != resources.material->glossinessID)
			{
				srv = _content->GetTextureMap(resources.material->glossinessID)->GetSRV();
				_context->PSSetShaderResources(9, 1, &srv);
				_currGlossinessID = resources.material->glossinessID;
			}

		if (resources.material->ambientID != CONTENT_NULL)
			if (_currAmbientID != resources.material->ambientID)
			{
				srv = _content->GetTexture(resources.material->ambientID)->GetSRV();
				_context->PSSetShaderResources(4, 1, &srv);
				_currAmbientID = resources.material->ambientID;
			}
		
		if (resources.material->lightID != CONTENT_NULL)
			if (_currLightID != resources.material->lightID)
			{
				srv = _content->GetTexture(resources.material->lightID)->GetSRV();
				_context->PSSetShaderResources(3, 1, &srv);
				_currLightID = resources.material->lightID;
			}
		
		if (resources.material->occlusionID != CONTENT_NULL)
			if (_currOcclusionID != resources.material->occlusionID)
			{
				srv = _content->GetTextureMap(resources.material->occlusionID)->GetSRV();
				_context->PSSetShaderResources(8, 1, &srv);
				_currLightID = resources.material->occlusionID;
			}

		if (resources.material->heightID != CONTENT_NULL)
		{
			if (_currHeightID != resources.material->heightID)
			{
				srv = _content->GetTextureMap(resources.material->heightID)->GetSRV();
				_context->DSSetShaderResources(0, 1, &srv);
				_currHeightID = resources.material->heightID;
			}
		}
		else if (_currHeightID != defaultHeightID)
		{
			srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
			_context->DSSetShaderResources(0, 1, &srv);
			_currHeightID = defaultHeightID;
		}

		if (resources.material->samplerID != CONTENT_NULL)
		{
			if (_currSamplerID != resources.material->samplerID)
			{
				ID3D11SamplerState *const ss = _content->GetSampler(resources.material->samplerID)->GetSamplerState();
				_context->PSSetSamplers(0, 1, &ss);
				_context->DSSetSamplers(0, 1, &ss);
				_currSamplerID = resources.material->samplerID;
			}
		}
		else if (_currSamplerID != defaultSamplerID)
		{
			ID3D11SamplerState *const ss = _content->GetSampler(defaultSamplerID)->GetSamplerState();
			_context->PSSetSamplers(0, 1, &ss);
			_context->DSSetSamplers(0, 1, &ss);
			_currSamplerID = defaultSamplerID;
		}

		if (!skipPixelShader)
		{
			if (resources.material->psID != CONTENT_NULL)
			{
				if (_currPsID != resources.material->psID)
				{
					ShaderD3D11 *ps = _content->GetShader(resources.material->psID);
					if (!ps)
					{
						ErrMsg(std::format("Failed to get pixel shader #{}!", resources.material->psID));
						return false;
					}

					if (!ps->BindShader(_context))
					{
						ErrMsg(std::format("Failed to bind pixel shader #{}!", resources.material->psID));
						return false;
					}
					_currPsID = resources.material->psID;
				}
			}
			else if (_currPsID != defaultPsID)
			{
				if (!_content->GetShader(defaultPsID)->BindShader(_context))
				{
					ErrMsg("Failed to bind default pixel shader!");
					return false;
				}
				_currPsID = defaultPsID;
			}
		}

		if (resources.material->vsID != CONTENT_NULL)
		{
			if (_currVsID != resources.material->vsID)
			{
				ShaderD3D11 *vs = _content->GetShader(resources.material->vsID);
				if (!vs)
				{
					ErrMsg(std::format("Failed to get vertex shader #{}!", resources.material->vsID));
					return false;
				}

				if (!vs->BindShader(_context))
				{
					ErrMsg(std::format("Failed to bind vertex shader #{}!", resources.material->vsID));
					return false;
				}
				_currVsID = resources.material->vsID;
			}
		}
		else if (_currVsID != defaultVsID)
		{
			if (!_content->GetShader(defaultVsID)->BindShader(_context))
			{
				ErrMsg("Failed to bind default vertex shader!");
				return false;
			}
			_currVsID = defaultVsID;
		}

		// Bind private entity resources
		if (!meshBehaviour->InitialBindBuffers())
		{
			ErrMsg(std::format("Failed to bind private buffers for instance #{}!", entity_i));
			return false;
		}

		// Perform draw calls
		if (loadedMesh == nullptr)
		{
			ErrMsg(std::format("Failed to perform draw call for instance #{}, loadedMesh is nullptr!", entity_i));
			return false;
		}

		const UINT
			prevTexID = _currTexID,
			prevAmbientID = _currAmbientID,
			prevSpecularID = _currSpecularID;

		const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
		for (UINT i = 0; i < subMeshCount; i++)
		{
			// Bind sub-mesh material textures if defined
			std::string path = loadedMesh->GetDiffusePath(i);
			if (path != "")
			{
				const UINT id = _content->GetTextureIDByPath(path);
				if (id != CONTENT_NULL && id != _currTexID)
				{
					srv = _content->GetTexture(id)->GetSRV();
					_context->PSSetShaderResources(0, 1, &srv);
					_currTexID = id;
				}
			}
			else if (prevTexID != _currTexID && prevTexID != CONTENT_NULL)
			{
				srv = _content->GetTexture(prevTexID)->GetSRV();
				_context->PSSetShaderResources(0, 1, &srv);
				_currTexID = prevTexID;
			}

			path = loadedMesh->GetAmbientPath(i);
			if (path != "")
			{
				const UINT id = _content->GetTextureIDByPath(path);
				if (id != CONTENT_NULL && id != _currAmbientID)
				{
					srv = _content->GetTexture(id)->GetSRV();
					_context->PSSetShaderResources(4, 1, &srv);
					_currAmbientID = id;
				}
			}
			else if (prevAmbientID != _currAmbientID && prevAmbientID != CONTENT_NULL)
			{
				srv = _content->GetTexture(prevAmbientID)->GetSRV();
				_context->PSSetShaderResources(4, 1, &srv);
				_currAmbientID = prevAmbientID;
			}

			path = loadedMesh->GetSpecularPath(i);
			if (path != "")
			{
				const UINT id = _content->GetTextureMapIDByPath(path, TextureType::SPECULAR);
				if (id != CONTENT_NULL && id != _currSpecularID)
				{
					srv = _content->GetTextureMap(id)->GetSRV();
					_context->PSSetShaderResources(2, 1, &srv);
					_currSpecularID = id;
				}
			}
			else if (prevSpecularID != _currSpecularID && prevSpecularID != CONTENT_NULL)
			{
				srv = _content->GetTextureMap(prevSpecularID)->GetSRV();
				_context->PSSetShaderResources(2, 1, &srv);
				_currSpecularID = prevSpecularID;
			}

			ID3D11Buffer *const specularBuffer = loadedMesh->GetSpecularBuffer(i);
			_context->PSSetConstantBuffers(1, 1, &specularBuffer);

			if (!loadedMesh->PerformSubMeshDrawCall(_context, i))
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, i));
				return false;
			}
		}

		entity_i++;
	}

	return true;
}

bool Graphics::RenderCustom(
	ID3D11RenderTargetView *targetRTV,
	ID3D11DepthStencilView *targetDSV,
	const D3D11_VIEWPORT *targetViewport,
	const std::string &pixelShader, 
	bool overlayStage)
{
	// Clear & bind render targets
	constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (!overlayStage) // Skip clearing scene render target if on the overlay-stage
		_context->ClearRenderTargetView(targetRTV, clearColor);

	_context->ClearDepthStencilView(targetDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);
	_context->OMSetRenderTargets(1, &targetRTV, targetDSV);

	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	_context->RSSetViewports(1, &_viewport);
	_context->RSSetState(_wireframe ? _wireframeRasterizer.Get() : _defaultRasterizer.Get());

	// Bind camera data
	if (!_currViewCamera->BindViewBuffers())
	{
		ErrMsg("Failed to bind view camera buffers!");
		return false;
	}

	// Bind spotlight collection
	if (!_currSpotLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind spotlight buffers!");
		return false;
	}

	// Bind pointlight collection
	if (!_currPointLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind pointlight buffers!");
		return false;
	}

	// Bind light tile data
	ID3D11ShaderResourceView *const lightTileBuffer = _lightGridBuffer.GetSRV();
	_context->PSSetShaderResources(14, 1, &lightTileBuffer);

	// Bind global light data
	ID3D11Buffer *const globalLightBuffer = _globalLightBuffer.GetBuffer();
	_context->PSSetConstantBuffers(0, 1, &globalLightBuffer);

	// Bind general data
	ID3D11Buffer *const generalData = _generalDataBuffer.GetBuffer();
	_context->PSSetConstantBuffers(5, 1, &generalData);

	static ID3D11SamplerState *const ss = _content->GetSampler("SS_Clamp")->GetSamplerState();
	_context->PSSetSamplers(0, 1, &ss);

	static ID3D11SamplerState *const ssShadow = _content->GetSampler("SS_Shadow")->GetSamplerState();
	_context->PSSetSamplers(1, 1, &ssShadow);

	// Bind camera lighting data
	if (!_currViewCamera->BindPSLightingBuffers())
	{
		ErrMsg("Failed to bind camera buffers!");
		return false;
	}

	// Bind geometry stage resources
	static UINT geometryInputLayoutID = _content->GetInputLayoutID("IL_Fallback");
	_context->IASetInputLayout(_content->GetInputLayout(geometryInputLayoutID)->GetInputLayout());
	_currInputLayoutID = geometryInputLayoutID;

	if (!_content->GetShader(pixelShader)->BindShader(_context))
	{
		ErrMsg(std::format("Failed to bind pixel shader!"));
		return false;
	}

	if (!RenderGeometry(overlayStage, true))
	{
		ErrMsg("Failed to render geometry!");
		return false;
	}

	// Unbind pointlight collection
	if (!_currPointLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind pointlight buffers!");
		return false;
	}

	// Unbind spotlight collection
	if (!_currSpotLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind spotlight buffers!");
		return false;
	}

	// Unbind tesselation shaders
	_context->HSSetShader(nullptr, nullptr, 0);
	_context->DSSetShader(nullptr, nullptr, 0);

	// Unbind render target
	static ID3D11RenderTargetView *const nullRTV = nullptr;
	_context->OMSetRenderTargets(1, &nullRTV, nullptr);

	return true;
}

bool Graphics::RenderTransparency(
	ID3D11RenderTargetView *targetRTV,
	ID3D11DepthStencilView *targetDSV,
	const D3D11_VIEWPORT *targetViewport)
{
	_context->OMSetDepthStencilState(_tdss.Get(), 0);

	ID3D11BlendState *prevBlendState;
	FLOAT prevBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT prevSampleMask = 0;
	_context->OMGetBlendState(&prevBlendState, prevBlendFactor, &prevSampleMask);

	constexpr float transparentBlendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_context->OMSetBlendState(_tbs.Get(), transparentBlendFactor, 0xffffffff);

	_context->OMSetRenderTargets(1, &targetRTV, targetDSV);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST); // If no tessellation: D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	_context->RSSetViewports(1, targetViewport);
	_context->RSSetState(_wireframe ? _wireframeRasterizer.Get() : _defaultRasterizer.Get());

	// Bind camera data
	if (!_currViewCamera->BindViewBuffers())
	{
		ErrMsg("Failed to bind view camera buffers!");
		return false;
	}

	// Bind billboard camera data
	if (!_currViewCamera->BindTransparentBuffers())
	{
		ErrMsg("Failed to bind billboard camera buffers!");
		return false;
	}

	// Bind transparency stage resources
	static UINT transparencyInputLayoutID = _content->GetInputLayoutID("IL_Fallback");
	if (_currInputLayoutID != transparencyInputLayoutID)
	{
		_context->IASetInputLayout(_content->GetInputLayout(transparencyInputLayoutID)->GetInputLayout());
		_currInputLayoutID = transparencyInputLayoutID;
	}

	static UINT vsID = _content->GetShaderID("VS_Geometry");
	if (_currVsID != vsID)
	{
		if (!_content->GetShader(vsID)->BindShader(_context))
		{
			ErrMsg("Failed to bind geometry vertex shader!");
			return false;
		}
		_currVsID = vsID;
	}

	static UINT hsID = _content->GetShaderID("HS_LOD");
	if (!_content->GetShader(hsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD hull shader!");
		return false;
	}

	static UINT dsID = _content->GetShaderID("DS_LOD");
	if (!_content->GetShader(dsID)->BindShader(_context))
	{
		ErrMsg("Failed to bind LOD domain shader!");
		return false;
	}

	static UINT psID = _content->GetShaderID("PS_Transparent");
	if (_currPsID != psID)
	{
		if (!_content->GetShader(psID)->BindShader(_context))
		{
			ErrMsg("Failed to bind transparent pixel shader!");
			return false;
		}
		_currPsID = psID;
	}

	static UINT defaultSamplerID = _content->GetSamplerID("SS_Clamp");
	if (_currSamplerID != defaultSamplerID)
	{
		ID3D11SamplerState *const ss = _content->GetSampler(defaultSamplerID)->GetSamplerState();
		_context->PSSetSamplers(0, 1, &ss);
		_context->DSSetSamplers(0, 1, &ss);
		_currSamplerID = defaultSamplerID;
	}

	// Bind global light data
	ID3D11Buffer *const globalLightBuffer = _globalLightBuffer.GetBuffer();
	_context->PSSetConstantBuffers(0, 1, &globalLightBuffer);

	// Bind general data
	ID3D11Buffer *const generalData = _generalDataBuffer.GetBuffer();
	_context->PSSetConstantBuffers(5, 1, &generalData);

	// Bind light tile data
	ID3D11ShaderResourceView *const lightTileBuffer = _lightGridBuffer.GetSRV();
	_context->PSSetShaderResources(14, 1, &lightTileBuffer);

	// Bind spotlight collection
	if (!_currSpotLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind spotlight buffers!");
		return false;
	}

	// Bind pointlight collection
	if (!_currPointLightCollection->BindPSBuffers(_context))
	{
		ErrMsg("Failed to bind pointlight buffers!");
		return false;
	}

	static UINT defaultNormalID = _content->GetTextureMapID("TexMap_Default_Normal");
	if (_currNormalID != defaultNormalID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultNormalID)->GetSRV();
		_context->PSSetShaderResources(1, 1, &srv);
		_currNormalID = defaultNormalID;
	}

	static UINT defaultSpecularID = _content->GetTextureMapID("TexMap_Default_Specular");
	if (_currSpecularID != defaultSpecularID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultSpecularID)->GetSRV();
		_context->PSSetShaderResources(2, 1, &srv);
		_currSpecularID = defaultSpecularID;
	}
	
	static UINT defaultAmbientID = _content->GetTextureID("Tex_Ambient");
	if (_currAmbientID != defaultAmbientID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTexture(defaultAmbientID)->GetSRV();
		_context->PSSetShaderResources(4, 1, &srv);
		_currAmbientID = defaultAmbientID;
	}

	static UINT defaultHeightID = _content->GetTextureMapID("TexMap_Default_Height");
	if (_currHeightID != defaultHeightID)
	{
		ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
		_context->DSSetShaderResources(0, 1, &srv);
		_currHeightID = defaultHeightID;
	}

	_currMeshID = CONTENT_NULL;
	const MeshD3D11 *loadedMesh = nullptr;

	UINT entity_i = 0;
	for (auto &[resources, instance] : _currViewCamera->GetTransparentQueue())
	{
		MeshBehaviour *meshBehaviour = dynamic_cast<MeshBehaviour *>(instance.subject);

		if (!meshBehaviour)
		{
			ErrMsg(std::format("Skipping depth-rendering for non-mesh #{}!", entity_i));
			return false;
		}

		// Bind shared geometry resources
		if (_currMeshID != resources.meshID)
		{
			loadedMesh = _content->GetMesh(resources.meshID);
			if (!loadedMesh->BindMeshBuffers(_context))
			{
				ErrMsg(std::format("Failed to bind mesh buffers for instance #{}!", entity_i));
				return false;
			}
			_currMeshID = resources.meshID;
		}
		else if (loadedMesh == nullptr)
			loadedMesh = _content->GetMesh(resources.meshID);

		if (_currTexID != resources.material->textureID)
		{
			ID3D11ShaderResourceView *const srv = _content->GetTexture(resources.material->textureID)->GetSRV();
			_context->PSSetShaderResources(0, 1, &srv);
			_currTexID = resources.material->textureID;
		}

		if (resources.material->normalID != CONTENT_NULL)
			if (_currNormalID != resources.material->normalID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.material->normalID)->GetSRV();
				_context->PSSetShaderResources(1, 1, &srv);
				_currNormalID = resources.material->normalID;
			}

		if (resources.material->specularID != CONTENT_NULL)
			if (_currSpecularID != resources.material->specularID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.material->specularID)->GetSRV();
				_context->PSSetShaderResources(2, 1, &srv);
				_currSpecularID = resources.material->specularID;
			}
		
		if (resources.material->ambientID != CONTENT_NULL)
			if (_currAmbientID != resources.material->ambientID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTexture(resources.material->ambientID)->GetSRV();
				_context->PSSetShaderResources(4, 1, &srv);
				_currAmbientID = resources.material->ambientID;
			}

		if (resources.material->heightID != CONTENT_NULL)
		{
			if (_currHeightID != resources.material->heightID)
			{
				ID3D11ShaderResourceView *const srv = _content->GetTextureMap(resources.material->heightID)->GetSRV();
				_context->DSSetShaderResources(0, 1, &srv);
				_currHeightID = resources.material->heightID;
			}
		}
		else if (_currHeightID != defaultHeightID)
		{
			ID3D11ShaderResourceView *const srv = _content->GetTextureMap(defaultHeightID)->GetSRV();
			_context->DSSetShaderResources(0, 1, &srv);
			_currHeightID = defaultHeightID;
		}

		if (resources.material->heightID != CONTENT_NULL)
		{
			if (_currSamplerID != resources.material->samplerID)
			{
				ID3D11SamplerState *const ss = _content->GetSampler(resources.material->samplerID)->GetSamplerState();
				_context->PSSetSamplers(0, 1, &ss);
				_context->DSSetSamplers(0, 1, &ss);
				_currSamplerID = resources.material->samplerID;
			}
		}
		else if (_currSamplerID != defaultSamplerID)
		{
			ID3D11SamplerState *const ss = _content->GetSampler(defaultSamplerID)->GetSamplerState();
			_context->PSSetSamplers(0, 1, &ss);
			_context->DSSetSamplers(0, 1, &ss);
			_currSamplerID = defaultSamplerID;
		}

		// Bind private entity resources
		if (!meshBehaviour->InitialBindBuffers())
		{
			ErrMsg(std::format("Failed to bind private buffers for instance #{}!", entity_i));
			return false;
		}

		// Perform draw calls
		if (loadedMesh == nullptr)
		{
			ErrMsg(std::format("Failed to perform draw call for instance #{}, loadedMesh is nullptr!", entity_i));
			return false;
		}

		const UINT subMeshCount = loadedMesh->GetNrOfSubMeshes();
		for (UINT i = 0; i < subMeshCount; i++)
		{
			ID3D11Buffer *const specularBuffer = loadedMesh->GetSpecularBuffer(i);
			_context->PSSetConstantBuffers(1, 1, &specularBuffer);

			if (!loadedMesh->PerformSubMeshDrawCall(_context, i))
			{
				ErrMsg(std::format("Failed to perform draw call for instance #{}, sub mesh #{}!", entity_i, i));
				return false;
			}
		}

		entity_i++;
	}

	// Unbind tesselation shaders
	_context->HSSetShader(nullptr, nullptr, 0);
	_context->DSSetShader(nullptr, nullptr, 0);

	// Unbind pointlight collection
	if (!_currPointLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind pointlight buffers!");
		return false;
	}

	// Unbind spotlight collection
	if (!_currSpotLightCollection->UnbindPSBuffers(_context))
	{
		ErrMsg("Failed to unbind spotlight buffers!");
		return false;
	}

	// Reset blend state
	_context->OMSetBlendState(prevBlendState, prevBlendFactor, prevSampleMask);
	_context->OMSetDepthStencilState(_ndss.Get(), 0);

	// Unbind render target
	static ID3D11RenderTargetView *const nullRTV = nullptr;
	_context->OMSetRenderTargets(1, &nullRTV, nullptr);

	return true;
}

bool Graphics::RenderPostFX()
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(147645345, "Graphics Render Post-Processing");
#endif

	static ID3D11SamplerState *const ss = _content->GetSampler("SS_Clamp")->GetSamplerState();
	_context->CSSetSamplers(0, 1, &ss); // Verify slot

	// Bind global light data
	ID3D11Buffer *const globalLightBuffer = _globalLightBuffer.GetBuffer();
	_context->CSSetConstantBuffers(0, 1, &globalLightBuffer);

	if (_renderPostFX)
	{
		// Perform Fog
		{
			// Bind distortion settings
			ID3D11Buffer *const distortionSettings = _distortionSettingsBuffer.GetBuffer();
			_context->CSSetConstantBuffers(2, 1, &distortionSettings);

			// Bind fog settings
			ID3D11Buffer *const fogSettings = _fogSettingsBuffer.GetBuffer();
			_context->CSSetConstantBuffers(6, 1, &fogSettings);

			// Bind general data
			ID3D11Buffer *const generalData = _generalDataBuffer.GetBuffer();
			_context->CSSetConstantBuffers(5, 1, &generalData);
			
			// Bind light tile data
			ID3D11ShaderResourceView *const lightTileBuffer = _lightGridBuffer.GetSRV();
			_context->CSSetShaderResources(14, 1, &lightTileBuffer);

			// Bind fog compute shader
			if (!_content->GetShader("CS_FogFX")->BindShader(_context))
			{
				ErrMsg(std::format("Failed to bind fog compute shader!"));
				return false;
			}

			// Bind fog render target
			ID3D11UnorderedAccessView *const uav[1] = { _fogRT.GetUAV() };
			_context->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

			// Bind depth resource
			ID3D11ShaderResourceView *const srv[1] = { _depthRT.GetSRV() };
			_context->CSSetShaderResources(0, 1, srv);

			// Bind spotlight collection
			if (!_currSpotLightCollection->BindCSBuffers(_context))
			{
				ErrMsg("Failed to bind spotlight buffers!");
				return false;
			}

			// Bind pointlight collection
			if (!_currPointLightCollection->BindCSBuffers(_context))
			{
				ErrMsg("Failed to bind pointlight buffers!");
				return false;
			}

			// Bind shadow sampler
			static ID3D11SamplerState *const ssShadow = _content->GetSampler("SS_Shadow")->GetSamplerState();
			_context->CSSetSamplers(1, 1, &ssShadow);

			// Bind camera lighting data
			if (!_currViewCamera->BindCSLightingBuffers())
			{
				ErrMsg("Failed to bind camera buffers!");
				return false;
			}
		
			// Bind camera inverse view data
			if (!_currViewCamera->BindInverseBuffers())
			{
				ErrMsg("Failed to bind inverse camera buffers!");
				return false;
			}


			// Send execution command
			_context->Dispatch(static_cast<UINT>(ceil(_viewportFog.Width / 8.0f)), static_cast<UINT>(ceil(_viewportFog.Height / 8.0f)), 1);


			// Unbind pointlight collection
			if (!_currPointLightCollection->UnbindCSBuffers(_context))
			{
				ErrMsg("Failed to unbind pointlight buffers!");
				return false;
			}

			// Unbind spotlight collection
			if (!_currSpotLightCollection->UnbindCSBuffers(_context))
			{
				ErrMsg("Failed to unbind spotlight buffers!");
				return false;
			}

			// Unbind compute shader resources
			ID3D11ShaderResourceView *nullSRV[1] = {};
			memset(nullSRV, 0, sizeof(ID3D11ShaderResourceView));
			_context->CSSetShaderResources(0, 1, nullSRV);

			// Unbind render target
			static ID3D11UnorderedAccessView *const nullUAV = nullptr;
			_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
		}

		// Perform Fog Blur
		if (_blurIterations > 0)
		{
			// Bind depth resource
			ID3D11ShaderResourceView *const depthSRV[1] = { _depthRT.GetSRV() };
			_context->CSSetShaderResources(1, 1, depthSRV);

			for (int i = 0; i < _blurIterations; i++)
			{
				ID3D11UnorderedAccessView *uavStageOne = _IntermediaryRT.GetUAV();
				ID3D11ShaderResourceView *srvStageOne = _fogRT.GetSRV();

				ID3D11UnorderedAccessView *uavStageTwo = _fogRT.GetUAV();
				ID3D11ShaderResourceView *srvStageTwo = _IntermediaryRT.GetSRV();

				// Blur Stage One
				{
					// Bind compute shader
					if (!_content->GetShader("CS_BlurHorizontalFX")->BindShader(_context))
					{
						ErrMsg(std::format("Failed to bind horizontal blur compute shader!"));
						return false;
					}

					// Bind render target
					ID3D11UnorderedAccessView *const uav[1] = { uavStageOne };
					_context->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

					// Bind shader resource
					ID3D11ShaderResourceView *const srv[1] = { srvStageOne };
					_context->CSSetShaderResources(0, 1, srv);


					// Send execution command
					_context->Dispatch(static_cast<UINT>(ceil(_viewportBlur.Width / 8.0f)), static_cast<UINT>(ceil(_viewportBlur.Height / 8.0f)), 1);


					// Unbind compute shader resources
					ID3D11ShaderResourceView *nullSRV[1] = {};
					memset(nullSRV, 0, sizeof(ID3D11ShaderResourceView));
					_context->CSSetShaderResources(0, 1, nullSRV);

					// Unbind render target
					static ID3D11UnorderedAccessView *const nullUAV = nullptr;
					_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
				}

				// Blur Stage Two
				{
					// Bind compute shader
					if (!_content->GetShader("CS_BlurVerticalFX")->BindShader(_context))
					{
						ErrMsg(std::format("Failed to bind vertical blur compute shader!"));
						return false;
					}

					// Bind render target
					ID3D11UnorderedAccessView *const uav[1] = { uavStageTwo };
					_context->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

					// Bind shader resource
					ID3D11ShaderResourceView *const srv[1] = { srvStageTwo };
					_context->CSSetShaderResources(0, 1, srv);


					// Send execution command
					_context->Dispatch(static_cast<UINT>(ceil(_viewportFog.Width / 8.0f)), static_cast<UINT>(ceil(_viewportFog.Height / 8.0f)), 1);


					// Unbind compute shader resources
					ID3D11ShaderResourceView *nullSRV[1] = {};
					memset(nullSRV, 0, sizeof(ID3D11ShaderResourceView));
					_context->CSSetShaderResources(0, 1, nullSRV);

					// Unbind render target
					static ID3D11UnorderedAccessView *const nullUAV = nullptr;
					_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
				}
			}

			// Unbind depth resource
			ID3D11ShaderResourceView *nullSRV[1] = {};
			memset(nullSRV, 0, sizeof(ID3D11ShaderResourceView));
			_context->CSSetShaderResources(1, 1, nullSRV);
		}

		// Perform Emission Blur
		if (_blurIterations > 0)
		{
			// Bind depth resource
			ID3D11ShaderResourceView *const depthSRV[1] = { _depthRT.GetSRV() };
			_context->CSSetShaderResources(1, 1, depthSRV);

			for (int i = 0; i < _blurIterations; i++)
			{
				ID3D11UnorderedAccessView *uavStageOne = _IntermediaryRT.GetUAV();
				ID3D11ShaderResourceView *srvStageOne = (i <= 0) ? _sceneRT.GetSRV() : _blurRT.GetSRV();

				ID3D11UnorderedAccessView *uavStageTwo = _blurRT.GetUAV();
				ID3D11ShaderResourceView *srvStageTwo = _IntermediaryRT.GetSRV();

				// Blur Stage One
				{
					// Bind compute shader
					if (!_content->GetShader("CS_BlurHorizontalFX")->BindShader(_context))
					{
						ErrMsg(std::format("Failed to bind horizontal blur compute shader!"));
						return false;
					}

					// Bind render target
					ID3D11UnorderedAccessView *const uav[1] = { uavStageOne };
					_context->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

					// Bind shader resource
					ID3D11ShaderResourceView *const srv[1] = { srvStageOne };
					_context->CSSetShaderResources(0, 1, srv);


					// Send execution command
					_context->Dispatch(static_cast<UINT>(ceil(_viewportBlur.Width / 8.0f)), static_cast<UINT>(ceil(_viewportBlur.Height / 8.0f)), 1);


					// Unbind compute shader resources
					ID3D11ShaderResourceView *nullSRV[1] = {};
					memset(nullSRV, 0, sizeof(ID3D11ShaderResourceView));
					_context->CSSetShaderResources(0, 1, nullSRV);

					// Unbind render target
					static ID3D11UnorderedAccessView *const nullUAV = nullptr;
					_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
				}

				// Blur Stage Two
				{
					// Bind compute shader
					if (!_content->GetShader("CS_BlurVerticalFX")->BindShader(_context))
					{
						ErrMsg(std::format("Failed to bind vertical blur compute shader!"));
						return false;
					}

					// Bind render target
					ID3D11UnorderedAccessView *const uav[1] = { uavStageTwo };
					_context->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

					// Bind shader resource
					ID3D11ShaderResourceView *const srv[1] = { srvStageTwo };
					_context->CSSetShaderResources(0, 1, srv);


					// Send execution command
					_context->Dispatch(static_cast<UINT>(ceil(_viewportBlur.Width / 8.0f)), static_cast<UINT>(ceil(_viewportBlur.Height / 8.0f)), 1);


					// Unbind compute shader resources
					ID3D11ShaderResourceView *nullSRV[1] = {};
					memset(nullSRV, 0, sizeof(ID3D11ShaderResourceView));
					_context->CSSetShaderResources(0, 1, nullSRV);

					// Unbind render target
					static ID3D11UnorderedAccessView *const nullUAV = nullptr;
					_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
				}
			}

			// Unbind depth resource
			ID3D11ShaderResourceView *nullSRV[1] = {};
			memset(nullSRV, 0, sizeof(ID3D11ShaderResourceView));
			_context->CSSetShaderResources(1, 1, nullSRV);
		}
	}

	// Combine
	{
		// Bind combine compute shader
		if (!_content->GetShader("CS_CombineFX")->BindShader(_context))
		{
			ErrMsg(std::format("Failed to bind fog compute shader!"));
			return false;
		}

		// Bind combine render target
		ID3D11UnorderedAccessView *const uav[1] = { _uav.Get() };
		_context->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

		// Bind screen, emission & fog resources
		ID3D11ShaderResourceView *srvs[3] = { 
			_sceneRT.GetSRV(),
			_blurRT.GetSRV(),
			_fogRT.GetSRV(),
		};
		_context->CSSetShaderResources(0, 3, srvs);


		// Send execution command
		_context->Dispatch(static_cast<UINT>(ceil(_viewport.Width / 8.0f)), static_cast<UINT>(ceil(_viewport.Height / 8.0f)), 1);

		memset(srvs, 0, sizeof(srvs));
		_context->CSSetShaderResources(0, 3, srvs);

		// Unbind render target
		static ID3D11UnorderedAccessView *const nullUAV = nullptr;
		_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	}

	return true;
}

#ifdef USE_IMGUI
bool Graphics::BeginUIRender() const
{
	_context->OMSetRenderTargets(1, _rtv.GetAddressOf(), _dsView.Get());

	ImGuiIO &io = ImGui::GetIO();
	io.NavActive = false;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs);

	return true;
}
bool Graphics::RenderUI(Time &time)
{
#ifdef PIX_TIMELINING
	PIXScopedEvent(484768168, "Graphics Render UI");
#endif

	std::string currRenderOutput;
	switch (_renderOutput)
	{
		case DEFAULT:				currRenderOutput = "Default";			break;
		case POSITION:				currRenderOutput = "Position";			break;
		case NORMAL:				currRenderOutput = "Normal";			break;
		case AMBIENT:				currRenderOutput = "Ambient";			break;
		case DIFFUSE:				currRenderOutput = "Diffuse";			break;
		case DEPTH:					currRenderOutput = "Depth";				break;
		case LIGHTING:				currRenderOutput = "Lighting";			break;
		case SHADOW:				currRenderOutput = "Shadow";			break;
		case SPECULAR:				currRenderOutput = "Specular";			break;
		case SPECULAR_STRENGTH:		currRenderOutput = "Specular Strength";	break;
		case UV_COORDS:				currRenderOutput = "UV Coordinates";	break;
		case TRANSPARENCY:			currRenderOutput = "Transparency";		break;
		default:					currRenderOutput = "Invalid";			break;
	}

	// Set ambient color
	{
		float color[3] = { _ambientColor.x, _ambientColor.y, _ambientColor.z };

		if (ImGui::ColorEdit3("Color", color))
		{
			_ambientColor.x = color[0];
			_ambientColor.y = color[1];
			_ambientColor.z = color[2];
		}
	}
	
	// Set vertex distortion effect strength
	static DirectX::XMFLOAT3 dO = { 0, 0, 0 };
	ImGui::InputFloat("Distortion Strength", &_distortionSettings.distortionStrength);
	ImGui::InputFloat3("Distortion Origin", &dO.x);
	_distortionSettings.distortionOrigin = dO;

	if (ImGui::Button(std::format("Render Output: {}", currRenderOutput).c_str()))
		_renderOutput = (RenderType)((int)_renderOutput + 1);

	if (ImGui::Button(std::format("Wireframe Mode: {}", _wireframe ? "Enabled" : "Disabled").c_str()))
		_wireframe = !_wireframe;

	if (ImGui::Button(std::format("Transparency: {}", _renderTransparency ? "Enabled" : "Disabled").c_str()))
		_renderTransparency = !_renderTransparency;

	if (ImGui::Button(std::format("V-Sync: {}", _vSync ? "Enabled" : "Disabled").c_str()))
		_vSync = !_vSync;
	
	ImGui::Checkbox("Debug Drawing", &_renderDebugDraw);

	if (ImGui::Checkbox("Post Processing", &_renderPostFX))
	{
		if (!_renderPostFX)
		{
			// Clear post processing resources
			constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			_context->ClearRenderTargetView(_blurRT.GetRTV(), clearColor);
			_context->ClearRenderTargetView(_fogRT.GetRTV(), clearColor);
		}
	}

	if (_renderPostFX)
	{
		if (ImGui::TreeNode("Post Processing Settings"))
		{
			ImGui::SeparatorText("Volumetric Fog");
			ImGui::DragFloat("Thickness", &_fogSettings.thickness, 0.01f);
			ImGui::DragFloat("Step Size", &_fogSettings.stepSize, 0.01f);

			int minSteps = (int)_fogSettings.minSteps;
			if (ImGui::DragInt("Min Samples", &minSteps, 1, 0, (int)_fogSettings.maxSteps))
				_fogSettings.minSteps = (UINT)minSteps;

			int maxSteps = (int)_fogSettings.maxSteps;
			if (ImGui::DragInt("Max Samples", &maxSteps, 1, minSteps))
				_fogSettings.maxSteps = (UINT)maxSteps;

			ImGui::SeparatorText("Blur");
			ImGui::DragInt("Blur Iterations", &_blurIterations, 1, 0, 24);

			ImGui::TreePop();
		}
	}
	
	if (ImGui::TreeNode("Shadow Rasterization"))
	{
		bool hasChanged = false;

		int fillMode = (int)_shadowRasterizerDesc.FillMode - 2;
		int cullMode = (int)_shadowRasterizerDesc.CullMode - 1;
		bool frontCounterClockwise = _shadowRasterizerDesc.FrontCounterClockwise;
		int depthBias = _shadowRasterizerDesc.DepthBias;
		float depthBiasClamp = _shadowRasterizerDesc.DepthBiasClamp;
		float slopeScaledDepthBias = _shadowRasterizerDesc.SlopeScaledDepthBias;
		bool depthClipEnable = _shadowRasterizerDesc.DepthClipEnable;
		bool scissorEnable = _shadowRasterizerDesc.ScissorEnable;
		bool multisampleEnable = _shadowRasterizerDesc.MultisampleEnable;
		bool antialiasedLineEnable = _shadowRasterizerDesc.AntialiasedLineEnable;

		if (ImGui::Combo("Fill Mode", &fillMode, "Wireframe\0Solid\0"))
		{
			_shadowRasterizerDesc.FillMode = (D3D11_FILL_MODE)(fillMode + 2);
			hasChanged = true;
		}

		if (ImGui::Combo("Cull Mode", &cullMode, "None\0Front\0Back\0"))
		{
			_shadowRasterizerDesc.CullMode = (D3D11_CULL_MODE)(cullMode + 1);
			hasChanged = true;
		}

		if (ImGui::Checkbox("Front Counter Clockwise", &frontCounterClockwise))
		{
			_shadowRasterizerDesc.FrontCounterClockwise = frontCounterClockwise;
			hasChanged = true;
		}

		if (ImGui::InputInt("Depth Bias", &depthBias))
		{
			_shadowRasterizerDesc.DepthBias = depthBias;
			hasChanged = true;
		}

		if (ImGui::InputFloat("Depth Bias Clamp", &depthBiasClamp))
		{
			_shadowRasterizerDesc.DepthBiasClamp = depthBiasClamp;
			hasChanged = true;
		}

		if (ImGui::InputFloat("Slope Scaled Depth Bias", &slopeScaledDepthBias))
		{
			_shadowRasterizerDesc.SlopeScaledDepthBias = slopeScaledDepthBias;
			hasChanged = true;
		}

		if (ImGui::Checkbox("Depth Clip", &depthClipEnable))
		{
			_shadowRasterizerDesc.DepthClipEnable = depthClipEnable;
			hasChanged = true;
		}

		if (ImGui::Checkbox("Scissor", &scissorEnable))
		{
			_shadowRasterizerDesc.ScissorEnable = scissorEnable;
			hasChanged = true;
		}

		if (ImGui::Checkbox("Multisample", &multisampleEnable))
		{
			_shadowRasterizerDesc.MultisampleEnable = multisampleEnable;
			hasChanged = true;
		}

		if (ImGui::Checkbox("Antialiased Line", &antialiasedLineEnable))
		{
			_shadowRasterizerDesc.AntialiasedLineEnable = antialiasedLineEnable;
			hasChanged = true;
		}

		ImGui::Dummy({ 0.0f, 6.0f });

		static bool applyContinuously = false;
		ImGui::Checkbox("Apply Continuously", &applyContinuously);

		bool applyPreset = false;
		if (!applyContinuously)
		{
			if (ImGui::Button("Apply Preset"))
				applyPreset = true;
		}

		static bool invalidPreset = false;
		if ((applyContinuously && hasChanged) || applyPreset)
		{
			ID3D11RasterizerState *tempRasterizer;

			invalidPreset = FAILED(_device->CreateRasterizerState(&_shadowRasterizerDesc, &tempRasterizer));

			if (!invalidPreset)
			{
				_shadowRasterizer.Reset();
				_shadowRasterizer = tempRasterizer;
			}
		}

		if (invalidPreset)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::Text("Invalid Preset!");
			ImGui::PopStyleColor();
		}

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Transparency Blending"))
	{
		bool hasChanged = false;

		static bool alphaToCoverageEnable = _transparentBlendDesc.AlphaToCoverageEnable;
		static bool independentBlendEnable = _transparentBlendDesc.IndependentBlendEnable;
		static bool blendEnable = _transparentBlendDesc.RenderTarget[0].BlendEnable;
		
		static int srcBlend = 4;
		static int destBlend = 1;
		static int blendOp = 0;
		
		static int srcBlendAlpha = 4;
		static int destBlendAlpha = 5;
		static int blendOpAlpha = 4;

		static int renderTargetWriteMask = _transparentBlendDesc.RenderTarget[0].RenderTargetWriteMask;

		constexpr D3D11_BLEND blendValues[] = {
			D3D11_BLEND_ZERO,
			D3D11_BLEND_ONE,
			D3D11_BLEND_SRC_COLOR,
			D3D11_BLEND_INV_SRC_COLOR,
			D3D11_BLEND_SRC_ALPHA,
			D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_DEST_ALPHA,
			D3D11_BLEND_INV_DEST_ALPHA,
			D3D11_BLEND_DEST_COLOR,
			D3D11_BLEND_INV_DEST_COLOR,
			D3D11_BLEND_SRC_ALPHA_SAT,
			D3D11_BLEND_BLEND_FACTOR,
			D3D11_BLEND_INV_BLEND_FACTOR,
			D3D11_BLEND_SRC1_COLOR,
			D3D11_BLEND_INV_SRC1_COLOR,
			D3D11_BLEND_SRC1_ALPHA,
			D3D11_BLEND_INV_SRC1_ALPHA
		};
		constexpr char blendNames[] = "ZERO\0ONE\0SRC_COLOR\0INV_SRC_COLOR\0SRC_ALPHA\0INV_SRC_ALPHA\0DEST_ALPHA\0INV_DEST_ALPHA\0DEST_COLOR\0INV_DEST_COLOR\0SRC_ALPHA_SAT\0BLEND_FACTOR\0INV_BLEND_FACTOR\0SRC1_COLOR\0INV_SRC1_COLOR\0SRC1_ALPHA\0INV_SRC1_ALPHA\0";
		
		constexpr D3D11_BLEND_OP blendOpValues[] = {
			D3D11_BLEND_OP_ADD,
			D3D11_BLEND_OP_SUBTRACT,
			D3D11_BLEND_OP_REV_SUBTRACT,
			D3D11_BLEND_OP_MIN,
			D3D11_BLEND_OP_MAX
		};
		constexpr char blendOpNames[] = "ADD\0SUBTRACT\0REV_SUBTRACT\0MIN\0MAX\0";

		if (ImGui::Checkbox("Alpha to Coverage", &alphaToCoverageEnable))
		{
			_transparentBlendDesc.AlphaToCoverageEnable = alphaToCoverageEnable;
			hasChanged = true;
		}

		if (ImGui::Checkbox("Independent Blend", &independentBlendEnable))
		{
			_transparentBlendDesc.IndependentBlendEnable = independentBlendEnable;
			hasChanged = true;
		}

		if (ImGui::Checkbox("Blend", &blendEnable))
		{
			_transparentBlendDesc.RenderTarget[0].BlendEnable = blendEnable ? 1 : 0;
			hasChanged = true;
		}

		if (ImGui::Combo("Source Blend", &srcBlend, blendNames))
		{
			_transparentBlendDesc.RenderTarget[0].SrcBlend = blendValues[srcBlend];
			hasChanged = true;
		}
		
		if (ImGui::Combo("Destination Blend", &destBlend, blendNames))
		{
			_transparentBlendDesc.RenderTarget[0].DestBlend = blendValues[destBlend];
			hasChanged = true;
		}

		if (ImGui::Combo("Blend Operation", &blendOp, blendOpNames))
		{
			_transparentBlendDesc.RenderTarget[0].BlendOp = blendOpValues[blendOp];
			hasChanged = true;
		}

		if (ImGui::Combo("Source Blend Alpha", &srcBlendAlpha, blendNames))
		{
			_transparentBlendDesc.RenderTarget[0].SrcBlendAlpha = blendValues[srcBlendAlpha];
			hasChanged = true;
		}

		if (ImGui::Combo("Destination Blend Alpha", &destBlendAlpha, blendNames))
		{
			_transparentBlendDesc.RenderTarget[0].DestBlendAlpha = blendValues[destBlendAlpha];
			hasChanged = true;
		}

		if (ImGui::Combo("Blend Operation Alpha", &blendOpAlpha, blendOpNames))
		{
			_transparentBlendDesc.RenderTarget[0].BlendOpAlpha = blendOpValues[blendOpAlpha];
			hasChanged = true;
		}

		bool renderTargetWriteMaskR = renderTargetWriteMask & D3D11_COLOR_WRITE_ENABLE_RED;
		if (ImGui::Checkbox("Write Red", &renderTargetWriteMaskR))
		{
			renderTargetWriteMask = renderTargetWriteMaskR ? 
				renderTargetWriteMask | D3D11_COLOR_WRITE_ENABLE_RED : 
				renderTargetWriteMask & ~D3D11_COLOR_WRITE_ENABLE_RED;

			_transparentBlendDesc.RenderTarget[0].RenderTargetWriteMask = renderTargetWriteMask;
			hasChanged = true;
		}

		bool renderTargetWriteMaskG = renderTargetWriteMask & D3D11_COLOR_WRITE_ENABLE_GREEN;
		if (ImGui::Checkbox("Write Green", &renderTargetWriteMaskG))
		{
			renderTargetWriteMask = renderTargetWriteMaskG ? 
				renderTargetWriteMask | D3D11_COLOR_WRITE_ENABLE_GREEN : 
				renderTargetWriteMask & ~D3D11_COLOR_WRITE_ENABLE_GREEN;

			_transparentBlendDesc.RenderTarget[0].RenderTargetWriteMask = renderTargetWriteMask;
			hasChanged = true;
		}

		bool renderTargetWriteMaskB = renderTargetWriteMask & D3D11_COLOR_WRITE_ENABLE_BLUE;
		if (ImGui::Checkbox("Write Blue", &renderTargetWriteMaskB))
		{
			renderTargetWriteMask = renderTargetWriteMaskB ?
				renderTargetWriteMask | D3D11_COLOR_WRITE_ENABLE_BLUE :
				renderTargetWriteMask & ~D3D11_COLOR_WRITE_ENABLE_BLUE;

			_transparentBlendDesc.RenderTarget[0].RenderTargetWriteMask = renderTargetWriteMask;
			hasChanged = true;
		}

		bool renderTargetWriteMaskA = renderTargetWriteMask & D3D11_COLOR_WRITE_ENABLE_ALPHA;
		if (ImGui::Checkbox("Write Alpha", &renderTargetWriteMaskA))
		{
			renderTargetWriteMask = renderTargetWriteMaskA ?
				renderTargetWriteMask | D3D11_COLOR_WRITE_ENABLE_ALPHA :
				renderTargetWriteMask & ~D3D11_COLOR_WRITE_ENABLE_ALPHA;

			_transparentBlendDesc.RenderTarget[0].RenderTargetWriteMask = renderTargetWriteMask;
			hasChanged = true;
		}

		ImGui::Dummy({ 0.0f, 6.0f });

		static bool applyContinuously = false;
		ImGui::Checkbox("Apply Continuously", &applyContinuously);

		bool applyPreset = false;
		if (!applyContinuously)
		{
			if (ImGui::Button("Apply Preset"))
				applyPreset = true;
		}

		static bool invalidPreset = false;
		if ((applyContinuously && hasChanged) || applyPreset)
		{
			ID3D11BlendState *tempBlendState;

			invalidPreset = FAILED(_device->CreateBlendState(&_transparentBlendDesc, &tempBlendState));

			if (!invalidPreset)
			{
				_tbs.Reset();
				_tbs = tempBlendState;
			}
		}

		if (invalidPreset)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::Text("Invalid Preset!");
			ImGui::PopStyleColor();
		}

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("Light Draw Info"))
	{
		ImGuiChildFlags childFlags = 0;
		childFlags |= ImGuiChildFlags_Border;
		childFlags |= ImGuiChildFlags_ResizeY;

		ImGui::BeginChild("Entity Hierarchy", ImVec2(0, 300), childFlags);

		ImGui::Text(std::format("Main Draws: {}", _currViewCamera->GetCullCount()).c_str());
		for (UINT i = 0; i < _currSpotLightCollection->GetNrOfLights(); i++)
		{
			const CameraBehaviour *spotlightCamera = _currSpotLightCollection->GetLightBehaviour(i)->GetShadowCamera();
			ImGui::Text(std::format("Spotlight #{} Draws: {}", i, spotlightCamera->GetCullCount()).c_str());
		}

		for (UINT i = 0; i < _currPointLightCollection->GetNrOfLights(); i++)
		{
			const CameraCubeBehaviour *pointlightCamera = _currPointLightCollection->GetLightBehaviour(i)->GetShadowCameraCube();
			ImGui::Text(std::format("Pointlight #{} Draws: {}", i, pointlightCamera->GetCullCount()).c_str());
		}

		ImGui::EndChild();
		ImGui::TreePop();
	}

	return true;
}
bool Graphics::EndUIRender() const
{
	ImGui::End();

	//static ID3D11RenderTargetView *const nullRTV = nullptr;
	_context->OMSetRenderTargets(1, _imGuiRtv.GetAddressOf(), nullptr);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	return true;
}
#endif

bool Graphics::EndFrame()
{
	if (FAILED(_swapChain->Present(_vSync ? 1 : 0, 0)))
	{
		ErrMsg("Failed to present geometry!");
		return false;
	}

	if (!ResetRenderState())
	{
		ErrMsg("Failed to reset render state!");
		return false;
	}

	return true;
}
bool Graphics::ResetRenderState()
{
	_currViewCamera->ResetRenderQueue();

	for (UINT i = 0; i < _currSpotLightCollection->GetNrOfLights(); i++)
		_currSpotLightCollection->GetLightBehaviour(i)->GetShadowCamera()->ResetRenderQueue();

	for (UINT i = 0; i < _currPointLightCollection->GetNrOfLights(); i++)
		_currPointLightCollection->GetLightBehaviour(i)->GetShadowCameraCube()->ResetRenderQueue();

	_currInputLayoutID	= CONTENT_NULL;
	_currMeshID			= CONTENT_NULL;
	_currVsID			= CONTENT_NULL;
	_currPsID			= CONTENT_NULL;
	_currTexID			= CONTENT_NULL;
	_currNormalID		= CONTENT_NULL;
	_currSpecularID		= CONTENT_NULL;
	_currGlossinessID	= CONTENT_NULL;
	_currAmbientID		= CONTENT_NULL;
	_currHeightID		= CONTENT_NULL;
	_currLightID		= CONTENT_NULL;
	_currOcclusionID	= CONTENT_NULL;
	_currSamplerID		= CONTENT_NULL;
	_currVsID			= CONTENT_NULL;
	_currPsID			= CONTENT_NULL;

	_isRendering = false;
	return true;
}
