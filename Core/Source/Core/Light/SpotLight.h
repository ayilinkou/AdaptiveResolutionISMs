#pragma once

#include <vector>

#include "Light.h"

namespace Core {
	struct SpotLightData
	{
		DirectX::XMFLOAT3 Color = { 1.f, 1.f, 1.f };
		float SpecularPower = 512.f;
		DirectX::XMFLOAT3 Position = { 0.f, 0.f, 0.f };
		float Intensity = 1.f;
		DirectX::XMFLOAT3 Attenuation = { 1.f, 0.f, 0.f }; // x = quadratic, y = linear, z = constant
		float Radius = 50.f;
		DirectX::XMFLOAT3 Direction = { 0.f, -1.f, 0.f };
		float CosInnerAngle;
		float CosOuterAngle;
		DirectX::XMFLOAT3 Padding;
		DirectX::XMMATRIX ViewProj;
	};

	class SpotLight : public Light
	{
	public:
		SpotLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 attenuation, DirectX::XMFLOAT3 dir);
		virtual ~SpotLight();

		virtual void RenderControls() override;

		void SetPosition(float x, float y, float z);
		void SetDirection(float x, float y, float z);
		void SetAngles(float inner, float outer);
		void SetAttenuation(float quadratic, float linear, float constant) { m_Data.Attenuation = { quadratic, linear, constant }; }
		void SetAccumTransform(const DirectX::XMMATRIX& transform) { m_AccumTransform = transform; }
		virtual void SetColor(float r, float g, float b) override { m_Data.Color = { r, g, b }; }
		virtual void SetSpecularPower(float power) override { m_Data.SpecularPower = power; }
		virtual void SetIntensity(float intensity) { m_Data.Intensity = intensity; }

		const SpotLightData& GetData() const { return m_Data; }
		const DirectX::XMMATRIX& GetViewProjT() const { return m_Data.ViewProj; }
		static const D3D11_VIEWPORT& GetShadowMapViewport() { return s_ShadowMapViewport; }
		static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>>& GetDSVs() { return s_DSVs; }
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowMapsSRV() { return s_ShadowMapsSRV; }

	private:
		static void InitStatics();
		static void ShutdownStatics();

		void UpdateView();
		void UpdateProj();
		void UpdateViewProj();

	private:
		SpotLightData m_Data;

		DirectX::XMFLOAT3 m_LocalPosition;
		DirectX::XMMATRIX m_AccumTransform = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX m_View;
		DirectX::XMMATRIX m_Proj;
		float m_ConeInnerAngle = 60.f;
		float m_ConeOuterAngle = 89.f;

		static D3D11_VIEWPORT s_ShadowMapViewport;
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> s_ShadowMapsSRV;
		static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> s_DSVs;

		friend class LightManager;
	};
}