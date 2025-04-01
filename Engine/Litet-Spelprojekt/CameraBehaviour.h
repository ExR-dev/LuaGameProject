#pragma once

#include <vector>
#include <map>
#include <d3d11_4.h>
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include "Behaviour.h"
#include "Content.h"
#include "RendererInfo.h"


struct CameraPlanes
{
	float nearZ = 0.1f;
	float farZ = 50.0f;
};

struct ProjectionInfo
{
	float fovAngleY = 80.0f * (DirectX::XM_PI / 180.0f);
	float aspectRatio = 1.0f;
	CameraPlanes planes;
};

struct CameraBufferData
{
	DirectX::XMFLOAT4X4 viewProjMatrix;
	DirectX::XMFLOAT3A position;
	DirectX::XMFLOAT3A direction;
	float nearZ;
	float farZ;
	float padding[2];
};

struct GeometryBufferData
{
	DirectX::XMFLOAT4X4A viewMatrix;
	DirectX::XMFLOAT4A position;
};

struct ResourceGroup
{
	UINT meshID = CONTENT_NULL;
	const Material *material = nullptr;
	bool shadowCaster = true;
	bool overlay = false;

	bool operator<(const ResourceGroup &other) const
	{
		if ((*material) != (*other.material))
			return (*material) < (*other.material);

		return meshID < other.meshID;
	}
}; 

struct RenderInstance
{
	Behaviour *subject;
	size_t subjectSize;
};

class CameraBehaviour final : public Behaviour
{
private:
	RendererInfo _rendererInfo;
	ProjectionInfo _defaultProjInfo, _currProjInfo;
	bool 
		_ortho = false,
		_invertedDepth = false;

	union
	{
		DirectX::BoundingFrustum perspective = {};
		DirectX::BoundingOrientedBox ortho;
	} _bounds;
	union
	{
		DirectX::BoundingFrustum perspective = {};
		DirectX::BoundingOrientedBox ortho;
	} _transformedBounds;
	bool _recalculateBounds = true;
	bool _transformedWithScale = false;
	bool _debugDraw = false;
	bool _overlayDraw = false;

	std::vector<DirectX::BoundingFrustum> _lightGridFrustums;
	std::vector<DirectX::BoundingFrustum> _transformedLightGridFrustums;

	ConstantBufferD3D11 _viewProjBuffer;
	std::unique_ptr<ConstantBufferD3D11> _viewProjPosBuffer = nullptr;
	std::unique_ptr<ConstantBufferD3D11> _invCamBuffer = nullptr;
	std::unique_ptr<ConstantBufferD3D11> _posBuffer = nullptr;
	bool _isDirty = true;

	// Let batching be handled by multimap
	std::multimap<ResourceGroup, RenderInstance> _geometryRenderQueue; 
	std::multimap<ResourceGroup, RenderInstance> _transparentRenderQueue;
	std::multimap<ResourceGroup, RenderInstance> _overlayRenderQueue;

	UINT _lastCullCount = 0;

protected:
	// Start runs once when the behaviour is created.
	[[nodiscard]] bool Start() override;

	// Update runs every frame.
	[[nodiscard]] bool Update(Time &time, const Input &input) override;

#ifdef USE_IMGUI
	// RenderUI runs every frame during ImGui rendering if the entity is selected.
	[[nodiscard]] bool RenderUI() override;
#endif

	// OnDirty runs when the Entity's transform is modified.
	void OnDirty() override;

public:
	CameraBehaviour() = default;
	CameraBehaviour(const ProjectionInfo &projectionInfo, bool isOrtho = false, bool invertDepth = true);
	~CameraBehaviour() = default;

	[[nodiscard]] DirectX::XMFLOAT4X4A GetViewMatrix();
	[[nodiscard]] DirectX::XMFLOAT4X4A GetProjectionMatrix() const;
	[[nodiscard]] DirectX::XMFLOAT4X4A GetViewProjectionMatrix();
	[[nodiscard]] const ProjectionInfo &GetCurrProjectionInfo() const;

	[[nodiscard]] bool ScaleToContents(const std::vector<DirectX::XMFLOAT4A> &nearBounds, const std::vector<DirectX::XMFLOAT4A> &innerBounds);
	[[nodiscard]] bool FitPlanesToPoints(const std::vector<DirectX::XMFLOAT4A> &points);
	/// This must be called from the behaviour in control of the camera.
	[[nodiscard]] bool UpdateBuffers();

	[[nodiscard]] bool BindDebugDrawBuffers() const;
	[[nodiscard]] bool BindShadowCasterBuffers() const;
	[[nodiscard]] bool BindGeometryBuffers() const;
	[[nodiscard]] bool BindPSLightingBuffers() const;
	[[nodiscard]] bool BindCSLightingBuffers() const;
	[[nodiscard]] bool BindTransparentBuffers() const;
	[[nodiscard]] bool BindViewBuffers() const;
	[[nodiscard]] bool BindInverseBuffers() const;

	[[nodiscard]] bool StoreBounds(DirectX::BoundingFrustum &bounds, bool includeScale);
	[[nodiscard]] bool StoreBounds(DirectX::BoundingOrientedBox &bounds, bool includeScale);

	[[nodiscard]] const DirectX::BoundingFrustum *GetLightGridFrustums();

	void QueueGeometry(const ResourceGroup &resource, const RenderInstance &instance);
	void QueueTransparent(const ResourceGroup &resource, const RenderInstance &instance);
	void ResetRenderQueue();

	[[nodiscard]] UINT GetCullCount() const;
	[[nodiscard]] std::multimap<ResourceGroup, RenderInstance> &GetGeometryQueue();
	[[nodiscard]] std::multimap<ResourceGroup, RenderInstance> &GetTransparentQueue();
	[[nodiscard]] std::multimap<ResourceGroup, RenderInstance> &GetOverlayQueue();

	void SetFOV(float fov);
	void SetOrtho(bool state);
	void SetPlanes(CameraPlanes planes);
	[[nodiscard]] float GetFOV() const;
	[[nodiscard]] bool GetOrtho() const;
	[[nodiscard]] CameraPlanes GetPlanes() const;

	void SetRendererInfo(const RendererInfo &rendererInfo);
	[[nodiscard]] RendererInfo GetRendererInfo() const;

	[[nodiscard]] ID3D11Buffer *GetCameraVSBuffer() const;
	[[nodiscard]] ID3D11Buffer *GetCameraGSBuffer() const;
	[[nodiscard]] ID3D11Buffer *GetCameraCSBuffer() const;

	// Returns a world-space ray from the camera in the direction of the screen position.
	void GetViewRay(
		/* in */ const DirectX::XMFLOAT2A &screenPos, const DirectX::XMFLOAT2A &screenSize,
		/* out */ DirectX::XMFLOAT3A &origin, DirectX::XMFLOAT3A &direction);

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string *code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string &code) override;

};
