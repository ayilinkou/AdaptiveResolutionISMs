#pragma once

#include <DirectXMath.h>

#include "Core/Component/SceneComponent.h"

namespace Core {
	class Camera : public SceneComponent
	{
	public:
		Camera() = delete;
		Camera(const DirectX::XMMATRIX& proj, float nearZ, float farZ);

		void SetPosition(float x, float y, float z) { m_Transform.Position = { x, y, z }; }
		void SetRotation(float pitch, float yaw) { m_Transform.SetRotation(pitch, yaw); }

		void CalcViewMatrix();

		DirectX::XMFLOAT3 GetLookDir() const { return m_LookDir; }
		DirectX::XMFLOAT3 GetPosition() const { return m_Transform.Position; }
		float* GetPositionPtr() { return reinterpret_cast<float*>(&m_Transform.Position); }

		DirectX::XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
		DirectX::XMMATRIX GetProjMatrix() const { return m_ProjMatrix; }
		DirectX::XMMATRIX GetViewProjMatrix() const { return m_ViewMatrix * GetProjMatrix(); }

		float GetNearZ() const { return m_NearZ; }
		float GetFarZ() const { return m_FarZ; }

		void RotateCamera(float mouseX, float mouseY);
		void AddWASDVector(const DirectX::XMFLOAT3 v) { m_wasdVector.x += v.x; m_wasdVector.y += v.y; m_wasdVector.z += v.z; }
		void AddQEVector(float v) { m_qeVector += v; }
		void MoveCamera(float dt);

	private:
		DirectX::XMFLOAT3 m_LookDir;
		DirectX::XMMATRIX m_ViewMatrix;
		const DirectX::XMMATRIX m_ProjMatrix;

		DirectX::XMMATRIX m_RotationMatrix;

		const float m_NearZ;
		const float m_FarZ;

		float m_CameraSpeed = 10.f;
		float m_MouseSens = 0.1f;

		DirectX::XMFLOAT3 m_wasdVector;
		float m_qeVector;
	};
}
