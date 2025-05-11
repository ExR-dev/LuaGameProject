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

	for (int i = 0; i < EditorMode::COUNT; i++)
		m_editorModeScenes[i] = nullptr;
}

int EditorScene::EditorScene::Start(WindowInfo *windowInfo, CmdState *cmdState)
{
	ZoneScopedC(RandomUniqueColor());

	m_windowInfo = windowInfo;
	m_cmdState = cmdState;
	m_renderTexture = raylib::RenderTexture(m_windowInfo->p_screenWidth, m_windowInfo->p_screenHeight);

	m_camera.target = raylib::Vector2(0, 0);
	m_camera.offset = raylib::Vector2(m_windowInfo->p_screenWidth / 2.0f, m_windowInfo->p_screenHeight / 2.0f);
	m_camera.rotation = 0.0f;
	m_camera.zoom = 1.0f;

	for (int i = 0; i < EditorMode::COUNT; i++)
	{
		m_editorModeScenes[i] = std::make_unique<EditorModeScene>();
		m_editorModeScenes[i].get()->Init(windowInfo, m_editorModeNames[i]);
	}

	return 1;
}

Game::SceneState EditorScene::EditorScene::Loop()
{
	ZoneScopedC(RandomUniqueColor());

	auto &modeScene = *(m_editorModeScenes[m_editorMode].get());
	auto &scene = modeScene.scene;
	auto &L = modeScene.L;

#if defined(LUA_DEBUG) && !defined(LEAK_DETECTION)
	if (Game::Game::Instance().CmdStepMode)
	{
		if (Game::Game::Instance().CmdTakeSteps > 0)
		{
			Game::Game::Instance().CmdTakeSteps--;
		}
		else
		{
			ExecuteCommandList(L, m_cmdState, scene.GetRegistry());
			Windows::SleepW(16);
			return Game::SceneState::None;
		}
	}
#endif

	Game::SceneState state = Game::SceneState::None;
	if (m_sceneUpdateMode != SceneUpdateMode::Paused)
		state = Update();

	Render();

#ifndef LEAK_DETECTION
	ExecuteCommandList(L, m_cmdState, scene.GetRegistry());
#endif

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

	for (int i = 0; i < EditorMode::COUNT; i++)
		m_windowInfo->BindLuaWindow(m_editorModeScenes[i].get()->L);

	m_renderTexture = raylib::RenderTexture(m_windowInfo->p_screenWidth, m_windowInfo->p_screenHeight);
	m_camera.offset = raylib::Vector2(m_windowInfo->p_screenWidth / 2.0f, m_windowInfo->p_screenHeight / 2.0f);
}
#pragma endregion

#pragma region Protected
Game::SceneState EditorScene::EditorScene::Update()
{
	ZoneScopedC(RandomUniqueColor());

	auto &modeScene = *(m_editorModeScenes[m_editorMode].get());
	auto &physicsHandler = modeScene.physicsHandler;
	auto &scene = modeScene.scene;
	auto &L = modeScene.L;

	auto mRState = Input::GetMouseState(Input::GameMouse::GAME_MOUSE_RIGHT);
	auto mInfo = Input::GetMouseInfo();

	if (m_isDraggingCamera)
	{
		m_dragOffset = ScreenToWorldPos(mInfo.position);
		raylib::Vector2 newTarget = Vector2Subtract(m_camera.target, Vector2Subtract(m_dragOffset, m_dragOrigin));
		m_camera.SetTarget(newTarget);

		if (mRState & Input::RELEASED)
			m_isDraggingCamera = false;
	}

	if (IsWithinSceneView(mInfo.position))
	{
		if (Input::CheckKeyPressed(Input::GAME_KEY_R))
		{
			m_camera.zoom = 1.0f;
			m_camera.target = raylib::Vector2(0, 0);
		}

		if (mInfo.scroll != 0.0f)
		{
			if (mInfo.scroll > 0.0f)
				m_camera.zoom *= (1.0f + 0.1f * mInfo.scroll);
			else
				m_camera.zoom /= (1.0f - 0.1f * mInfo.scroll);
		}

		if (mRState & Input::PRESSED)
		{
			m_isDraggingCamera = true;
			m_dragOrigin = ScreenToWorldPos(mInfo.position);
		}
	}

	// Update systems
	float dTime = 0.0f;
	if (m_sceneUpdateMode >= SceneUpdateMode::Running)
		dTime = Time::DeltaTime();

	scene.SystemsOnUpdate(dTime);

	// Update Physics
	physicsHandler.Update(L, &scene);

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
				collider.bodyId = physicsHandler.CreateRigidBody(static_cast<int>(entity), collider, transform);
				collider.createBody = false;
			}
		});
	};

	scene.RunSystem(createPhysicsBodies);

	scene.CleanUp(L);

	return Game::SceneState::None;
}

