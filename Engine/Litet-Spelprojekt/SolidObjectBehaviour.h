#pragma once
#include "Behaviour.h"
#include "DirectXMath.h"
#include "Intersections.h"

class SolidObjectBehaviour : public Behaviour
{
private:

protected:

	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start();

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input);

	// Like Update, but later.
	[[nodiscard]] bool LateUpdate(Time &time, const Input &input);

	// FixedUpdate runs every physics update (20hz by default).
	[[nodiscard]] bool FixedUpdate(const float &deltaTime, const Input &input);

	// Render runs every frame when objects are being queued for rendering.
	[[nodiscard]] bool Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo);

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI();
#endif

	// BindBuffers runs before drawcalls pertaining to the Entity are performed.
	[[nodiscard]] bool BindBuffers();

	// OnEnable runs immediately after the behaviour is enabled.
	void OnEnable();

	// OnEnable runs immediately after the behaviour is disabled.
	void OnDisable();

	// OnDirty runs when the Entity's transform is modified.
	void OnDirty();

	[[nodiscard]] bool OnSelect();
	[[nodiscard]] bool OnHover();


public:
	SolidObjectBehaviour() = default;
	~SolidObjectBehaviour() = default;

	// Adjust position of entity based on collision properties
	void AdjustForCollision(const Collisions::CollisionData &data);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code);
};