#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

#define TO_VEC(x)		(*reinterpret_cast<DirectX::XMVECTOR *>(&(x)))
#define TO_VEC_PTR(x)	( reinterpret_cast<DirectX::XMVECTOR *>(&(x)))
#define TO_CONST_VEC(x)	(*reinterpret_cast<const DirectX::XMVECTOR *>(&(x)))

#define SIGN(x)	(x > 0.0f ? 1.0f : -1.0f)

static constexpr float DEG_TO_RAD = (DirectX::XM_PI / 180.0f);
static constexpr float RAD_TO_DEG = (180.0f / DirectX::XM_PI);

[[nodiscard]] static inline const DirectX::XMFLOAT4A To4(const DirectX::XMFLOAT3A &vec)
{
	return *reinterpret_cast<const DirectX::XMFLOAT4A *>(&vec);
}
[[nodiscard]] static inline const DirectX::XMFLOAT4A To4(const DirectX::XMFLOAT3 &vec)
{
	return DirectX::XMFLOAT4A(vec.x, vec.y, vec.z, 0.0f);
}
[[nodiscard]] static inline const DirectX::XMFLOAT3A To3(const DirectX::XMFLOAT4A &vec)
{
	return *reinterpret_cast<const DirectX::XMFLOAT3A *>(&vec);
}
[[nodiscard]] static inline const DirectX::XMFLOAT3A To3(const DirectX::XMFLOAT4 &vec)
{
	return *reinterpret_cast<const DirectX::XMFLOAT3A *>(&vec);
}
[[nodiscard]] static inline const DirectX::XMFLOAT3A To3(const DirectX::XMFLOAT3 &vec)
{
	return DirectX::XMFLOAT3A(vec.x, vec.y, vec.z);
}
[[nodiscard]] static inline const DirectX::XMFLOAT3 To3(const DirectX::XMFLOAT3A &vec)
{
	return DirectX::XMFLOAT3(vec.x, vec.y, vec.z);
}
[[nodiscard]] static inline const float To1(const DirectX::XMFLOAT4A &vec)
{
	return vec.x;
}
[[nodiscard]] static inline const float To1(const DirectX::XMFLOAT3A &vec)
{
	return vec.x;
}
[[nodiscard]] static inline const float To1(const DirectX::XMFLOAT4 &vec)
{
	return vec.x;
}
[[nodiscard]] static inline const float To1(const DirectX::XMFLOAT3 &vec)
{
	return vec.x;
}

[[nodiscard]] static inline DirectX::XMVECTOR Load(const float &f)
{
	return DirectX::XMLoadFloat(&f);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT3 &float3)
{
	return DirectX::XMLoadFloat3(&float3);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT4 &float4)
{
	return DirectX::XMLoadFloat4(&float4);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT3A &float3A)
{
	return DirectX::XMLoadFloat3A(&float3A);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT4A &float4A)
{
	return DirectX::XMLoadFloat4A(&float4A);
}
[[nodiscard]] static inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT3X3 &float3x3)
{
	return DirectX::XMLoadFloat3x3(&float3x3);
}
[[nodiscard]] static inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT4X4 &float4x4)
{
	return DirectX::XMLoadFloat4x4(&float4x4);
}
[[nodiscard]] static inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT4X4A &float4x4A)
{
	return DirectX::XMLoadFloat4x4A(&float4x4A);
}

[[nodiscard]] static inline DirectX::XMVECTOR Load(const float *f)
{
	return DirectX::XMLoadFloat(f);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT3 *float3)
{
	return DirectX::XMLoadFloat3(float3);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT4 *float4)
{
	return DirectX::XMLoadFloat4(float4);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT3A *float3A)
{
	return DirectX::XMLoadFloat3A(float3A);
}
[[nodiscard]] static inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT4A *float4A)
{
	return DirectX::XMLoadFloat4A(float4A);
}
[[nodiscard]] static inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT3X3 *float3x3)
{
	return DirectX::XMLoadFloat3x3(float3x3);
}
[[nodiscard]] static inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT4X4 *float4x4)
{
	return DirectX::XMLoadFloat4x4(float4x4);
}
[[nodiscard]] static inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT4X4A *float4x4A)
{
	return DirectX::XMLoadFloat4x4A(float4x4A);
}

