#pragma once

#include "EngineSettings.h"

#ifdef LEAK_DETECTION
#include "DebugNew.h"
#endif

#define _USE_MATH_DEFINES

#include <vector>
#include <algorithm>
#include <queue>
#include <memory>
#include <iostream>
#include <fstream>
#include <ctime>
#include <filesystem>
#include <cstdlib>
#include <functional>
#include <Windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>
#include <Audio.h>

#include "Window.h"
#ifdef USE_SDL3
#include <SDL3/SDL.h>
#include "WindowSDL3.h"
#endif

#ifdef USE_IMGUI
#include "ImGui/imconfig.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_sdl3.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_stdlib.h"
#endif

#ifdef PIX_TIMELINING
#include "pix3.h"
#endif
#include "stb_image.h"

#include "GameMath.h"
#include "D3D11Helper.h"
#include "Time.h"
#include "Input.h"
#include "InputBindings.h"
#include "Material.h"
#include "Transform.h"
#include "ErrMsg.h"
#include "RendererInfo.h"
#include "RenderQueuer.h"

#include "ConstantBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "IndexBufferD3D11.h"
#include "InputLayoutD3D11.h"
#include "MeshD3D11.h"
#include "RenderTargetD3D11.h"
#include "SamplerD3D11.h"
#include "ShaderD3D11.h"
#include "ShaderResourceTextureD3D11.h"
#include "SimpleMeshD3D11.h"
#include "StructuredBufferD3D11.h"
#include "SubMeshD3D11.h"
#include "VertexBufferD3D11.h"

#ifdef USE_RAYLIB

namespace RL
{
    //#include "raylib.h"
}
#endif

constexpr UINT MENU_SCENE = 0;
constexpr UINT GAME_SCENE = 1;
constexpr UINT CRED_SCENE = 2;
constexpr UINT START_CUTSCENE = 3;
