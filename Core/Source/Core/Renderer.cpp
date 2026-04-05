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

	Renderer::~Renderer()
	{
		Shutdown();
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

		AdapterAndOutput adapterAndOutput = GetBestAdapterAndOutput();
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

		ID3D11Texture2D* BackBufferPtr;
		ASSERT_NOT_FAILED(m_SwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBufferPtr));
		ASSERT_NOT_FAILED(m_Device->CreateRenderTargetView(BackBufferPtr, nullptr, &m_BackBufferRTV));
		NAME_D3D_RESOURCE(BackBufferPtr, "Back buffer texture");
		NAME_D3D_RESOURCE(m_BackBufferRTV, "Back buffer RTV");
		BackBufferPtr->Release();
		BackBufferPtr = nullptr;

		D3D11_TEXTURE2D_DESC DepthTextureDesc = {};
		DepthTextureDesc.Width = m_Spec.WinSpec.Width;
		DepthTextureDesc.Height = m_Spec.WinSpec.Height;
		DepthTextureDesc.MipLevels = 1u;
		DepthTextureDesc.ArraySize = 1u;
		DepthTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		DepthTextureDesc.SampleDesc.Count = 1u;
		DepthTextureDesc.SampleDesc.Quality = 0u;
		DepthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		DepthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		ASSERT_NOT_FAILED(m_Device->CreateTexture2D(&DepthTextureDesc, nullptr, &m_DepthStencilTexture));
		NAME_D3D_RESOURCE(m_DepthStencilTexture, "Depth stencil texture");

		D3D11_DEPTH_STENCIL_DESC DepthStencilDesc = {};
		DepthStencilDesc.DepthEnable = true;
		DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		DepthStencilDesc.StencilEnable = true;
		DepthStencilDesc.StencilReadMask = 0xFF;
		DepthStencilDesc.StencilWriteMask = 0xFF;

		DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		ASSERT_NOT_FAILED(m_Device->CreateDepthStencilState(&DepthStencilDesc, &m_DepthStencilStateWriteEnabled));
		NAME_D3D_RESOURCE(m_DepthStencilStateWriteEnabled, "Depth stencil state write enabled");
		
		m_Context->OMSetDepthStencilState(m_DepthStencilStateWriteEnabled.Get(), 1u);

		D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
		DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		ASSERT_NOT_FAILED(m_Device->CreateDepthStencilView(m_DepthStencilTexture.Get(), &DepthStencilViewDesc, &m_DSV));
		NAME_D3D_RESOURCE(m_DSV, "Depth stencil view");

		m_Context->OMSetRenderTargets(1, m_BackBufferRTV.GetAddressOf(), m_DSV.Get());

		D3D11_SHADER_RESOURCE_VIEW_DESC DepthStencilSRVDesc = {};
		DepthStencilSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		DepthStencilSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		DepthStencilSRVDesc.Texture2D.MipLevels = 1;

		ASSERT_NOT_FAILED(m_Device->CreateShaderResourceView(m_DepthStencilTexture.Get(), &DepthStencilSRVDesc, &m_DepthStencilSRV));
		NAME_D3D_RESOURCE(m_DepthStencilSRV, "Depth stencil SRV");

		D3D11_BLEND_DESC BlendDesc = {};
		BlendDesc.AlphaToCoverageEnable = false;
		BlendDesc.IndependentBlendEnable = false;
		BlendDesc.RenderTarget[0].BlendEnable = false;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ASSERT_NOT_FAILED(m_Device->CreateBlendState(&BlendDesc, &m_BlendStateOpaque));
		NAME_D3D_RESOURCE(m_BlendStateOpaque, "Blend state opaque");
		
		BlendDesc.RenderTarget[0].BlendEnable = true;
		BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		ASSERT_NOT_FAILED(m_Device->CreateBlendState(&BlendDesc, &m_BlendStateTransparent));
		NAME_D3D_RESOURCE(m_BlendStateTransparent, "Blend state transparent");

		m_Context->OMSetBlendState(m_BlendStateOpaque.Get(), nullptr, 0xFFFFFFFF);

		D3D11_RASTERIZER_DESC RasterDesc = {};
		RasterDesc.AntialiasedLineEnable = false;
		RasterDesc.CullMode = D3D11_CULL_BACK;
		RasterDesc.DepthBias = 0;
		RasterDesc.DepthBiasClamp = 0.f;
		RasterDesc.DepthClipEnable = true;
		RasterDesc.FillMode = D3D11_FILL_SOLID;
		RasterDesc.FrontCounterClockwise = false;
		RasterDesc.MultisampleEnable = false;
		RasterDesc.ScissorEnable = false;
		RasterDesc.SlopeScaledDepthBias = 0.f;

		ASSERT_NOT_FAILED(m_Device->CreateRasterizerState(&RasterDesc, &m_RasterStateBackFaceCullOn));
		NAME_D3D_RESOURCE(m_RasterStateBackFaceCullOn, "Raster state back face cull on");

		RasterDesc.CullMode = D3D11_CULL_NONE;

		ASSERT_NOT_FAILED(m_Device->CreateRasterizerState(&RasterDesc, &m_RasterStateBackFaceCullOff));
		NAME_D3D_RESOURCE(m_RasterStateBackFaceCullOff, "Raster state back face cull off");

		m_Context->RSSetState(m_RasterStateBackFaceCullOn.Get());

		m_Viewport.Width = (float)m_Spec.WinSpec.Width;
		m_Viewport.Height = (float)m_Spec.WinSpec.Height;
		m_Viewport.MinDepth = 0.f;
		m_Viewport.MaxDepth = 1.f;
		m_Viewport.TopLeftX = 0.f;
		m_Viewport.TopLeftY = 0.f;

		m_Context->RSSetViewports(1u, &m_Viewport);

		D3D11_SAMPLER_DESC SamplerDesc = {};
		SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		SamplerDesc.MinLOD = 0;
		SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ASSERT_NOT_FAILED(m_Device->CreateSamplerState(&SamplerDesc, &m_SamplerLinear));
		NAME_D3D_RESOURCE(m_SamplerLinear, "Linear sampler state");

		m_Context->VSSetSamplers(0u, 1u, m_SamplerLinear.GetAddressOf());
		m_Context->HSSetSamplers(0u, 1u, m_SamplerLinear.GetAddressOf());
		m_Context->DSSetSamplers(0u, 1u, m_SamplerLinear.GetAddressOf());
		m_Context->GSSetSamplers(0u, 1u, m_SamplerLinear.GetAddressOf());
		m_Context->PSSetSamplers(0u, 1u, m_SamplerLinear.GetAddressOf());
		m_Context->CSSetSamplers(0u, 1u, m_SamplerLinear.GetAddressOf());

		D3D11_QUERY_DESC QueryDesc = {};
		QueryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
		ASSERT_NOT_FAILED(m_Device->CreateQuery(&QueryDesc, &m_PipelineStatsQuery));
		NAME_D3D_RESOURCE(m_PipelineStatsQuery, "Pipeline stats query");
	}

	void Renderer::Shutdown()
	{
		m_SwapChain->SetFullscreenState(false, NULL);

		m_Context->ClearState();
		m_Context->Flush();

		m_Context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		m_Context->RSSetState(nullptr);

		m_DepthStencilTexture.Reset();
		m_DepthStencilStateWriteEnabled.Reset();
		m_DSV.Reset();
		m_DepthStencilSRV.Reset();
		m_BlendStateOpaque.Reset();
		m_BlendStateTransparent.Reset();
		m_RasterStateBackFaceCullOn.Reset();
		m_RasterStateBackFaceCullOff.Reset();
		m_SamplerLinear.Reset();
		m_BackBufferRTV.Reset();
		m_PipelineStatsQuery.Reset();

		m_SwapChain.Reset();

		m_Context->ClearState();
		m_Context->Flush();
		m_Context.Reset();

		Microsoft::WRL::ComPtr<ID3D11Debug> d3dDebug;
		if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&d3dDebug))))
		{
			d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		}

		m_Device.Reset();
	}

	void Renderer::BeginScene(float red, float green, float blue, float alpha)
	{
		float Color[4];
		Color[0] = red;
		Color[1] = green;
		Color[2] = blue;
		Color[3] = alpha;

		m_Context->ClearRenderTargetView(m_BackBufferRTV.Get(), Color);
		m_Context->ClearDepthStencilView(m_DSV.Get(), D3D11_CLEAR_DEPTH, 1.f, 0u);
	}

	void Renderer::EndScene()
	{
		if (m_Spec.WinSpec.bUseVSync)
		{
			m_SwapChain->Present(1u, 0u);
		}
		else
		{
			m_SwapChain->Present(0u, 0u);
		}
	}

	Renderer::AdapterAndOutput Renderer::GetBestAdapterAndOutput() const
	{
		AdapterAndOutput out;
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