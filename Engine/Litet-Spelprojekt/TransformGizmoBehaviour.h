#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "Behaviour.h"
#include "MeshBehaviour.h"
#include "Transform.h"

#ifdef DEBUG_BUILD
class TransformGizmoBehaviour final : public Behaviour
{
private:
	int _selectedAxisID = -1;
	DirectX::XMFLOAT3A _activeAxis = { 0, 0, 0 };

	DirectX::XMFLOAT3A _prevEntPos = { 0, 0, 0 };
	DirectX::XMFLOAT3A _prevSelectPos = { 0, 0, 0 };
	DirectX::XMFLOAT4A _initialEntRot = { 0, 0, 0, 0 };
	DirectX::XMFLOAT3A _initialEntScale = { 0, 0, 0 };
	float _scaleFactor = 0.0f;

	MeshBehaviour *_gizmoMesh = nullptr;
	UINT _translationMeshID = -1, _rotationMeshID = -1, _scalingMeshID = -1;
	float _gizmoScale = 1.0f;
	float _gizmoLen = 2.225f;

	float _posGridSize = 0.5f;
	float _rotGridSize = 15.0f;

	ReferenceSpace _editSpace = Local;
	TransformationType _editType = Translate;

	bool _moveRelative = false;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

public:
	TransformGizmoBehaviour() = default;
	~TransformGizmoBehaviour() = default;

	void SetGridSize(float size);
	void SetGizmoScale(float scale);
};
#endif
