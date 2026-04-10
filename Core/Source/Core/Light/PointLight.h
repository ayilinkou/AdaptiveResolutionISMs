#pragma once

#include <vector>
#include <array>

#include "Light.h"

namespace Core {
	struct PointLightData
	{
		DirectX::XMFLOAT3 Color = { 1.f, 1.f, 1.f };
		float SpecularPower = 512.f;
		DirectX::XMFLOAT3 Position = { 0.f, 0.f, 0.f };
		float Intensity = 1.f;
		DirectX::XMFLOAT3 Attenuation = { 1.f, 0.f, 0.f }; // x = quadratic, y = linear, z = constant
		float Radius = 50.f;
	};

	class PointLight : public Light
	{
	public:
		PointLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 attenuation);
		virtual ~PointLight();

		virtual void RenderControls() override;

		void SetPosition(float x, float y, float z);
		void SetAccumTransform(const DirectX::XMMATRIX& transform) { m_AccumTransform = transform; }
		virtual void SetColor(float r, float g, float b) override { m_Data.Color = { r, g, b }; }
		virtual void SetSpecularPower(float power) override { m_Data.SpecularPower = power; }
		virtual void SetIntensity(float intensity) { m_Data.Intensity = intensity; }

		const PointLightData& GetData() const { return m_Data; }
		const std::array<DirectX::XMMATRIX, 6> GetViewsT() const;
		const DirectX::XMMATRIX GetProjT() const { return DirectX::XMMatrixTranspose(m_Proj); }
		static const D3D11_VIEWPORT& GetShadowMapViewport() { return s_ShadowMapViewport; }
		static std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>>& GetRTVs() { return s_RTVs; }
		static Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& GetDSV() { return s_DSV; }
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowMapsSRV() { return s_ShadowMapsSRV; }

	private:
		static void InitStatics();
		static void ShutdownStatics();

		void UpdateProj();

	private:
		PointLightData m_Data;
		
		DirectX::XMFLOAT3 m_LocalPosition;
		DirectX::XMMATRIX m_AccumTransform = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX m_Proj;

		static D3D11_VIEWPORT s_ShadowMapViewport;
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> s_ShadowMapsSRV;
		static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> s_DSV;
		static std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> s_RTVs;

		friend class LightManager;
	};
}