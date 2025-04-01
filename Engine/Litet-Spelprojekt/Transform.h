#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <functional>

#include "ConstantBufferD3D11.h"

enum ReferenceSpace
{
	Local,
	World
};

enum TransformationType
{
	Translate,
	Rotate,
	Scale
};

class Transform
{
private:
	Transform *_parent = nullptr;
	std::vector<Transform*> _children;

	DirectX::XMFLOAT3A _localPosition = { 0, 0, 0    };
	DirectX::XMFLOAT4A _localRotation = { 0, 0, 0, 1 };
	DirectX::XMFLOAT3A _localScale =	{ 1, 1, 1    };
	DirectX::XMFLOAT4X4A _localMatrix = { 1, 0, 0, 0,
										  0, 1, 0, 0,
										  0, 0, 1, 0,
										  0, 0, 0, 1 };

	DirectX::XMFLOAT3A _worldPosition = { 0, 0, 0    };
	DirectX::XMFLOAT4A _worldRotation = { 0, 0, 0, 1 };
	DirectX::XMFLOAT3A _worldScale =	{ 1, 1, 1    };
	DirectX::XMFLOAT4X4A _worldMatrix = { 1, 0, 0, 0, 
										  0, 1, 0, 0, 
										  0, 0, 1, 0, 
										  0, 0, 0, 1 };

	ConstantBufferD3D11 _worldMatrixBuffer;

	std::vector<std::function<void()>> _dirtyCallbacks;

	bool _isDirty = true;
	bool _isScenePosDirty = true;
	bool _isWorldPositionDirty = true;	// Dirtied by parent position, rotation and scale.
	bool _isWorldRotationDirty = true;	// Dirtied by parent rotation.
	bool _isWorldScaleDirty = true;		// Dirtied by parent rotation and scale.
	bool _isWorldMatrixDirty = true;	// Dirtied by parent position, rotation and scale.
	bool _isLocalMatrixDirty = true;	// Never dirtied by parent.

	inline void AddChild(Transform *child);
	inline void RemoveChild(Transform *child);

	void SetWorldPositionDirty();
	void SetWorldRotationDirty();
	void SetWorldScaleDirty();
	void SetAllDirty();

	[[nodiscard]] DirectX::XMFLOAT3A *WorldPosition();
	[[nodiscard]] DirectX::XMFLOAT4A *WorldRotation();
	[[nodiscard]] DirectX::XMFLOAT3A *WorldScale();
	[[nodiscard]] DirectX::XMFLOAT4X4A *WorldMatrix();
	[[nodiscard]] DirectX::XMFLOAT4X4A *LocalMatrix();

	[[nodiscard]] const DirectX::XMFLOAT3A InverseTransformPoint(DirectX::XMFLOAT3A point) const;
	[[nodiscard]] const DirectX::XMFLOAT4X4A GetWorldRotationAndScale();

public:
	Transform() = default;
	explicit Transform(ID3D11Device *device, const DirectX::XMFLOAT4X4A &worldMatrix);
	~Transform();
	Transform(const Transform &other) = delete;
	Transform &operator=(const Transform &other) = delete;
	Transform(Transform &&other) = delete;
	Transform &operator=(Transform &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device);
	[[nodiscard]] bool Initialize(ID3D11Device *device, const DirectX::XMFLOAT4X4A &worldMatrix);

	void SetDirty();
	[[nodiscard]] bool IsDirty() const;
	bool IsScenePosDirty() const;
	void AddDirtyCallback(std::function<void(void)> callback);
	void CleanScenePos();

	[[nodiscard]] Transform *GetParent() const;
	void SetParent(Transform *parent, bool worldPositionStays = false);

	[[nodiscard]] DirectX::XMFLOAT3A GetRight(ReferenceSpace space = Local);
	[[nodiscard]] DirectX::XMFLOAT3A GetUp(ReferenceSpace space = Local);
	[[nodiscard]] DirectX::XMFLOAT3A GetForward(ReferenceSpace space = Local);
	void GetAxes(DirectX::XMFLOAT3A *right, DirectX::XMFLOAT3A *up, DirectX::XMFLOAT3A *forward, ReferenceSpace space = Local);

	[[nodiscard]] const DirectX::XMFLOAT3A &GetPosition(ReferenceSpace space = Local);
	[[nodiscard]] const DirectX::XMFLOAT4A &GetRotation(ReferenceSpace space = Local);
	[[nodiscard]] const DirectX::XMFLOAT3A &GetScale(ReferenceSpace space = Local);

	void SetPosition(const DirectX::XMFLOAT3A &position, ReferenceSpace space = Local);
	void SetPosition(const DirectX::XMFLOAT4A &position, ReferenceSpace space = Local);
	void SetRotation(const DirectX::XMFLOAT4A &rotation, ReferenceSpace space = Local);
	void SetScale(const DirectX::XMFLOAT3A &scale, ReferenceSpace space = Local);
	void SetScale(const DirectX::XMFLOAT4A &scale, ReferenceSpace space = Local);
	void SetMatrix(const DirectX::XMFLOAT4X4A &mat, ReferenceSpace space = Local);

	void Move(const DirectX::XMFLOAT3A &direction, ReferenceSpace space = Local);
	void Move(const DirectX::XMFLOAT4A &direction, ReferenceSpace space = Local);
	void Rotate(const DirectX::XMFLOAT3A &euler, ReferenceSpace space = Local);
	void Rotate(const DirectX::XMFLOAT4A &euler, ReferenceSpace space = Local);
	void Scale(const DirectX::XMFLOAT3A &scale, ReferenceSpace space = Local);
	void Scale(const DirectX::XMFLOAT4A &scale, ReferenceSpace space = Local);

	void MoveRelative(const DirectX::XMFLOAT3A &direction, ReferenceSpace space = Local);
	void MoveRelative(const DirectX::XMFLOAT4A &direction, ReferenceSpace space = Local);
	void RotateAxis(const DirectX::XMFLOAT3A &axis, const float &amount, ReferenceSpace space = Local);
	void RotateAxis(const DirectX::XMFLOAT4A &axis, const float &amount, ReferenceSpace space = Local);
	void RotateQuaternion(const DirectX::XMFLOAT4A &quaternion, ReferenceSpace space = Local);

	[[nodiscard]] const DirectX::XMFLOAT3A GetEuler(ReferenceSpace space = Local);
	void SetEuler(const DirectX::XMFLOAT3A &pitchYawRoll, ReferenceSpace space = Local);
	void SetEuler(const DirectX::XMFLOAT4A &pitchYawRoll, ReferenceSpace space = Local);

	void RotatePitch(float amount, ReferenceSpace space = Local);
	void RotateYaw(float amount, ReferenceSpace space = Local);
	void RotateRoll(float amount, ReferenceSpace space = Local);

	[[nodiscard]] bool UpdateConstantBuffer(ID3D11DeviceContext *context);
	[[nodiscard]] ID3D11Buffer *GetConstantBuffer() const;
	[[nodiscard]] const DirectX::XMFLOAT4X4A &GetLocalMatrix();
	[[nodiscard]] const DirectX::XMFLOAT4X4A &GetWorldMatrix();
	[[nodiscard]] const DirectX::XMFLOAT4X4A GetUnscaledWorldMatrix();
	[[nodiscard]] const DirectX::XMFLOAT4X4A &GetMatrix(ReferenceSpace space);
};