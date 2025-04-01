#pragma once

#include <d3d11.h>
#include "EngineSettings.h"
#include "Time.h"
#include "Input.h"
#include "Transform.h"
#include "RendererInfo.h"

class Scene;
class Entity;
class CameraBehaviour;
class RenderQueuer;

class Behaviour
{
private:
	bool _isInitialized = false;
	bool _isEnabledSelf = true;
	bool _doSerialize = true;

	Entity *_entity = nullptr;

protected:
	std::string _name = "";

	// Start runs once when the behaviour is created.
	[[nodiscard]] virtual bool Start();

	// Update runs every frame.
	[[nodiscard]] virtual bool Update(Time &time, const Input &input);
	
	// ParallelUpdate runs after update and exeutes in parallel with all other behaviours, so one must ensure thread safety between behaviours.
	[[nodiscard]] virtual bool ParallelUpdate(const Time &time, const Input &input);
	
	// Like Update, but later.
	[[nodiscard]] virtual bool LateUpdate(Time &time, const Input &input);

	// FixedUpdate runs every physics update (20hz by default).
	[[nodiscard]] virtual bool FixedUpdate(const float &deltaTime, const Input &input);

	// Render runs for all objects queued for rendering before they are rendered.
	[[nodiscard]] virtual bool BeforeRender();

	// Render runs when objects are being queued for rendering.
	[[nodiscard]] virtual bool Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo);

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] virtual bool RenderUI();
#endif

	// BindBuffers runs before drawcalls pertaining to the Entity are performed.
	[[nodiscard]] virtual bool BindBuffers();

	// OnEnable runs immediately after the behaviour is enabled.
	virtual void OnEnable();

	// OnEnable runs immediately after the behaviour is disabled.
	virtual void OnDisable();

	// OnDirty runs when the Entity's transform is modified.
	virtual void OnDirty();

	// Serializes the behaviour to a string.
	[[nodiscard]] virtual bool Serialize(std::string* code) const;

	// Deserializes the behaviour from a string.
	[[nodiscard]] virtual bool Deserialize(const std::string& code);

	[[nodiscard]] virtual bool OnSelect();
	[[nodiscard]] virtual bool OnHover();
	[[nodiscard]] virtual bool OffHover();

public:
	Behaviour() = default;
	virtual ~Behaviour() = default;

	[[nodiscard]] bool Initialize(Entity *entity, const std::string &behaviourName = "");
	[[nodiscard]] bool IsInitialized() const;

	void SetSerialization(bool state);
	[[nodiscard]] bool IsSerializable() const;

	[[nodiscard]] Entity *GetEntity() const;
	[[nodiscard]] Transform *GetTransform() const;
	[[nodiscard]] Scene *GetScene() const;

	void SetName(const std::string &name);
	[[nodiscard]] const std::string &GetName() const;

	[[nodiscard]] bool InitialUpdate(Time &time, const Input &input);
	[[nodiscard]] bool InitialParallelUpdate(const Time &time, const Input &input);
	[[nodiscard]] bool InitialLateUpdate(Time &time, const Input &input);
	[[nodiscard]] bool InitialFixedUpdate(const float &deltaTime, const Input &input);
	[[nodiscard]] bool InitialBeforeRender();
	[[nodiscard]] bool InitialRender(const RenderQueuer &queuer, const RendererInfo &rendererInfo);
#ifdef USE_IMGUI
	[[nodiscard]] bool InitialRenderUI();
#endif
	[[nodiscard]] bool InitialBindBuffers();

	[[nodiscard]] bool InitialOnSelect();
	[[nodiscard]] bool InitialOnHover();
	[[nodiscard]] bool InitialOffHover();

	[[nodiscard]] bool IsEnabled() const;
	[[nodiscard]] bool IsEnabledSelf() const;
	void InheritEnabled(bool state);
	void SetEnabled(bool state);
	void SetDirty();

	[[nodiscard]] bool InitialSerialize(std::string* code) const;
	[[nodiscard]] bool InitialDeserialize(const std::string& code);

};