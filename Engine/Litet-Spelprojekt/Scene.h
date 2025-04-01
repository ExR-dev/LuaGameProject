#pragma once

#include <d3d11.h>

#include "SceneHolder.h"
#include "Entity.h"
#include "Graphics.h"
#include "SpotLightCollection.h"
#include "PointLightCollection.h"
#include "Material.h"
#include "SoundEngine.h"
#include "CameraBehaviour.h"
#include "CollisionHandler.h"
#include "DebugDrawer.h"
#include "GraphManager.h"
#include "TimelineManager.h"
#ifdef USE_IMGUI
#include "tinyfiledialogs.h"
#endif

class Game;
class GraphNodeBehaviour;
class MonsterBehaviour;
#ifdef DEBUG_BUILD
class DebugPlayerBehaviour;
class TransformGizmoBehaviour;
#endif

// Contains and manages entities, cameras and lights. Also handles queueing entities for rendering.
class Scene
{
private:
	struct CameraCullingView
	{
		union
		{
			DirectX::BoundingFrustum frustum = {};
			DirectX::BoundingOrientedBox box;
		} view;

		bool isOrtho = false;
		UINT prevCullCount = 0;

		CamRenderQueuer queuer = nullptr;
		RendererInfo rendererInfo;
	};
	struct CubeCullingView
	{
		DirectX::BoundingBox viewBox = {};
		std::vector<DirectX::BoundingFrustum> viewFrustums;

		UINT prevCullCount = 0;

		std::vector<CubeRenderQueuer> queuers;
		RendererInfo rendererInfo;

		std::function<void(UINT)> setCullCountFunc = nullptr;
	};

	std::vector<std::unique_ptr<Entity>> _globalEntities = {};
	std::unique_ptr<SpotLightCollection> _spotlights;
	std::unique_ptr<PointLightCollection> _pointlights;

	bool _initialized = false;
	Game *_game = nullptr;
	ID3D11Device *_device = nullptr;
	ID3D11DeviceContext *_context = nullptr;
	Content *_content = nullptr;
	Graphics *_graphics = nullptr;
	GraphManager _graphManager = {};
	SceneHolder _sceneHolder;
#ifdef DEBUG_BUILD
	DebugPlayerBehaviour *_debugPlayer = nullptr;
	TransformGizmoBehaviour *_transformGizmo = nullptr;
#endif
	const Input *_input = nullptr;

	CameraBehaviour *_viewCamera = nullptr;
	CameraBehaviour *_playerCamera = nullptr;
	CameraBehaviour *_animationCamera = nullptr;

	Entity *_player = nullptr;
	MonsterBehaviour *_monster = nullptr;
	ColliderBehaviour *_terrainBehaviour = nullptr;
	const Collisions::Terrain *_terrain = nullptr;

	CollisionHandler _collisionHandler;
	SoundEngine _soundEngine;
    
	TimelineManager _timelineManager;

#ifdef DEBUG_BUILD
	bool _isGeneratingEntityBounds = false;
	bool _isGeneratingVolumeTree = false;
	bool _isGeneratingCameraCulling = false;
	bool _rayCastFromMouse = false;
	int _cameraCubeSide = 0;
#endif

#ifdef USE_IMGUI
	bool _undockSceneHierarchy = false;
	bool _undockEntityHierarchy = false;
	std::vector<UINT> _undockedEntities = {};
#endif

	std::string _saveFile = "";

	[[nodiscard]] bool UpdateSound();

#ifdef USE_IMGUI
	[[nodiscard]] bool RenderEntityCreatorUI();
	[[nodiscard]] bool RenderEntityUI(UINT id);
	[[nodiscard]] bool RenderEntityHierarchyUI(Entity *root, UINT depth, std::string search = "");
#endif

public:
	Scene();
	~Scene();
	Scene(const Scene &other) = default;
	Scene &operator=(const Scene &other) = default;
	Scene(Scene &&other) = default;
	Scene &operator=(Scene &&other) = default;

	[[nodiscard]] bool Initialize(ID3D11Device *device, ID3D11DeviceContext *context, Game *game, Content *content, Graphics *graphics,
		float gameVolume, const std::string &saveFile = "");
	[[nodiscard]] bool InitializeMenu(ID3D11Device *device, ID3D11DeviceContext *context, Game *game, Content *content, Graphics *graphics,
		float gameVolume, const std::string &saveFile = "");
	[[nodiscard]] bool InitializeStartCutscene(ID3D11Device *device, ID3D11DeviceContext *context, Game *game, Content *content, Graphics *graphics,
		float gameVolume, const std::string &saveFile = "");
	[[nodiscard]] bool InitializeGame(ID3D11Device *device, ID3D11DeviceContext *context, Game *game, Content *content, Graphics *graphics,
		float gameVolume, const std::string &saveFile = "");
	[[nodiscard]] bool InitializeCred(ID3D11Device *device, ID3D11DeviceContext *context, Game *game, Content *content, Graphics *graphics,
		float gameVolume, const std::string &saveFile = "");

	[[nodiscard]] bool IsInitialized() const;
	void SetInitialized(bool state);

	[[nodiscard]] bool Update(Time &time, const Input &input);
	[[nodiscard]] bool LateUpdate(Time &time, const Input &input);
	[[nodiscard]] bool FixedUpdate(const float &deltaTime, const Input &input);

