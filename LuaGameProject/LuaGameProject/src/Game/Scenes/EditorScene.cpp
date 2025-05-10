#pragma region Includes & Definitions
#include "stdafx.h"
#include "EditorScene.h"

#include "../Utilities/InputHandler.h"
#include "../Utilities/LuaInput.h"
#include "../Utilities/ModLoader.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif
#pragma endregion

#pragma region Public
EditorScene::EditorScene::EditorScene()
{
	ZoneScopedC(RandomUniqueColor());

	Game::IsQuitting = false;
}
EditorScene::EditorScene::~EditorScene()
{
	ZoneScopedC(RandomUniqueColor());

	Game::IsQuitting = true;

	if (L)
	{
		lua_close(L);
		L = nullptr;
	}
}

int EditorScene::EditorScene::Start(WindowInfo *windowInfo)
{
	ZoneScopedC(RandomUniqueColor());

	m_windowInfo = windowInfo;
	m_renderTexture = raylib::RenderTexture(m_windowInfo->p_screenWidth, m_windowInfo->p_screenHeight);

	// Setup Box2D
	m_physicsHandler.Setup();

	// Setup Lua enviroment

	// Create internal lua state
	L = luaL_newstate();
	luaL_openlibs(L);

	m_windowInfo->BindLuaWindow(L);

	Scene::lua_openscene(L, &m_scene);

	m_luaGame = LuaGame::LuaGame(L, &m_scene);
	LuaGame::LuaGame::lua_opengame(L, &m_luaGame);

	m_camera.target = raylib::Vector2(64, 0);
	m_camera.offset = raylib::Vector2(m_windowInfo->p_screenWidth / 2.0f, m_windowInfo->p_screenHeight / 2.0f);
	m_camera.rotation = 0.0f;
	m_camera.zoom = 1.5f;

	BindLuaInput(L);

	// Add lua require path
	std::string luaScriptPath = std::format("{}/{}?{}", fs::current_path().generic_string(), FILE_PATH, FILE_EXT);
	LuaDoString(std::format("package.path = \"{};\" .. package.path", luaScriptPath).c_str());

	// Initialize Lua data & mods
	ModLoader::LuaLoadData(L, DATA_PATH);
	ModLoader::LuaLoadMods(L, MOD_PATH);

	m_scene.SystemsInitialize(L);

	LuaDoFileCleaned(L, LuaFilePath("InitEditorScene")); // Creates entities

	return 1;
}

Game::SceneState EditorScene::EditorScene::Loop()
{
	ZoneScopedC(RandomUniqueColor());

	auto state = Update();

	Render();

	return state;
}

void EditorScene::EditorScene::OnSwitchToScene()
{
	ZoneScopedC(RandomUniqueColor());

	OnResizeWindow();
}

void EditorScene::EditorScene::OnResizeWindow()
{
	ZoneScopedC(RandomUniqueColor());

	m_windowInfo->BindLuaWindow(L);
	m_renderTexture = raylib::RenderTexture(m_windowInfo->p_screenWidth, m_windowInfo->p_screenHeight);
}
#pragma endregion

