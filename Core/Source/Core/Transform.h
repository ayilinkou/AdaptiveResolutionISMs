#pragma once

#include <algorithm>
#include <cmath>

#include "DirectXMath.h"

struct Transform
{
	DirectX::XMFLOAT3 Position = { 0.f, 0.f, 0.f };
	DirectX::XMFLOAT3 Rotation = { 0.f, 0.f, 0.f };
	DirectX::XMFLOAT3 Scale = { 1.f, 1.f, 1.f };

	void SetRotation(float pitch, float yaw)
	{
		pitch = std::clamp(pitch, -89.9f, 89.9f);
		yaw = std::fmod(yaw, 360.f);

		Rotation = { pitch, yaw, 0.f };
	}
};