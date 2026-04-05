#pragma once

#include "d3d11.h"

#include "wrl.h"

#include "Window.h"

namespace Core {
	using namespace Microsoft::WRL;
	
	struct RendererSpec
	{
		float NearPlane;
		float FarPlane;
		HWND hwnd;
		WindowSpec WinSpec;
	};
	
	class Renderer
	{
	public:
		Renderer(const RendererSpec& spec);
		~Renderer();

		static Renderer* Get() { return s_pInstance; }

		void Init();
		void Shutdown();

		void BeginScene(float red, float green, float blue, float alpha);
		void EndScene();

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
		ComPtr<ID3D11ShaderResourceView> m_DepthStencilSRV;
		ComPtr<ID3D11BlendState> m_BlendStateOpaque;
		ComPtr<ID3D11BlendState> m_BlendStateTransparent;
		ComPtr<ID3D11RasterizerState> m_RasterStateBackFaceCullOn;
		ComPtr<ID3D11RasterizerState> m_RasterStateBackFaceCullOff;
		ComPtr<ID3D11SamplerState> m_SamplerLinear;
		ComPtr<ID3D11Query> m_PipelineStatsQuery;

		D3D11_VIEWPORT m_Viewport = {};

	private:
		RendererSpec m_Spec;
		static Renderer* s_pInstance;

		float m_ScreenAspect;
	};
}