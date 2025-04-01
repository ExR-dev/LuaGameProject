#pragma once

#include "RendererInfo.h"
#include "CameraBehaviour.h"
#include "CameraCubeBehaviour.h"

class RenderQueuer
{
public:
	virtual void QueueGeometry(const ResourceGroup &resource, const RenderInstance &instance) const = 0;
	virtual void QueueTransparent(const ResourceGroup &resource, const RenderInstance &instance) const = 0;
};

class CamRenderQueuer final : public RenderQueuer
{
private:
	CameraBehaviour *_cameraBehaviour = nullptr;

public:
	CamRenderQueuer(CameraBehaviour *cameraBehaviour)
	{
		_cameraBehaviour = cameraBehaviour;
	}

	void QueueGeometry(const ResourceGroup &resource, const RenderInstance &instance) const override
	{
		_cameraBehaviour->QueueGeometry(resource, instance);
	}
	void QueueTransparent(const ResourceGroup &resource, const RenderInstance &instance) const override
	{
		_cameraBehaviour->QueueTransparent(resource, instance);
	}
};

class CubeRenderQueuer final : public RenderQueuer
{
private:
	CameraCubeBehaviour *_cameraCubeBehaviour = nullptr;
	UINT _sideIndex = 0;

public:
	CubeRenderQueuer(CameraCubeBehaviour *cameraCubeBehaviour, UINT cameraIndex)
	{
		_cameraCubeBehaviour = cameraCubeBehaviour;
		_sideIndex = cameraIndex;
	}

	[[nodiscard]] UINT GetSideIndex() const
	{
		return _sideIndex;
	}

	void QueueGeometry(const ResourceGroup &resource, const RenderInstance &instance) const override
	{
		_cameraCubeBehaviour->QueueGeometry(_sideIndex, resource, instance);
	}
	void QueueTransparent(const ResourceGroup &resource, const RenderInstance &instance) const override
	{
		_cameraCubeBehaviour->QueueTransparent(_sideIndex, resource, instance);
	}
};