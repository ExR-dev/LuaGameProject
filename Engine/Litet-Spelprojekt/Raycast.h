#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

struct RaycastOut
{
    Entity *entity = nullptr;
    float distance = FLT_MAX;
};

static bool Raycast(
    const DirectX::XMFLOAT3 &origin, const DirectX::XMFLOAT3 &direction, 
    const DirectX::BoundingBox &box, float &length)
{
	if (box.Contains(XMLoadFloat3(&origin)))
	{
		length = 0.0f;
		return true;
	}

	if (box.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), length))
	{
		return true;
	}

    return false;
}

static bool Raycast(
    const DirectX::XMFLOAT3 &origin, const DirectX::XMFLOAT3 &direction,
    const DirectX::BoundingOrientedBox &box, float &length)
{
    /*if (box.Contains(XMLoadFloat3(&origin)))
    {
        length = 0.0f;
        return true;
    }*/

	length = FLT_MAX;

	DirectX::XMFLOAT3 p;
	DirectX::XMFLOAT3 normal;
	int side;

	float minV = -1 * FLT_MAX,
		maxV = FLT_MAX;

	DirectX::XMVECTOR
		r0 = DirectX::XMLoadFloat3(&origin),
		rD = DirectX::XMLoadFloat3(&direction);

	DirectX::XMVECTOR
		orientation = DirectX::XMLoadFloat4(&box.Orientation),
		halfLength = DirectX::XMLoadFloat3(&box.Extents),
		center = DirectX::XMLoadFloat3(&box.Center),
		axes[3] = {
			DirectX::XMVector3Rotate({ 1, 0, 0 }, orientation),
			DirectX::XMVector3Rotate({ 0, 1, 0 }, orientation),
			DirectX::XMVector3Rotate({ 0, 0, 1 }, orientation)
	};

	DirectX::XMVECTOR
		rayToCenter = DirectX::XMVectorSubtract(center, r0),
		nMin = DirectX::XMVectorSet(0, 0, 0, 0),
		nMax = DirectX::XMVectorSet(0, 0, 0, 0);

	for (int a = 0; a < 3; a++)
	{
		DirectX::XMVECTOR axis = axes[a];
		float hl = halfLength.m128_f32[a];

		float distAlongAxis = DirectX::XMVector3Dot(axis, rayToCenter).m128_f32[0],
			f = DirectX::XMVector3Dot(axis, rD).m128_f32[0];

		if (fabsf(f) > FLT_MIN)
		{
			DirectX::XMVECTOR tnMin = axis,
				tnMax = DirectX::XMVectorScale(axis, -1);

			float t0 = (distAlongAxis + hl) / f,
				t1 = (distAlongAxis - hl) / f;

			if (t0 > t1) // Flip Intersection Order
			{
				float temp = t0;
				t0 = t1;
				t1 = temp;

				tnMin = tnMax;
				tnMax = axis;
			}

			if (t0 > minV) // Keep the longer entry-point
			{
				minV = t0;
				nMin = tnMin;
			}

			if (t1 < maxV) // Keep the shorter exit-point
			{
				maxV = t1;
				nMax = tnMax;
			}

			if (minV > maxV) return false; // Ray misses OBB
			if (maxV < 0.0f) return false; // OBB is behind Ray
		}
		else if (-distAlongAxis - hl > 0.0f || -distAlongAxis + hl < 0.0f)
		{
			return false; // Ray is orthogonal to axis but no located between the axis-planes
		}
	}

	// Find closest positive intersection
	if (minV > 0.0f)
	{
		length = minV;
		DirectX::XMStoreFloat3(&normal, DirectX::XMVector3Normalize(nMin));
		side = 1;
	}
	else
	{
		length = maxV;
		DirectX::XMStoreFloat3(&normal, DirectX::XMVectorNegate(DirectX::XMVector3Normalize(DirectX::XMVectorNegate(nMax))));
		side = -1;
	}

	DirectX::XMStoreFloat3(&p, DirectX::XMVectorAdd(r0, DirectX::XMVectorScale(rD, length)));
	return true;
}