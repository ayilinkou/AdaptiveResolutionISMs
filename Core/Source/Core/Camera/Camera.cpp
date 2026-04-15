#include "Camera.h"

namespace Core {
	Camera::Camera(const DirectX::XMMATRIX& proj, float nearZ, float farZ)
		: m_ProjMatrix(proj), m_NearZ(nearZ), m_FarZ(farZ)
	{
		m_Transform.Position = { 0.f, 0.f, -5.f };
		m_LookDir = { 0.f, 0.f, 1.f };
		m_wasdVector = { 0.f, 0.f, 0.f };
		m_qeVector = 0.f;
	}

	void Camera::CalcViewMatrix()
	{
		DirectX::XMFLOAT3 up = { 0.f, 1.f, 0.f };
		DirectX::XMVECTOR upVector, positionVector, lookAtVector;

		float pitch = DirectX::XMConvertToRadians(m_Transform.Rotation.x);
		float yaw = DirectX::XMConvertToRadians(m_Transform.Rotation.y);
		constexpr float roll = DirectX::XMConvertToRadians(0.f);

		positionVector = DirectX::XMLoadFloat3(&m_Transform.Position);

		m_RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

		lookAtVector = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&m_LookDir));
		lookAtVector = DirectX::XMVector3TransformCoord(lookAtVector, m_RotationMatrix);
		lookAtVector = DirectX::XMVectorAdd(positionVector, lookAtVector);

		upVector = DirectX::XMLoadFloat3(&up);
		upVector = DirectX::XMVector3TransformCoord(upVector, m_RotationMatrix);

		m_ViewMatrix = DirectX::XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
	}

	void Camera::RotateCamera(float mouseX, float mouseY)
	{
		m_Transform.SetRotation(m_Transform.Rotation.x - mouseY * m_MouseSens, m_Transform.Rotation.y + mouseX * m_MouseSens);
	}

	void Camera::MoveCamera(float dt)
	{
		if (m_wasdVector.x == 0.f &&
			m_wasdVector.y == 0.f &&
			m_wasdVector.z == 0.f &&
			m_qeVector == 0.f)
		{
			return;
		}
		
		// Q and E are for moving along the Y axis only, therefore only WASD must be transformed
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&m_wasdVector);
		DirectX::XMVECTOR qe = DirectX::XMVectorSet(0.f, m_qeVector, 0.f, 0.f);
		v = DirectX::XMVector3TransformCoord(v, m_RotationMatrix);
		v = DirectX::XMVectorAdd(v, qe);
		v = DirectX::XMVector3Normalize(v);
		v = DirectX::XMVectorScale(v, m_CameraSpeed * dt);

		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_Transform.Position);
		pos = DirectX::XMVectorAdd(v, pos);

		DirectX::XMStoreFloat3(&m_Transform.Position, pos);
		m_wasdVector = { 0.f, 0.f, 0.f };
		m_qeVector = 0.f;
	}
}
