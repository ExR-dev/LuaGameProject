#pragma once
#include "Behaviour.h"

class RestrictedViewBehaviour final : public Behaviour
{
private:
	DirectX::XMFLOAT3A _startOrientation = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3A _viewDirection = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3A _upDirection = { 0.0f, 1.0f, 0.0f };

	float _allowedDotOffset = 1.0f;
	float _offsetInDeg = 90.0f;
	float _prevOffset = 1.0f;

protected:
	[[nodiscard]] bool Start() override;
	[[nodiscard]] bool Update(Time& time, const Input& input) override;

public:
	RestrictedViewBehaviour() = default;
	~RestrictedViewBehaviour() = default;

	// Serializes the behaviour to a string.
	[[nodiscard]] bool Serialize(std::string* code) const override;

	// Deserializes the behaviour from a string.
	[[nodiscard]] bool Deserialize(const std::string& code) override;

	void SetViewDirection(const DirectX::XMFLOAT3 &orientation, const DirectX::XMFLOAT3 &viewDirection, const DirectX::XMFLOAT3 &upDirection);
	void SetAllowedOffset(const float &offsetDegrees);
};