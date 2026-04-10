#pragma once

#include <memory>

#include "d3d11.h"
#include "DirectXMath.h"
#include "wrl.h"

#include "Core/Window/Window.h"
#include "Core/Shader/ShaderProgram.h"
#include "Core/Light/LightManager.h"
#include "Core/Utility/Constants.h"

namespace Core {

	class Camera;

	using namespace Microsoft::WRL;
	
	struct CameraBuffer
	{
		DirectX::XMMATRIX View;
		DirectX::XMMATRIX Proj;
		DirectX::XMFLOAT3 CameraPos;
		float Padding;
	};

	struct GlobalCBuffer
	{
		CameraBuffer CameraData;
		LightBuffer LightData;
		UINT ScreenWidth;
		UINT ScreenHeight;
		float NearZ;
		float FarZ;
		float Time;
		DirectX::XMFLOAT3 Padding;
	};

	struct RendererSpec
	{
		float NearPlane;
		float FarPlane;
		HWND hwnd;
		WindowSpec WinSpec;
	};

	struct VramInfo
	{
		UINT64 Budget;
		UINT64 CurrentUsage;
	};
	
	class Renderer
	{
	public:
		Renderer(const RendererSpec& spec);
		~Renderer();

		static Renderer* Get() { return s_pInstance; }

		void Init();
		void Shutdown();

		void CreateInputLayouts();
		void CreateShaderPrograms();
		void DestroyShaderPrograms();

		void BeginScene();
		void EndScene();

		void EnableBlending();
		void DisableBlending();
		void EnableDepthWrite();
		void DisableDepthWrite();

		void BindForOpaqueDraws();
		void BindForTransparentDraws();
		void BindForDSVShadowPass();
		void BindForPointLightShadowPass();
		void SetBackFaceCulling(bool bEnabled);
		void SetBackBufferViewport();

		DirectX::XMFLOAT4& GetClearColor() { return m_ClearColor; }
		void SetClearColor(DirectX::XMFLOAT4 clearColor) { m_ClearColor = clearColor; }
		void ResetClearColor() { m_ClearColor = m_BaseClearColor; }

		// this shows VRAM usage for the adapter across all processes, not just this application
		VramInfo QueryVramUsage() const;

	private:
		struct AdapterAndOutput
		{
			ComPtr<IDXGIAdapter> Adapter;
			ComPtr<IDXGIOutput> Output;
			DXGI_MODE_DESC Mode = {};
		};

		AdapterAndOutput GetBestAdapterAndOutput() const;

	private:
		ComPtr<ID3D11Device> m_Device;
		ComPtr<ID3D11DeviceContext> m_Context;
		ComPtr<IDXGISwapChain> m_SwapChain;
		ComPtr<ID3D11RenderTargetView> m_BackBufferRTV;
		ComPtr<ID3D11Texture2D> m_DepthStencilTexture;
		ComPtr<ID3D11DepthStencilView> m_DSV;
		ComPtr<ID3D11DepthStencilState> m_DepthStencilStateWriteEnabled;
		ComPtr<ID3D11DepthStencilState> m_DepthStencilStateWriteDisabled;
		ComPtr<ID3D11ShaderResourceView> m_DepthStencilSRV;
		ComPtr<ID3D11BlendState> m_BlendStateOpaque;
		ComPtr<ID3D11BlendState> m_BlendStateTransparent;
		ComPtr<ID3D11RasterizerState> m_RasterStateBackFaceCullOn;
		ComPtr<ID3D11RasterizerState> m_RasterStateBackFaceCullOff;
		ComPtr<ID3D11SamplerState> m_SamplerLinear;
		ComPtr<ID3D11SamplerState> m_ShadowMapSampler;
		ComPtr<ID3D11Query> m_PipelineStatsQuery;
		ComPtr<ID3D11Buffer> m_GlobalCBuffer;

		D3D11_VIEWPORT m_Viewport = {};

	private:
		void CreateGlobalConstantBuffer();
		void UpdateGlobalConstantBuffer(Camera* ActiveCamera, float appTime);

	public:
		ComPtr<ID3D11Device> GetDevice() const { return m_Device; }
		ComPtr<ID3D11DeviceContext> GetContext() const { return m_Context; }

		ComPtr<ID3D11RenderTargetView> GetBackBufferRTV() const { return m_BackBufferRTV; }
		ComPtr<ID3D11DepthStencilView> GetDSV() const { return m_DSV; }

	private:
		RendererSpec m_Spec;
		static Renderer* s_pInstance;

		GlobalCBuffer m_GlobalCBufferData;

		ComPtr<ID3D11InputLayout> m_ModelInputLayout;

		std::unique_ptr<ShaderProgram> m_OpaqueShaderProgram;
		std::unique_ptr<ShaderProgram> m_TransparentShaderProgram;
		std::unique_ptr<ShaderProgram> m_DSVShadowShaderProgram;
		std::unique_ptr<ShaderProgram> m_PointLightShadowShaderProgram;

		DirectX::XMFLOAT4 m_BaseClearColor;
		DirectX::XMFLOAT4 m_ClearColor;

		friend class Application;
	};
}