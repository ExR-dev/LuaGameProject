#pragma once

#include "Behaviour.h"
#include "Content.h"
#include <fstream>

class MeshBehaviour final : public Behaviour
{
private:
	UINT _meshID = CONTENT_NULL;
	const Material *_material = nullptr;

	bool 
		_isTransparent = false,
		_isOverlay = false,
		_castShadows = true,
		_updatePosBuffer = true,
		_updateMatBuffer = false,
		_recalculateBounds = true;

	float _alphaCutoff = 0.5f;

	ConstantBufferD3D11
		_materialBuffer,
		_posBuffer;

	DirectX::BoundingOrientedBox _bounds;
	DirectX::BoundingOrientedBox _transformedBounds;

	[[nodiscard]] bool ValidateMaterial(const Material **material);

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

	// Render runs every frame when objects are being queued for rendering.
	[[nodiscard]] bool Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo) override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

	// BindBuffers runs before drawcalls pertaining to the Entity are performed.
	[[nodiscard]] bool BindBuffers() override;

	// OnDirty runs when the Entity's transform is modified.
	void OnDirty() override;

	
public:
	MeshBehaviour() = default;
	MeshBehaviour(const DirectX::BoundingOrientedBox &bounds, bool isTransparent = false, bool castShadows = true, bool isOverlay = false);
	MeshBehaviour(const DirectX::BoundingOrientedBox &bounds, UINT meshID, const Material *material, bool isTransparent = false, bool castShadows = true, bool isOverlay = false);
	~MeshBehaviour() = default;

	void StoreBounds(DirectX::BoundingOrientedBox &meshBounds);

	void SetMesh(UINT meshID, bool updateBounds = false);
	[[nodiscard]] bool SetMaterial(const Material *material);
	void SetTransparent(bool state);
	void SetOverlay(bool state);
	void SetCastShadows(bool state);
	void SetAlphaCutoff(float value);
	void SetBounds(DirectX::BoundingOrientedBox &newBounds);

	[[nodiscard]] UINT GetMesh() const;
	[[nodiscard]] const Material *GetMaterial() const;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;

};
