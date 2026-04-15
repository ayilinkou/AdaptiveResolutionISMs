#pragma once

#include <vector>

#include "Light.h"
#include "Core/Renderer/RenderQueue.h"

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
		float NearZ;
		float MinBias;
		float MaxBias;
		DirectX::XMMATRIX ViewProj;
		UINT ShadowMapRes;
		UINT GlobalID;
		UINT ShadowType;
		float Padding;
	};

	class SpotLight : public Light
	{
	public:
		SpotLight(DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 attenuation, DirectX::XMFLOAT3 dir);
		virtual ~SpotLight();

		virtual void RenderControls() override;

		void SetPosition(const DirectX::XMFLOAT3 pos);
		void SetPosition(float x, float y, float z);
		void SetDirection(float x, float y, float z);
		void SetAngles(float inner, float outer);
		void SetRadius(float radius) { m_Data.Radius = radius; }
		void SetAttenuation(float quadratic, float linear, float constant) { m_Data.Attenuation = { quadratic, linear, constant }; }
		void SetAccumTransform(const DirectX::XMMATRIX& transform) { m_AccumTransform = transform; }
		void SetShadowType(ShadowType newShadowType) { m_Data.ShadowType = (UINT)newShadowType; }
		void SetShadowMapRes(UINT newShadowMapRes) { m_Data.ShadowMapRes = newShadowMapRes; }
		virtual void SetColor(float r, float g, float b) override { m_Data.Color = { r, g, b }; }
		virtual void SetSpecularPower(float power) override { m_Data.SpecularPower = power; }
		virtual void SetIntensity(float intensity) { m_Data.Intensity = intensity; }

		const SpotLightData& GetData() const { return m_Data; }
		const DirectX::XMMATRIX GetViewT() const { return DirectX::XMMatrixTranspose(m_View); }
		const DirectX::XMMATRIX GetProjT() const { return DirectX::XMMatrixTranspose(m_Proj); }
		const DirectX::XMMATRIX& GetViewProjT() const { return m_Data.ViewProj; }
		float GetNearZ() const { return m_NearZ; }
		float GetFarZ() const { return m_Data.Radius; }
		ShadowType GetShadowType() const { return (ShadowType)m_Data.ShadowType; }
		const UINT GetGlobalID() const { return m_Data.GlobalID; }

		static const D3D11_VIEWPORT& GetShadowMapViewport() { return s_ShadowMapViewport; }
		static const D3D11_VIEWPORT& GetISMViewport() { return s_ISMViewport; }
		static const D3D11_VIEWPORT& GetLowISMViewport() { return s_LowISMViewport; }
		static constexpr UINT GetShadowMapRes() { return s_SHADOW_MAP_RES; }
		static constexpr UINT GetISMRes() { return s_ISM_RES; }
		static constexpr UINT GetLowISMRes() { return s_LOW_ISM_RES; }
		static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>>& GetShadowMapDSVs() { return s_ShadowMapDSVs; }
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShadowMapsSRV() { return s_ShadowMapsSRV; }
		static std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>>& GetISM_UAVs() { return s_ISM_UAVs; }
		static std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetISMMipSRVs() { return s_ISMMipSRVs; }
		static std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>>& GetLowISM_UAVs() { return s_LowISM_UAVs; }
		static std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetLowISMMipSRVs() { return s_LowISMMipSRVs; }

	private:
		static void InitStatics();
		static void ShutdownStatics();

		static void InitISMs(UINT res, const std::string& name, std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>>& UAVs,
			std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& SRVs);

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
		const float m_NearZ = 0.1f;

		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> s_ShadowMapsSRV;
		static std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> s_ShadowMapDSVs;
		static std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> s_ISMMipSRVs;
		static std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>> s_ISM_UAVs;
		static std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> s_LowISMMipSRVs;
		static std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>>> s_LowISM_UAVs;

		static constexpr UINT s_SHADOW_MAP_RES = 1024u;
		static constexpr UINT s_ISM_RES = 256u;
		static constexpr UINT s_LOW_ISM_RES = 128u;
		static constexpr D3D11_VIEWPORT s_ShadowMapViewport = { 0.f, 0.f, (float)s_SHADOW_MAP_RES, (float)s_SHADOW_MAP_RES, 0.f, 1.f };
		static constexpr D3D11_VIEWPORT s_ISMViewport = { 0.f, 0.f, (float)s_ISM_RES, (float)s_ISM_RES, 0.f, 1.f };
		static constexpr D3D11_VIEWPORT s_LowISMViewport = { 0.f, 0.f, (float)s_LOW_ISM_RES, (float)s_LOW_ISM_RES, 0.f, 1.f };
		static UINT s_GlobalID;

		friend class LightManager;
	};
}