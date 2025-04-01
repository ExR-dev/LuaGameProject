#pragma once

#include <vector>
#include <map>
#include <d3d11_4.h>
#include <DirectXCollision.h>
#include <DirectXMath.h>

#include "Behaviour.h"
#include "CameraBehaviour.h"
#include "Content.h"
#include "RendererInfo.h"

class RenderQueuer;

class CameraCubeBehaviour final : public Behaviour
{
private:
	struct CubeSide
	{
		DirectX::BoundingFrustum bounds, transformedBounds;

		ConstantBufferD3D11 viewProjBuffer;
		std::unique_ptr<ConstantBufferD3D11> viewProjPosBuffer = nullptr;
		std::unique_ptr<ConstantBufferD3D11> posBuffer = nullptr;

		// Let batching be handled by multimap
		std::multimap<ResourceGroup, RenderInstance> geometryRenderQueue;
		std::multimap<ResourceGroup, RenderInstance> transparentRenderQueue;
	};

	std::array<CubeSide, 6> _cubeSides;
	RendererInfo _rendererInfo;
	CameraPlanes _cameraPlanes;

	DirectX::BoundingBox _bounds, _transformedBounds;

	bool _hasCSBuffer = false;
	bool _recalculateFrustumBounds = true;
	bool _recalculateBounds = true;
	bool _isDirty = true;
	bool _invertedDepth = false;

	UINT _lastCullCount = 0;

	void GetAxes(UINT cameraIndex, DirectX::XMFLOAT3A *right, DirectX::XMFLOAT3A *up, DirectX::XMFLOAT3A *forward);
	[[nodiscard]] DirectX::XMFLOAT4A GetRotation(UINT cameraIndex);

protected:
	[[nodiscard]] bool Start() override;

	void OnDirty() override;

#ifdef USE_IMGUI
	[[nodiscard]] bool RenderUI() override;
#endif

	[[nodiscard]] bool Serialize(std::string *code) const override;

	[[nodiscard]] bool Deserialize(const std::string &code) override;

public:
	CameraCubeBehaviour(const CameraPlanes &planes, bool hasCSBuffer, bool invertDepth = true);
	~CameraCubeBehaviour() = default;

	[[nodiscard]] DirectX::XMFLOAT4X4A GetViewMatrix(UINT cameraIndex);
	[[nodiscard]] DirectX::XMFLOAT4X4A GetProjectionMatrix() const;
	[[nodiscard]] DirectX::XMFLOAT4X4A GetViewProjectionMatrix(UINT cameraIndex);
	
	/// This must be called from the behaviour in control of the camera.
	[[nodiscard]] bool UpdateBuffers();

	[[nodiscard]] bool BindShadowCasterBuffers(UINT cameraIndex) const;
	[[nodiscard]] bool BindGeometryBuffers(UINT cameraIndex) const;
	[[nodiscard]] bool BindLightingBuffers(UINT cameraIndex) const;
	[[nodiscard]] bool BindTransparentBuffers(UINT cameraIndex) const;
	[[nodiscard]] bool BindViewBuffers(UINT cameraIndex) const;
	[[nodiscard]] bool BindMainBuffers(UINT cameraIndex) const;

	[[nodiscard]] bool StoreBounds(DirectX::BoundingFrustum &bounds, UINT cameraIndex);
	[[nodiscard]] bool StoreBounds(DirectX::BoundingBox &bounds);

	void QueueGeometry(UINT cameraIndex, const ResourceGroup &resource, const RenderInstance &instance);
	void QueueTransparent(UINT cameraIndex, const ResourceGroup &resource, const RenderInstance &instance);
	void ResetRenderQueue();

	[[nodiscard]] UINT GetCullCount() const;
	void SetCullCount(UINT cullCount);
	[[nodiscard]]std::multimap<ResourceGroup, RenderInstance> &GetGeometryQueue(UINT cameraIndex);
	[[nodiscard]]std::multimap<ResourceGroup, RenderInstance> &GetTransparentQueue(UINT cameraIndex);

	void SetRendererInfo(const RendererInfo &rendererInfo);
	void SetFarZ(float farZ);
	[[nodiscard]] RendererInfo GetRendererInfo() const;
	[[nodiscard]] float GetFarZ() const;
	
	[[nodiscard]] ID3D11Buffer *GetCameraVSBuffer(UINT cameraIndex) const;
	[[nodiscard]] ID3D11Buffer *GetCameraGSBuffer(UINT cameraIndex) const;
	[[nodiscard]] ID3D11Buffer *GetCameraCSBuffer(UINT cameraIndex) const;
};