int EditorScene::EditorScene::Render()
{
	ZoneScopedC(RandomUniqueColor());

	auto &modeScene = *(m_editorModeScenes[m_editorMode].get());
	auto &physicsHandler = modeScene.physicsHandler;
	auto &scene = modeScene.scene;
	auto &L = modeScene.L;

	BeginDrawing();
	ClearBackground(raylib::Color(24, 18, 13));

	// Draw Scene
	m_renderTexture.BeginMode();
	{
		ClearBackground(raylib::Color(100, 149, 237));

		// Render systems
		scene.SystemsOnRender(Time::DeltaTime());

		// Scene
		{
			ZoneNamedNC(renderSceneZone, "Render Scene", RandomUniqueColor(), true);
			BeginMode2D(m_camera);

			// Render grid
			{
				ZoneNamedNC(renderGridZone, "Render Grid", RandomUniqueColor(), true);

				constexpr int lines = 25;
				constexpr float stepSize = 100.0f;
				constexpr float innerThickness = 1.0f;
				constexpr float thickness = 4.0f;

				for (int i = -lines; i < lines; i++)
				{
					DrawLineEx(
						raylib::Vector2(i * stepSize, -lines * stepSize),
						raylib::Vector2(i * stepSize,  lines * stepSize),
						innerThickness,
						raylib::Color(25, 25, 25, 128)
					);
					DrawLineEx(
						raylib::Vector2(i * stepSize, -lines * stepSize),
						raylib::Vector2(i * stepSize,  lines * stepSize),
						thickness,
						raylib::Color(25, 25, 25, 64)
					);

					DrawLineEx(
						raylib::Vector2(-lines * stepSize, i * stepSize),
						raylib::Vector2( lines * stepSize, i * stepSize),
						innerThickness,
						raylib::Color(25, 25, 25, 128)
					);
					DrawLineEx(
						raylib::Vector2(-lines * stepSize, i * stepSize),
						raylib::Vector2( lines * stepSize, i * stepSize),
						thickness,
						raylib::Color(25, 25, 25, 64)
					);
				}
			}


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
						texture = ResourceManager::GetTextureResource(textureName);

						if (!texture)
						{
							ResourceManager::LoadTextureResource(textureName);
							texture = ResourceManager::GetTextureResource(textureName);
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
			scene.RunSystem(drawSystem);

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
			scene.RunSystem(createPhysicsBodies);

			// Draw a circle at the mouse position, converted to scene coordinates
			raylib::Vector2 mousePos = GetMousePosition();
			if (IsWithinSceneView(mousePos))
			{
				raylib::Vector2 scenePos = ScreenToWorldPos(mousePos);
				DrawCircleV(scenePos, 5.0f / m_camera.zoom, raylib::Color(255, 0, 0, 255));
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

	auto &modeScene = *(m_editorModeScenes[m_editorMode].get());

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

		if (GameMath::EqualsAny(m_editorMode, EditorMode::Sandbox, EditorMode::LevelCreator))
		{
			if (ImGui::Begin("Scene Hierarchy"))
			{
				if (ImGui::Button("Create Entity"))
				{
					int id = modeScene.scene.CreateEntity();
					ECS::Transform transform{ {0, 0}, 0, {100, 100} };
					modeScene.scene.SetComponent(id, transform);
				}

				std::function<void(entt::registry &registry)> renderEntityUI = [&](entt::registry &registry) {
					ZoneNamedNC(createPhysicsBodiesZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

					auto view = registry.view<ECS::Transform>();

					view.each([&](const entt::entity &entity, ECS::Transform &transform) {
						ZoneNamedNC(drawSpriteZone, "Lambda Create Physics Bodies", RandomUniqueColor(), true);

						const int id = static_cast<int>(entity);
						if (ImGui::Selectable(std::format("Entity {}", id).c_str(), (id == m_selectedEntity)))
							m_selectedEntity = id;
					});
				};

				modeScene.scene.RunSystem(renderEntityUI);
			}
			ImGui::End();

			if (ImGui::Begin("Entity Editor"))
			{
				if (modeScene.scene.IsEntity(m_selectedEntity))
				{
					if (modeScene.scene.HasComponents<ECS::Active>(m_selectedEntity))
						modeScene.scene.GetComponent<ECS::Active>(m_selectedEntity).RenderUI();

					if (modeScene.scene.HasComponents<ECS::Transform>(m_selectedEntity))
						modeScene.scene.GetComponent<ECS::Transform>(m_selectedEntity).RenderUI();

					if (modeScene.scene.HasComponents<ECS::Collider>(m_selectedEntity))
						modeScene.scene.GetComponent<ECS::Collider>(m_selectedEntity).RenderUI();

					if (modeScene.scene.HasComponents<ECS::Sprite>(m_selectedEntity))
						modeScene.scene.GetComponent<ECS::Sprite>(m_selectedEntity).RenderUI();

					if (modeScene.scene.HasComponents<ECS::Behaviour>(m_selectedEntity))
						modeScene.scene.GetComponent<ECS::Behaviour>(m_selectedEntity).RenderUI();

					std::string items[] = { "Collider", "Sprite", "Behaviour" };

					if (ImGui::BeginCombo("##AddComponentCombo", "Add Component"))
					{
						for (int n = 0; n < IM_ARRAYSIZE(items); n++)
						{
							std::string current = items[n];
							if (ImGui::Selectable(current.c_str()))
							{
								if (current == "Collider")
									modeScene.scene.SetComponent<ECS::Collider>(m_selectedEntity, ECS::Collider());
								else if (current == "Behaviour")
									modeScene.scene.SetComponent<ECS::Behaviour>(m_selectedEntity, ECS::Behaviour("Behaviours/Enemy", m_selectedEntity, modeScene.L));
								else if (current == "Sprite")
								{
									const float color[4]{ 0, 0, 0, 1 };
									modeScene.scene.SetComponent<ECS::Sprite>(m_selectedEntity, ECS::Sprite("\0", color, 0));
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

		if (m_editorMode == EditorMode::PresetCreator)
		{
			// HACK: for now, call weapon editor immediately
			LuaDoFileCleaned(modeScene.L, LuaFilePath("Dev/WeaponEditorUI"));
		}

		// Render the render texture window
		{
			ZoneNamedNC(renderSceneWindowZone, "Render Scene Window", RandomUniqueColor(), true);

			ImGuiWindowFlags viewFlags = ImGuiWindowFlags_None;
			viewFlags |= ImGuiWindowFlags_NoScrollbar;

			int stylesPushed = 0;
			stylesPushed++; ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			stylesPushed++; ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

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

				// Draw Play/Freeze/Pause buttons
				{
					int styleVars = 0, styleCols = 0;
					styleVars++; ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(-1, -1));
					styleCols++; ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.15f));
					styleCols++; ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.1f));
					styleCols++; ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.3f));

					ImVec2 buttonsMid = imGuiWindowContentRegionMin + ImVec2(sceneViewSize.x * 0.5f, 16.0f);
					ImVec2 buttonSize = ImVec2(32, 32);
					int buttonSpacing = 48;

					// Play
					ImGui::SetCursorPos(buttonsMid + ImVec2(-buttonSpacing, 0));
					if (ImGui::ImageButton("##PlayButton",
						(ImTextureID)ResourceManager::GetTextureResource("PlayIcon.png")->id,
						buttonSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
						(m_sceneUpdateMode == SceneUpdateMode::Running) ? ImVec4(.5f, .5f, .5f, 1.f) : ImVec4(1, 1, 1, 1)
					))
					{
						m_sceneUpdateMode = SceneUpdateMode::Running;
					}

					// Freeze
					ImGui::SetCursorPos(buttonsMid);
					if (ImGui::ImageButton("##FreezeButton",
						(ImTextureID)ResourceManager::GetTextureResource("FreezeIcon.png")->id,
						buttonSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
						(m_sceneUpdateMode == SceneUpdateMode::Frozen) ? ImVec4(.5f, .5f, .5f, 1.f) : ImVec4(1, 1, 1, 1)
					))
					{
						m_sceneUpdateMode = SceneUpdateMode::Frozen;
					}

					// Pause
					ImGui::SetCursorPos(buttonsMid + ImVec2(buttonSpacing, 0));
					if (ImGui::ImageButton("##PauseButton",
						(ImTextureID)ResourceManager::GetTextureResource("PauseIcon.png")->id,
						buttonSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
						(m_sceneUpdateMode == SceneUpdateMode::Paused) ? ImVec4(.5f, .5f, .5f, 1.f) : ImVec4(1, 1, 1, 1)
					))
					{
						m_sceneUpdateMode = SceneUpdateMode::Paused;
					}
					ImGui::PopStyleColor(styleCols);
					ImGui::PopStyleVar(styleVars);
				}

				// Draw Reset Scene/Reload Data buttons
				{
					ImVec2 buttonsTopRight = imGuiWindowContentRegionMin + ImVec2(sceneViewSize.x - 16.0f, 16.0f);
					ImVec2 buttonSize = ImVec2(64, 24);
					int buttonSpacing = buttonSize.y + 16;

					// Reset Scene
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.2f, 0.15f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.3f, 0.25f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.65f, 0.1f, 0.05f, 1.0f));

					ImGui::SetCursorPos(buttonsTopRight + ImVec2(-buttonSize.x, 0));
					if (ImGui::Button("Reset##ResetSceneButton", buttonSize))
					{
						m_selectedEntity = -1;
						m_editorModeScenes[m_editorMode] = std::make_unique<EditorModeScene>();
						m_editorModeScenes[m_editorMode].get()->Init(m_windowInfo, m_editorModeNames[m_editorMode]);
					}
					ImGui::PopStyleColor(3);

					// Reload Data
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.75f, 0.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.65f, 0.3f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.55f, 0.55f, 0.075f, 1.0f));

					ImGui::SetCursorPos(buttonsTopRight + ImVec2(-buttonSize.x, buttonSpacing));
					if (ImGui::Button("Reload##ReloadDataButton", buttonSize))
					{
						m_editorModeScenes[m_editorMode].get()->LoadData();
					}
					ImGui::PopStyleColor(3);
				}
			}

			ImGui::End();
			ImGui::PopStyleVar(stylesPushed);
		}

		if (ImGui::Begin("Editor Settings##EditorSettingsWindow"))
		{
			for (int i = 0; i < EditorMode::COUNT; i++)
			{
				bool isSelected = (m_editorMode == i);
				std::string modeName = m_editorModeNames[i];

				if (ImGui::Selectable(std::format("{}##EditorMode{}", modeName, i).c_str(), &isSelected))
				{
					if (isSelected)
					{
						SwitchEditorMode(static_cast<EditorMode>(i));
					}
				}
			}
		}
		ImGui::End();
	}

	ImGui::End();
	rlImGuiEnd();
	return 1;
}

