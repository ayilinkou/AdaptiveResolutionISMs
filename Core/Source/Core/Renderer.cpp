#include <vector>

#include "Renderer.h"
#include "MyMacros.h"

namespace Core {
	Renderer* Renderer::s_pInstance = nullptr;

	using namespace Microsoft::WRL;

	Renderer::Renderer(const RendererSpec& spec)
	{
		s_pInstance = this;
		m_Spec = spec;
		m_ScreenAspect = 0.f;
	}

	void Renderer::Init()
	{
		HRESULT hResult;
		
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 1u;
		swapChainDesc.BufferDesc.Width = m_Spec.WinSpec.Width;
		swapChainDesc.BufferDesc.Height = m_Spec.WinSpec.Height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		BestAdapterAndOutput adapterAndOutput = GetBestAdapterAndOutput();
		if (m_Spec.WinSpec.bUseVSync)
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = adapterAndOutput.Mode.RefreshRate.Numerator;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = adapterAndOutput.Mode.RefreshRate.Denominator;
		}
		else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = m_Spec.hwnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = !m_Spec.WinSpec.bFullscreen;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // swap to DXGI_SWAP_FLIP_DISCARD when added a second buffer

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		ASSERT_NOT_FAILED(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			D3D11_CREATE_DEVICE_DEBUG,
			&featureLevel,
			1u,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&m_SwapChain,
			&m_Device,
			nullptr,
			&m_Context)
		);
	}

	void Renderer::Shutdown()
	{

	}

	void Renderer::BeginScene(float red, float green, float blue, float alpha)
	{
	}

	void Renderer::EndScene()
	{

	}

	Renderer::BestAdapterAndOutput Renderer::GetBestAdapterAndOutput() const
	{
		BestAdapterAndOutput out;
		IDXGIFactory* factory;
		HRESULT hResult;

		ASSERT_NOT_FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));

		// find best adapter, using VRAM as a guide
		SIZE_T maxVRAM = 0;
		for (UINT i = 0u; ; i++)
		{
			ComPtr<IDXGIAdapter> adapter;
			if (factory->EnumAdapters(i, &adapter) == DXGI_ERROR_NOT_FOUND)
				break;

			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			if (desc.DedicatedVideoMemory > maxVRAM)
			{
				maxVRAM = desc.DedicatedVideoMemory;
				out.Adapter = adapter;
			}
		}

		// find best output and mode, using refresh rate as a guide
		UINT bestHz = 0;
		for (UINT i = 0; ; i++)
		{
			ComPtr<IDXGIOutput> output;
			if (out.Adapter->EnumOutputs(i, &output) == DXGI_ERROR_NOT_FOUND)
				break;

			DXGI_OUTPUT_DESC outDesc;
			output->GetDesc(&outDesc);

			UINT numModes = 0;
			output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr);
			std::vector<DXGI_MODE_DESC> modes(numModes);
			output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modes.data());

			for (auto& mode : modes)
			{
				UINT hz = mode.RefreshRate.Numerator / mode.RefreshRate.Denominator;
				if (hz > bestHz)
				{
					bestHz = hz;
					out.Output = output;
					out.Mode = mode;
				}
			}
		}
		
		return out;
	}
}