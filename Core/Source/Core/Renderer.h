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

		static Renderer* Get() { return s_pInstance; }

		void Init();
		void Shutdown();

		void BeginScene(float red, float green, float blue, float alpha);
		void EndScene();

	private:
		struct BestAdapterAndOutput
		{
			ComPtr<IDXGIAdapter> Adapter;
			ComPtr<IDXGIOutput> Output;
			DXGI_MODE_DESC Mode = {};
		};

		BestAdapterAndOutput GetBestAdapterAndOutput() const;

	private:
		ComPtr<ID3D11Device> m_Device;
		ComPtr<ID3D11DeviceContext> m_Context;
		ComPtr<IDXGISwapChain> m_SwapChain;

	private:
		RendererSpec m_Spec;
		static Renderer* s_pInstance;

		float m_ScreenAspect;
	};
}