static inline void Store(float &dest, const DirectX::XMVECTOR &vec)
{
	DirectX::XMStoreFloat(&dest, vec);
}
static inline void Store(DirectX::XMFLOAT3 &dest, const DirectX::XMVECTOR &vec)
{
	DirectX::XMStoreFloat3(&dest, vec);
}
static inline void Store(DirectX::XMFLOAT4 &dest, const DirectX::XMVECTOR &vec)
{
	DirectX::XMStoreFloat4(&dest, vec);
}
static inline void Store(DirectX::XMFLOAT3A &dest, const DirectX::XMVECTOR &vec)
{
	DirectX::XMStoreFloat3A(&dest, vec);
}
static inline void Store(DirectX::XMFLOAT4A &dest, const DirectX::XMVECTOR &vec)
{
	DirectX::XMStoreFloat4A(&dest, vec);
}
static inline void Store(DirectX::XMFLOAT3X3 &dest, const DirectX::XMMATRIX &mat)
{
	DirectX::XMStoreFloat3x3(&dest, mat);
}
static inline void Store(DirectX::XMFLOAT4X4 &dest, const DirectX::XMMATRIX &mat)
{
	DirectX::XMStoreFloat4x4(&dest, mat);
}
static inline void Store(DirectX::XMFLOAT4X4A &dest, const DirectX::XMMATRIX &mat)
{
	DirectX::XMStoreFloat4x4A(&dest, mat);
}

/// Calculate the reach of a light based on its falloff and color value.
/// This is done to prevent harsh edges between light tiles.
/// This cutoff is matched by shaders using CutoffLight() in LightData.hlsli.
[[nodiscard]] static inline float CalculateLightReach(DirectX::XMFLOAT3 color, float falloff)
{
	// The intensity at which the light is considered to have no effect.
	// Must match the value of the same name in CutoffLight().
	float intensityCutoff = 0.01f; // previously 0.005

	float value = max(color.x, max(color.y, color.z));

	// intensity = value / (1 + (distance * falloff)^2)
	return sqrt((value / intensityCutoff) - 1.0f) / falloff;
};

[[nodiscard]] static inline DirectX::XMFLOAT3 RGBtoHSV(DirectX::XMFLOAT3 rgb)
{
	float
		r = rgb.x,
		g = rgb.y,
		b = rgb.z;

	float max = max(r, max(g, b));
	float min = min(r, min(g, b));
	float diff = max - min;

	float h = 0.0, s, v;

	if (max == min) 	h = 0.0f;
	else if (max == r)	h = fmodf((60.0f * ((g - b) / diff) + 360.0f), 360.0f);
	else if (max == g)	h = fmodf((60.0f * ((b - r) / diff) + 120.0f), 360.0f);
	else if (max == b)	h = fmodf((60.0f * ((r - g) / diff) + 240.0f), 360.0f);
	else				h = 0.0f;

	s = (max == 0.0f) ? (0.0f) : ((diff / max) * 1.0f);
	v = max;

	return { h, s, v };
};
[[nodiscard]] static inline DirectX::XMFLOAT3 HSVtoRGB(DirectX::XMFLOAT3 hsv)
{
	float
		r = 0.0f,
		g = 0.0f,
		b = 0.0f;

	if (hsv.y == 0.0f)
	{
		r = hsv.z;
		g = hsv.z;
		b = hsv.z;
	}
	else
	{
		int i;
		float f, p, q, t;

		if (hsv.x == 360.0f)
			hsv.x = 0.0f;
		else
			hsv.x = hsv.x / 60.0f;

		i = (int)trunc(hsv.x);
		f = hsv.x - i;

		p = hsv.z * (1.0f - hsv.y);
		q = hsv.z * (1.0f - (hsv.y * f));
		t = hsv.z * (1.0f - (hsv.y * (1.0f - f)));

		switch (i)
		{
		case 0:
			r = hsv.z;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = hsv.z;
			b = p;
			break;

		case 2:
			r = p;
			g = hsv.z;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = hsv.z;
			break;

		case 4:
			r = t;
			g = p;
			b = hsv.z;
			break;

		default:
			r = hsv.z;
			g = p;
			b = q;
			break;
		}

	}

	return {
		r,
		g,
		b
	};
};

[[nodiscard]] static inline float RandomFloat(float min, float max)
{
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
};
