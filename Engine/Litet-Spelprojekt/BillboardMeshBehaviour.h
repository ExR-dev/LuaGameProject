#pragma once
#include "Behaviour.h"
#include "MeshBehaviour.h"

class BillboardMeshBehaviour : public Behaviour
{
private:
	MeshBehaviour *_meshBehaviour = nullptr;
	Material _material;

	bool _transparent = true;
	bool _castShadows = false;
	bool _overlay = false;

	bool _keepUpright = true;
	float _rotation = 0.0f;
	float _normalOffset = 0.0f;
	float _scale = 1.0f;

protected:
	[[nodiscard]] bool Start() override;

	[[nodiscard]] bool ParallelUpdate(const Time &time, const Input &input) override;

#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI() override;
#endif

	void OnEnable() override;
	void OnDisable() override;

public:
	BillboardMeshBehaviour() = default;
	~BillboardMeshBehaviour() = default;

	BillboardMeshBehaviour(
		const Material &material, float rotation, float normalOffset, float size, 
		bool keepUpright, bool isTransparent, bool castShadows, bool isOverlay);

	void SetSize(float size);
	void SetRotation(float rotation);
	MeshBehaviour* GetMeshBehaviour() const;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;
};