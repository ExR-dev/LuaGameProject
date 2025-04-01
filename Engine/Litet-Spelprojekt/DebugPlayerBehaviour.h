#pragma once

#include "Behaviour.h"
#include "Transform.h"
#include "CameraBehaviour.h"
#include "RenderQueuer.h"
#include "Raycast.h"

class DebugPlayerBehaviour final : public Behaviour
{
#ifdef DEBUG_BUILD
private:
	CameraBehaviour *_mainCamera = nullptr;
	CameraBehaviour *_secondaryCamera = nullptr;
	CameraBehaviour *_currCameraPtr = nullptr;

	ReferenceSpace _editSpace = Local;
	TransformationType _editType = Translate;

	int _currCamera = -2;
	int _currSelection = -1;
	bool _moveRelative = false;
	bool _rayCastFromMouse = false;

	bool _useMainCamera = true;
	bool _rotateLights = false;
	bool _drawPointer = false;

	Entity *_cursorPositioningTarget = nullptr;
	bool _includePositioningTargetInTree = false;

	std::vector<std::pair<KeyCode, UINT>> _duplicateBinds = {};
	int _addDuplicateBindForEntity = -1;

	[[nodiscard]] bool UpdateGlobalEntities(Time &time, const Input &input);

	// out contains entity and distance to entity from camera, pos is the coordinates for the ray hit
	[[nodiscard]] bool RayCastFromCamera(RaycastOut &out);	// Casts a ray from _camera in the direction of _camera
	[[nodiscard]] bool RayCastFromCamera(RaycastOut &out, DirectX::XMFLOAT3A &pos, DirectX::XMFLOAT3A &dir);
	[[nodiscard]] bool RayCastFromMouse(RaycastOut &out, const Input &input);	// Casts a ray from mouse position on nearplane along the z-axis
	[[nodiscard]] bool RayCastFromMouse(RaycastOut &out, DirectX::XMFLOAT3A &pos, DirectX::XMFLOAT3A &dir, const Input &input);

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

	// Render runs every frame when objects are being queued for rendering.
	[[nodiscard]] bool Render(const RenderQueuer &queuer, const RendererInfo &rendererInfo) override;

public:
	DebugPlayerBehaviour() = default;
	~DebugPlayerBehaviour() = default;

	void SetCamera(CameraBehaviour *cam);
	void SetSelection(int index);
	void SetSelectionID(int id);
	[[nodiscard]] int GetSelection() const;

	void SetEditSpace(ReferenceSpace space);
	[[nodiscard]] ReferenceSpace GetEditSpace() const;

	void SetEditType(TransformationType type);
	[[nodiscard]] TransformationType GetEditType() const;

	void AssignDuplicateToKey(UINT id);
	bool IsAssigningDuplicateToKey(UINT id) const;
	bool IsValidDuplicateBind(KeyCode key) const;
	void AddDuplicateBind(KeyCode key, UINT id);
	void RemoveDuplicateBind(UINT id);
	bool HasDuplicateBind(UINT id) const;
	KeyCode GetDuplicateBind(UINT id);
	void ClearDuplicateBinds();

	void PositionWithCursor(Entity *ent);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;
#endif
};
