#pragma once

#include <algorithm>
#include <cmath>

#include "DirectXMath.h"

namespace Core {
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

		DirectX::XMMATRIX GetWorldMatrix() const
		{
			DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
			matrix *= DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
			matrix *= DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(Rotation.y));
			matrix *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(Rotation.x));
			matrix *= DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(Rotation.z));
			matrix *= DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
			return matrix;
		}
	};
}