void EditorScene::EditorScene::SwitchEditorMode(EditorMode mode)
{
	if (m_editorMode == mode)
		return;

	m_editorMode = mode;
	m_selectedEntity = -1;

	//m_editorModeScenes[m_editorMode].get()->LoadData();
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
raylib::Vector2 EditorScene::EditorScene::WorldToScreenPos(const raylib::Vector2 &pos) const
{
	raylib::Vector2 transformedPos = pos;

	transformedPos = m_camera.GetWorldToScreen(transformedPos);

	transformedPos.x /= (float)m_renderTexture.texture.width;
	transformedPos.y /= (float)m_renderTexture.texture.height;

	transformedPos.x *= m_sceneViewRect.width;
	transformedPos.y *= m_sceneViewRect.height;

	transformedPos.x += m_sceneViewRect.x;
	transformedPos.y += m_sceneViewRect.y;

	return transformedPos;
}
bool EditorScene::EditorScene::IsWithinSceneView(const raylib::Vector2 &pos) const
{
	if (!m_sceneViewOpen)
		return false;
	return m_sceneViewRect.CheckCollision(pos);
}

void EditorScene::EditorScene::EditorModeScene::Init(WindowInfo *windowInfo, const std::string &name)
{
	// Setup Lua enviroment
	L = luaL_newstate();
	luaL_openlibs(L);
	windowInfo->BindLuaWindow(L);

	physicsHandler.Setup();

	Scene::lua_openscene(L, &scene);

	luaGame = LuaGame::LuaGame(L, &scene);
	LuaGame::LuaGame::lua_opengame(L, &luaGame);

	BindLuaInput(L);

	// Add lua require path
	std::string luaScriptPath = std::format("{}/{}?{}", fs::current_path().generic_string(), FILE_PATH, LUA_EXT);
	LuaDoString(L, std::format("package.path = \"{};\" .. package.path", luaScriptPath).c_str());

	// Initialize Lua data & mods
	LoadData();

	scene.SystemsInitialize(L);

	LuaDoFileCleaned(L, LuaFilePath(std::format("Scenes/InitEditor{}Scene", name))); // Creates entities
}
void EditorScene::EditorScene::EditorModeScene::LoadData() const
{
	ModLoader::LuaLoadData(L, DATA_PATH);
	ModLoader::LuaLoadMods(L, MOD_PATH);
}
#pragma endregion