	[[nodiscard]] bool Render(Time &time, const Input &input);
#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI();
#endif

	[[nodiscard]] bool Serialize(std::string *code) const;
	[[nodiscard]] bool SerializeEntity(std::string *code, Entity *entity, bool forceSerialize = false) const;
	[[nodiscard]] bool Deserialize();
	[[nodiscard]] bool DeserializeEntity(const std::string &line, std::optional<Entity**> out = std::nullopt);
	void PostDeserialize();

	[[nodiscard]] ID3D11Device *GetDevice() const;
	[[nodiscard]] ID3D11DeviceContext *GetContext() const;
	[[nodiscard]] Content *GetContent() const;
	[[nodiscard]] SceneHolder *GetSceneHolder();
	[[nodiscard]] Graphics *GetGraphics() const;
	[[nodiscard]] GraphManager *GetGraphManager();
	[[nodiscard]] const Input *GetInput() const;
	[[nodiscard]] DebugDrawer *GetDebugDrawer() const;
	[[nodiscard]] CollisionHandler *GetCollisionHandler();
	[[nodiscard]] std::vector<std::unique_ptr<Entity>> *GetGlobalEntities();
	[[nodiscard]] SpotLightCollection *GetSpotlights() const;
	[[nodiscard]] PointLightCollection *GetPointlights() const;
#ifdef DEBUG_BUILD
	[[nodiscard]] DebugPlayerBehaviour *GetDebugPlayer() const;
#endif
	[[nodiscard]] TimelineManager* GetTimelineManager();
	[[nodiscard]] SoundEngine *GetSoundEngine();
	[[nodiscard]] Game *GetGame() const;
	[[nodiscard]] std::string GetSaveFile() const;

	[[nodiscard]] Entity *GetPlayer() const;
	[[nodiscard]] MonsterBehaviour *GetMonster() const;
	ColliderBehaviour *GetTerrainBehaviour() const;
	[[nodiscard]] const Collisions::Terrain *GetTerrain() const;

#ifdef DEBUG_BUILD
	void SetDebugPlayer(DebugPlayerBehaviour *debugPlayer);
#endif

	void SetViewCamera(CameraBehaviour *camera);
	[[nodiscard]] CameraBehaviour *GetViewCamera();
	[[nodiscard]] CameraBehaviour *GetPlayerCamera();
	[[nodiscard]] CameraBehaviour *GetAnimationCamera();

	[[nodiscard]] bool CreateEntity(Entity **out, std::string name, const DirectX::BoundingOrientedBox &bounds, bool hasVolume);
	[[nodiscard]] bool CreateGlobalEntity(Entity **out, std::string name, const DirectX::BoundingOrientedBox &bounds, bool hasVolume);

	[[nodiscard]] bool CreateMeshEntity(Entity **out, const std::string &name, UINT meshID, const Material &material, bool isTransparent = false, bool shadowCaster = true, bool isOverlay = false);
	[[nodiscard]] bool CreateBillboardMeshEntity(Entity **out, const std::string &name, const Material &material, float rotation = 0.0f, float normalOffset = 0.0f, float size = 1.0f, bool keepUpright = true, bool isTransparent = true, bool shadowCaster = false, bool isOverlay = false);
	[[nodiscard]] bool CreateGlobalMeshEntity(Entity **out, const std::string &name, UINT meshID, const Material &material, bool isTransparent = false, bool shadowCaster = true, bool isOverlay = false);

	[[nodiscard]] bool CreateCameraEntity(Entity **out, const std::string &name, float fov, float aspect, float nearZ, float farZ);
	[[nodiscard]] bool CreateAnimationCamera();

	[[nodiscard]] bool CreatePlayerEntity(Entity **out);
	[[nodiscard]] bool CreateMonsterEntity(Entity **out);

	[[nodiscard]] bool CreateLanternEntity(Entity **out);

	[[nodiscard]] bool CreateSpotLightEntity(Entity **out, const std::string &name, DirectX::XMFLOAT3 color, float falloff, float angle, bool ortho = false, float nearZ = 0.1f, UINT updateFrequency = 2);
	[[nodiscard]] bool CreatePointLightEntity(Entity **out, const std::string &name, DirectX::XMFLOAT3 color, float falloff, float nearZ = 0.1f, UINT updateFrequency = 3);
	[[nodiscard]] bool CreateSimpleSpotLightEntity(Entity **out, const std::string &name, DirectX::XMFLOAT3 color, float falloff, float angle, bool ortho = false);
	[[nodiscard]] bool CreateSimplePointLightEntity(Entity **out, const std::string &name, DirectX::XMFLOAT3 color, float falloff);

	[[nodiscard]] bool CreateGraphNodeEntity(Entity **out, GraphNodeBehaviour **node, DirectX::XMFLOAT3 pos);
	[[nodiscard]] bool CreateSoundEmitterEntity(Entity** out, const std::string& name, const std::string& fileName, bool loop = false, float volume = 1.0f, float distanceScaler = 75.0f, float reverbScaler = 1.0f, float minimumDelay = 2.0f, float maximumDelay = 10.0f);

	void SetSceneVolume(float volume);
	void SuspendSceneSound();
	void ResumeSceneSound();

	void ResetScene();

	void 👢();
};