#pragma region Protected
Game::SceneState EditorScene::EditorScene::Update()
{
	ZoneScopedC(RandomUniqueColor());

	if (Input::CheckKeyPressed(Input::GAME_KEY_R))
	{
		m_camera.zoom = 1.0f;
	}

	// Update systems
	m_scene.SystemsOnUpdate(Time::DeltaTime());

	// Update Physics
	m_physicsHandler.Update(L, &m_scene);

	std::function<void(entt::registry& registry)> createPhysicsBodies = [&](entt::registry& registry) {
		ZoneNamedNC(createPhysicsBodiesZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

		auto view = registry.view<ECS::Collider, ECS::Transform>();
		view.use<ECS::Collider>();

		view.each([&](const entt::entity entity, ECS::Collider& collider, ECS::Transform& transform) {
			ZoneNamedNC(drawSpriteZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

			// Create body
			if (!collider.createBody)
			{
				b2Body_SetTransform(collider.bodyId, { transform.Position[0], transform.Position[1] }, { cosf(transform.Rotation * DEG2RAD), sinf(transform.Rotation * DEG2RAD) });
			}
			else
			{
				collider.bodyId = m_physicsHandler.CreateRigidBody(static_cast<int>(entity), collider, transform);
				collider.createBody = false;
			}
		});
	};

	m_scene.RunSystem(createPhysicsBodies);

	m_scene.CleanUp(L);

	return Game::SceneState::None;
}

int EditorScene::EditorScene::Render()
{
	ZoneScopedC(RandomUniqueColor());

	BeginDrawing();
	ClearBackground(raylib::Color(24, 18, 13));

	// Draw Scene
	m_renderTexture.BeginMode();
	{
		ClearBackground(raylib::Color(100, 149, 237));

		// Render systems
		m_scene.SystemsOnRender(Time::DeltaTime());

		// Scene
		{
			ZoneNamedNC(renderSceneZone, "Render Scene", RandomUniqueColor(), true);
			BeginMode2D(m_camera);

			// Draw sprites
			std::function<void(entt::registry &registry)> drawSystem = [](entt::registry &registry) {
				ZoneNamedNC(drawSpritesZone, "Lambda Draw Sprites", RandomUniqueColor(), true);

				auto view = registry.view<ECS::Sprite, ECS::Transform>();
				view.use<ECS::Sprite>();

				view.each([&](const entt::entity entity, const ECS::Sprite &sprite, const ECS::Transform &transform) {
					ZoneNamedNC(drawSpriteZone, "Lambda Draw Sprite", RandomUniqueColor(), true);

					// If the entity has an active component, check if it is active
					if (registry.all_of<ECS::Active>(entity))
					{
						ECS::Active &active = registry.get<ECS::Active>(entity);
						if (!active.IsActive)
							return; // Skip drawing if the entity is not active
					}

					int flip = transform.Scale[1] > 0 ? 1 : -1;

					raylib::Color color(*(raylib::Vector4 *)(&(sprite.Color)));
					raylib::Rectangle rect(
						transform.Position[0],
						transform.Position[1],
						transform.Scale[0],
						transform.Scale[1] * flip
					);

					raylib::Vector2 origin(
						(transform.Scale[0] / 2),
						(transform.Scale[1] / 2) * flip
					);

					std::string textureName = sprite.SpriteName;
					const raylib::Texture2D *texture = nullptr;

					if (textureName != "")
					{
						texture = ResourceManager::Instance().GetTextureResource(textureName);

						if (!texture)
						{
							ResourceManager::Instance().LoadTextureResource(textureName);
							texture = ResourceManager::Instance().GetTextureResource(textureName);
						}
					}

					if (texture)
					{
						DrawTexturePro(
							*texture,
							raylib::Rectangle(0, 0, (float)texture->width, (float)(texture->height * flip)),
							rect,
							origin,
							transform.Rotation,
							color
						);
					}
					else
					{
						DrawRectanglePro(
							rect,
							origin,
							transform.Rotation,
							color
						);
					}
				});
			};
			m_scene.RunSystem(drawSystem);

			std::function<void(entt::registry &registry)> createPhysicsBodies = [&](entt::registry &registry) {
				ZoneNamedNC(createPhysicsBodiesZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

				auto view = registry.view<ECS::Collider, ECS::Transform>();
				view.use<ECS::Collider>();

				view.each([&](ECS::Collider &collider, ECS::Transform &transform) {
					ZoneNamedNC(drawSpriteZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

					if (collider.debug)
					{
						const float w = fabsf(transform.Scale[0]),
							h = fabsf(transform.Scale[1]);
						b2Vec2 p = b2Body_GetWorldPoint(collider.bodyId, { 0, 0 });
						//b2Transform t;

						b2Rot rotation = b2Body_GetRotation(collider.bodyId);
						float radians = b2Rot_GetAngle(rotation);

						Rectangle rect = { p.x, p.y , w, h };
						DrawRectanglePro(rect, { w / 2, h / 2 }, radians * RAD2DEG, { 0, 228, 46, 100 });
					}
				});
			};

			m_scene.RunSystem(createPhysicsBodies);

			// Draw a circle at the mouse position, converted to scene coordinates
			raylib::Vector2 mousePos = GetMousePosition();
			if (IsWithinSceneView(mousePos))
			{
				raylib::Vector2 scenePos = ScreenToWorldPos(mousePos);
				DrawCircleV(scenePos, 4, raylib::Color(255, 0, 0, 255));
			}

			EndMode2D();
		}
	}
	m_renderTexture.EndMode();

	RenderUI();

	EndDrawing();
	return 1;
}
#pragma endregion

#pragma region Private
int EditorScene::EditorScene::RenderUI()
{
	ZoneScopedC(RandomUniqueColor());

	rlImGuiBeginDelta(Time::DeltaTime());

	// Manual character input passthrough, as the raylib queue has already been flushed
	ImGuiIO &io = ImGui::GetIO();
	if (io.WantCaptureKeyboard)
	{
		const std::string charBuffer = Input::GetUnicodeInput();

		for (int i = 0; i < charBuffer.length(); i++)
		{
			int key = charBuffer[i];
			io.AddInputCharacter(key);
		}
	}

#ifdef IMGUI_HAS_DOCK
	ImGui::DockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode);
#endif

	ImGuiWindowFlags editorFlags = ImGuiWindowFlags_None;
	editorFlags |= ImGuiWindowFlags_NoTitleBar;
	editorFlags |= ImGuiWindowFlags_NoResize;
	editorFlags |= ImGuiWindowFlags_NoMove;
	editorFlags |= ImGuiWindowFlags_NoScrollbar;
	editorFlags |= ImGuiWindowFlags_NoCollapse;
	editorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	editorFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	editorFlags |= ImGuiWindowFlags_NoDocking;
	editorFlags |= ImGuiWindowFlags_NoBackground;

	ImGui::Begin("Editor##EditorWindow", nullptr, editorFlags);
	ImGui::SetWindowPos(ImVec2(io.DisplaySize.x / 2 - ImGui::GetWindowWidth() / 2, 0));
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));

	// Insert ImGui code here
	{
		ZoneNamedNC(renderCustomUIZone, "Render Custom ImGui UI", RandomUniqueColor(), true);

		// Render the render texture window
		{
			ImGuiWindowFlags viewFlags = ImGuiWindowFlags_None;
			viewFlags |= ImGuiWindowFlags_NoScrollbar;

			int stylesPushed = 0;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); stylesPushed++;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0); stylesPushed++;

			if (ImGui::Begin("View##RenderTextureWindow", &m_sceneViewOpen, viewFlags))
			{
				ImVec2 imGuiWindowPos = ImGui::GetWindowPos();
				ImVec2 imGuiWindowSize = ImGui::GetWindowSize();

				ImVec2 imGuiWindowContentRegionMin = ImGui::GetWindowContentRegionMin();
				ImVec2 imGuiWindowContentRegionMax = ImGui::GetWindowContentRegionMax();

				float renderAspect = (float)m_renderTexture.texture.width / (float)m_renderTexture.texture.height;

				ImVec2 viewCenter = ImVec2(
					imGuiWindowSize.x / 2 - m_renderTexture.texture.width / 2,
					imGuiWindowSize.y / 2 - m_renderTexture.texture.height / 2
				);

				ImVec2 sceneViewSize = (imGuiWindowContentRegionMax - imGuiWindowContentRegionMin);
				ImVec2 sceneViewCenter = (imGuiWindowContentRegionMin + imGuiWindowContentRegionMax) * 0.5f;

				// Draw the render texture
				float viewAspect = sceneViewSize.x / sceneViewSize.y;

				ImVec2 scaledBounds = ImVec2(0, 0);
				if (renderAspect > viewAspect)
				{
					scaledBounds.x = sceneViewSize.x;
					scaledBounds.y = sceneViewSize.x / renderAspect;
				}
				else
				{
					scaledBounds.x = sceneViewSize.y * renderAspect;
					scaledBounds.y = sceneViewSize.y;
				}

				ImVec2 renderTextureCorner = sceneViewCenter - (scaledBounds * 0.5f);
				ImGui::SetCursorPos(renderTextureCorner);

				ImGui::Image(
					(ImTextureID)m_renderTexture.texture.id,
					scaledBounds,
					ImVec2(0, 1), // Top left
					ImVec2(1, 0)  // Bottom right
				);

				// Get the scene view bounds
				m_sceneViewRect = raylib::Rectangle(
					GameMath::ImToRayVec(renderTextureCorner + imGuiWindowPos),
					GameMath::ImToRayVec(scaledBounds)
				);

				// Draw FPS Counter
				int fps = GetFPS();
				ImGui::SetCursorPos(imGuiWindowContentRegionMin + ImVec2(8, 8));
				ImGui::Text("FPS: %d", fps);
			}

			ImGui::End();
			ImGui::PopStyleVar(stylesPushed);
		}

		static int selectedEntity = -1;

		if (ImGui::Begin("Scene Hierarchy"))
		{

			if (ImGui::Button("Create Entity"))
			{
				int id = m_scene.CreateEntity();
				ECS::Transform transform{ {0, 0}, 0, {100, 100} };
				m_scene.SetComponent(id, transform);
			}

			std::function<void(entt::registry &registry)> renderEntityUI = [&](entt::registry &registry) {
				ZoneNamedNC(createPhysicsBodiesZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

				auto view = registry.view<ECS::Transform>();

				view.each([&](const entt::entity &entity, ECS::Transform &transform) {
					ZoneNamedNC(drawSpriteZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

					const int id = static_cast<int>(entity);
					if (ImGui::Selectable(std::format("Entity {}", id).c_str(), (id == selectedEntity)))
						selectedEntity = id;
				});
			};

			m_scene.RunSystem(renderEntityUI);

		}
		ImGui::End();

		if (ImGui::Begin("Entity Editor"))
		{
			if (m_scene.IsEntity(selectedEntity))
			{
				if (m_scene.HasComponents<ECS::Active>(selectedEntity))
					m_scene.GetComponent<ECS::Active>(selectedEntity).RenderUI();

				if (m_scene.HasComponents<ECS::Transform>(selectedEntity))
					m_scene.GetComponent<ECS::Transform>(selectedEntity).RenderUI();

				if (m_scene.HasComponents<ECS::Collider>(selectedEntity))
					m_scene.GetComponent<ECS::Collider>(selectedEntity).RenderUI();

				if (m_scene.HasComponents<ECS::Sprite>(selectedEntity))
					m_scene.GetComponent<ECS::Sprite>(selectedEntity).RenderUI();

				if (m_scene.HasComponents<ECS::Behaviour>(selectedEntity))
					m_scene.GetComponent<ECS::Behaviour>(selectedEntity).RenderUI();

				std::string items[] = { "Collider", "Sprite", "Behaviour" };

				if (ImGui::BeginCombo("##AddComponentCombo", "Add Component"))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						std::string current = items[n];
						if (ImGui::Selectable(current.c_str()))
						{
							if (current == "Collider")
								m_scene.SetComponent<ECS::Collider>(selectedEntity, ECS::Collider());
							else if (current == "Behaviour")
								m_scene.SetComponent<ECS::Behaviour>(selectedEntity, ECS::Behaviour("Behaviours/Enemy", selectedEntity, L));
							else if (current == "Sprite")
							{
								const float color[4]{ 0, 0, 0, 1 };
								m_scene.SetComponent<ECS::Sprite>(selectedEntity, ECS::Sprite("\0", color, 0));
							}
						}
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				ImGui::Text("Select a entity ...");
			}
		}
		ImGui::End();
	}

	ImGui::End();
	rlImGuiEnd();
	return 1;
}

raylib::Vector2 EditorScene::EditorScene::ScreenToWorldPos(const raylib::Vector2 &pos) const
{
	raylib::Vector2 transformedPos = pos;

	transformedPos.x -= m_sceneViewRect.x;
	transformedPos.y -= m_sceneViewRect.y;

	transformedPos.x /= m_sceneViewRect.width;
	transformedPos.y /= m_sceneViewRect.height;

	transformedPos.x *= (float)m_renderTexture.texture.width;
	transformedPos.y *= (float)m_renderTexture.texture.height;

	return m_camera.GetScreenToWorld(transformedPos);
}

bool EditorScene::EditorScene::IsWithinSceneView(const raylib::Vector2 &pos) const
{
	if (!m_sceneViewOpen)
		return false;
	return m_sceneViewRect.CheckCollision(pos);
}
#pragma endregion