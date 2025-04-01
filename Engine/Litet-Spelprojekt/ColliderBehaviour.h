#pragma once

#include "Behaviour.h"
#include "Colliders.h"

class ColliderBehaviour : public Behaviour
{
private:
	Collisions::Collider* _baseCollider = nullptr;
	Collisions::Collider* _transformedCollider = nullptr;

	bool _debugDraw = false;
	bool _isDirty = true;
	bool _isIntersecting = false;
	bool _toCheck = false;

protected:
	[[nodiscard]] bool Start() override;
	[[nodiscard]] bool Update(Time &time, const Input &input) override;
	[[nodiscard]] bool Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo) override;
#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI() override;
#endif
	void OnDirty() override;

public:
	ColliderBehaviour() = default;
	~ColliderBehaviour();

	// Note: collider vaues should be defined in local-space
	void SetCollider(Collisions::Collider *collider);
	const Collisions::Collider *GetCollider() const;

	void AddOnIntersection(std::function<void(const Collisions::CollisionData &)> callback);
	void AddOnCollisionEnter(std::function<void(const Collisions::CollisionData &)> callback);
	void AddOnCollisionExit(std::function<void(const Collisions::CollisionData &)> callback);

#ifdef DEBUG_BUILD
	[[nodiscard]] bool SetDebugCollider(Scene* scene, const Content *content);
#endif

	void SetIntersecting(bool value);
	bool GetIntersecting();

	void CheckDone();
	bool GetToCheck();

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};

