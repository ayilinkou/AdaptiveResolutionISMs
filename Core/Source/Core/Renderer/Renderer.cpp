#include <vector>

#include "dxgi1_4.h"
#include "imgui_impl_dx11.h"

#include "Renderer.h"
#include "Core/Utility/MyMacros.h"
#include "Core/Camera/Camera.h"

namespace Core {
	Renderer* Renderer::s_pInstance = nullptr;

	using namespace Microsoft::WRL;

	Renderer::Renderer(const RendererSpec& spec)
	{
		s_pInstance = this;
		m_Spec = spec;
		m_GlobalCBufferData = {};
		m_BaseClearColor = { 0.3f, 0.6f, 0.8f, 1.f };
		m_ClearColor = m_BaseClearColor;
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

		ID3D11Texture2D* backBufferPtr;
		ASSERT_NOT_FAILED(m_SwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr));
		ASSERT_NOT_FAILED(m_Device->CreateRenderTargetView(backBufferPtr, nullptr, &m_BackBufferRTV));
		NAME_D3D_RESOURCE(backBufferPtr, "Back buffer texture");
		NAME_D3D_RESOURCE(m_BackBufferRTV, "Back buffer RTV");
		backBufferPtr->Release();
		backBufferPtr = nullptr;

		D3D11_TEXTURE2D_DESC depthTextureDesc = {};
		depthTextureDesc.Width = m_Spec.WinSpec.Width;
		depthTextureDesc.Height = m_Spec.WinSpec.Height;
		depthTextureDesc.MipLevels = 1u;
		depthTextureDesc.ArraySize = 1u;
		depthTextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		depthTextureDesc.SampleDesc.Count = 1u;
		depthTextureDesc.SampleDesc.Quality = 0u;
		depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		ASSERT_NOT_FAILED(m_Device->CreateTexture2D(&depthTextureDesc, nullptr, &m_DepthStencilTexture));
		NAME_D3D_RESOURCE(m_DepthStencilTexture, "Depth stencil texture");

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		ASSERT_NOT_FAILED(m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilStateWriteEnabled));
		NAME_D3D_RESOURCE(m_DepthStencilStateWriteEnabled, "Depth stencil state write enabled");

		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.StencilWriteMask = 0;

		ASSERT_NOT_FAILED(m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilStateWriteDisabled));
		NAME_D3D_RESOURCE(m_DepthStencilStateWriteDisabled, "Depth stencil state write disabled");
		
		m_Context->OMSetDepthStencilState(m_DepthStencilStateWriteEnabled.Get(), 1u);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		ASSERT_NOT_FAILED(m_Device->CreateDepthStencilView(m_DepthStencilTexture.Get(), &depthStencilViewDesc, &m_DSV));
		NAME_D3D_RESOURCE(m_DSV, "Depth stencil view");

		m_Context->OMSetRenderTargets(1, m_BackBufferRTV.GetAddressOf(), m_DSV.Get());

		D3D11_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc = {};
		depthStencilSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		depthStencilSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		depthStencilSRVDesc.Texture2D.MipLevels = 1;

		ASSERT_NOT_FAILED(m_Device->CreateShaderResourceView(m_DepthStencilTexture.Get(), &depthStencilSRVDesc, &m_DepthStencilSRV));
		NAME_D3D_RESOURCE(m_DepthStencilSRV, "Depth stencil SRV");

		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = false;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ASSERT_NOT_FAILED(m_Device->CreateBlendState(&blendDesc, &m_BlendStateOpaque));
		NAME_D3D_RESOURCE(m_BlendStateOpaque, "Blend state opaque");
		
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		ASSERT_NOT_FAILED(m_Device->CreateBlendState(&blendDesc, &m_BlendStateTransparent));
		NAME_D3D_RESOURCE(m_BlendStateTransparent, "Blend state transparent");

		m_Context->OMSetBlendState(m_BlendStateOpaque.Get(), nullptr, 0xFFFFFFFF);

		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.f;

		ASSERT_NOT_FAILED(m_Device->CreateRasterizerState(&rasterDesc, &m_RasterStateBackFaceCullOn));
		NAME_D3D_RESOURCE(m_RasterStateBackFaceCullOn, "Raster state back face cull on");

		rasterDesc.CullMode = D3D11_CULL_NONE;

		ASSERT_NOT_FAILED(m_Device->CreateRasterizerState(&rasterDesc, &m_RasterStateBackFaceCullOff));
		NAME_D3D_RESOURCE(m_RasterStateBackFaceCullOff, "Raster state back face cull off");

		m_Context->RSSetState(m_RasterStateBackFaceCullOn.Get());

		m_Viewport.Width = (float)m_Spec.WinSpec.Width;
		m_Viewport.Height = (float)m_Spec.WinSpec.Height;
		m_Viewport.MinDepth = 0.f;
		m_Viewport.MaxDepth = 1.f;
		m_Viewport.TopLeftX = 0.f;
		m_Viewport.TopLeftY = 0.f;

		m_Context->RSSetViewports(1u, &m_Viewport);

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ASSERT_NOT_FAILED(m_Device->CreateSamplerState(&samplerDesc, &m_SamplerLinear));
		NAME_D3D_RESOURCE(m_SamplerLinear, "Linear sampler state");

		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.BorderColor[0] = 1.f;
		samplerDesc.BorderColor[1] = 1.f;
		samplerDesc.BorderColor[2] = 1.f;
		samplerDesc.BorderColor[3] = 1.f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ASSERT_NOT_FAILED(m_Device->CreateSamplerState(&samplerDesc, &m_ShadowMapSampler));
		NAME_D3D_RESOURCE(m_ShadowMapSampler, "Shadow map sampler state");

		ID3D11SamplerState* samplers[2] = { m_SamplerLinear.Get(), m_ShadowMapSampler.Get() };
		m_Context->VSSetSamplers(0u, 2u, samplers);
		m_Context->HSSetSamplers(0u, 2u, samplers);
		m_Context->DSSetSamplers(0u, 2u, samplers);
		m_Context->GSSetSamplers(0u, 2u, samplers);
		m_Context->PSSetSamplers(0u, 2u, samplers);
		m_Context->CSSetSamplers(0u, 2u, samplers);

		D3D11_QUERY_DESC queryDesc = {};
		queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
		ASSERT_NOT_FAILED(m_Device->CreateQuery(&queryDesc, &m_PipelineStatsQuery));
		NAME_D3D_RESOURCE(m_PipelineStatsQuery, "Pipeline stats query");

		CreateGlobalConstantBuffer();
		m_Context->VSSetConstantBuffers(0u, 1u, m_GlobalCBuffer.GetAddressOf());
		m_Context->HSSetConstantBuffers(0u, 1u, m_GlobalCBuffer.GetAddressOf());
		m_Context->DSSetConstantBuffers(0u, 1u, m_GlobalCBuffer.GetAddressOf());
		m_Context->GSSetConstantBuffers(0u, 1u, m_GlobalCBuffer.GetAddressOf());
		m_Context->PSSetConstantBuffers(0u, 1u, m_GlobalCBuffer.GetAddressOf());
		m_Context->CSSetConstantBuffers(0u, 1u, m_GlobalCBuffer.GetAddressOf());

		ImGui_ImplDX11_Init(m_Device.Get(), m_Context.Get());
	}

	void Renderer::Shutdown()
	{
		ImGui_ImplDX11_Shutdown();
		
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

	void Renderer::BeginScene()
	{
		m_Context->ClearRenderTargetView(m_BackBufferRTV.Get(), reinterpret_cast<float*>(&m_ClearColor));
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

	void Renderer::EnableBlending()
	{
		m_Context->OMSetBlendState(m_BlendStateTransparent.Get(), nullptr, 0xFFFFFFFF);
	}

	void Renderer::DisableBlending()
	{
		m_Context->OMSetBlendState(m_BlendStateOpaque.Get(), nullptr, 0xFFFFFFFF);
	}

	void Renderer::EnableDepthWrite()
	{
		m_Context->OMSetDepthStencilState(m_DepthStencilStateWriteEnabled.Get(), 1u);
	}

	void Renderer::DisableDepthWrite()
	{
		m_Context->OMSetDepthStencilState(m_DepthStencilStateWriteDisabled.Get(), 1u);
	}

	void Renderer::BindForOpaqueDraws()
	{
		m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Context->IASetInputLayout(m_ModelInputLayout.Get());
		m_Context->VSSetShader(m_OpaqueShaderProgram->GetVertexShader(), nullptr, 0u);
		m_Context->PSSetShader(m_OpaqueShaderProgram->GetPixelShader(), nullptr, 0u);
		EnableDepthWrite();
		DisableBlending();
	}

	void Renderer::BindForTransparentDraws()
	{
		m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Context->IASetInputLayout(m_ModelInputLayout.Get());
		m_Context->VSSetShader(m_TransparentShaderProgram->GetVertexShader(), nullptr, 0u);
		m_Context->PSSetShader(m_TransparentShaderProgram->GetPixelShader(), nullptr, 0u);
		DisableDepthWrite();
		EnableBlending();
	}

	void Renderer::BindForDSVShadowPass()
	{
		m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Context->IASetInputLayout(m_ModelInputLayout.Get());
		m_Context->VSSetShader(m_DSVShadowShaderProgram->GetVertexShader(), nullptr, 0u);
		m_Context->PSSetShader(m_DSVShadowShaderProgram->GetPixelShader(), nullptr, 0u);
		EnableDepthWrite();
		DisableBlending();
	}

	void Renderer::BindForPointShadowPass()
	{
		m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Context->IASetInputLayout(m_ModelInputLayout.Get());
		m_Context->VSSetShader(m_PointLightShadowShaderProgram->GetVertexShader(), nullptr, 0u);
		m_Context->PSSetShader(m_PointLightShadowShaderProgram->GetPixelShader(), nullptr, 0u);
		EnableDepthWrite();
		DisableBlending();
	}

	void Renderer::SetBackFaceCulling(bool bEnabled)
	{
		m_Context->RSSetState(bEnabled ? m_RasterStateBackFaceCullOn.Get() : m_RasterStateBackFaceCullOff.Get());
	}

	void Renderer::SetBackBufferViewport()
	{
		m_Context->RSSetViewports(1u, &m_Viewport);
	}

	VramInfo Renderer::QueryVramUsage() const
	{
		VramInfo info = {};

		ComPtr<IDXGIDevice> dxgiDevice;
		m_Device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));

		ComPtr<IDXGIAdapter> adapter;
		dxgiDevice->GetAdapter(&adapter);

		ComPtr<IDXGIAdapter3> adapter3;
		adapter->QueryInterface(IID_PPV_ARGS(&adapter3));

		DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};
		adapter3->QueryVideoMemoryInfo(
			0,
			DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
			&memoryInfo
		);

		info.CurrentUsage = memoryInfo.CurrentUsage / 1024 / 1024;
		info.Budget = memoryInfo.Budget / 1024 / 1024;

		return info;
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

	void Renderer::CreateGlobalConstantBuffer()
	{
		HRESULT hResult;
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(GlobalCBuffer);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ASSERT_NOT_FAILED(m_Device->CreateBuffer(&desc, nullptr, &m_GlobalCBuffer));
		NAME_D3D_RESOURCE(m_GlobalCBuffer, "Global constant buffer");
	}

	void Renderer::UpdateGlobalConstantBuffer(Camera* activeCamera, float appTime)
	{
		m_GlobalCBufferData.CameraData.View = DirectX::XMMatrixTranspose(activeCamera->GetViewMatrix());
		m_GlobalCBufferData.CameraData.Proj = DirectX::XMMatrixTranspose(activeCamera->GetProjMatrix());
		m_GlobalCBufferData.CameraData.CameraPos = activeCamera->GetPosition();
		m_GlobalCBufferData.LightData = LightManager::GetLightBufferData(); // TODO: could give LightManager the address and have it store directly, rather than copy
		m_GlobalCBufferData.NearZ = activeCamera->GetNearZ();
		m_GlobalCBufferData.FarZ = activeCamera->GetFarZ();
		m_GlobalCBufferData.Time = appTime;
		m_GlobalCBufferData.ScreenWidth = m_Spec.WinSpec.Width;
		m_GlobalCBufferData.ScreenHeight = m_Spec.WinSpec.Height;

		HRESULT hResult;
		D3D11_MAPPED_SUBRESOURCE mappedSubresource = {};
		ASSERT_NOT_FAILED(m_Context->Map(m_GlobalCBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedSubresource));
		memcpy(mappedSubresource.pData, &m_GlobalCBufferData, sizeof(GlobalCBuffer));
		m_Context->Unmap(m_GlobalCBuffer.Get(), 0u);
	}

	void Renderer::CreateInputLayouts()
	{
		HRESULT hResult;
		D3D11_INPUT_ELEMENT_DESC vertexLayoutElements[3] = {};
		vertexLayoutElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexLayoutElements[0].SemanticName = "POSITION";
		vertexLayoutElements[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexLayoutElements[0].AlignedByteOffset = 0;

		vertexLayoutElements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexLayoutElements[1].SemanticName = "NORMAL";
		vertexLayoutElements[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexLayoutElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		vertexLayoutElements[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexLayoutElements[2].SemanticName = "TEXCOORD";
		vertexLayoutElements[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexLayoutElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		
		UINT numElements = _countof(vertexLayoutElements);

		ASSERT_NOT_FAILED(m_Device->CreateInputLayout(vertexLayoutElements, numElements, m_OpaqueShaderProgram->GetVertexShaderBlob()->GetBufferPointer(),
			m_OpaqueShaderProgram->GetVertexShaderBlob()->GetBufferSize(), &m_ModelInputLayout));
		NAME_D3D_RESOURCE(m_ModelInputLayout, "Model input layout");
	}

	void Renderer::CreateShaderPrograms()
	{
		ShaderProgramDesc desc;
		desc.Vertex.Filepath = "../Core/Source/Core/Shader/Shaders/BasicVS.hlsl";
		desc.Pixel.Filepath = "../Core/Source/Core/Shader/Shaders/BasicPS.hlsl";
		m_OpaqueShaderProgram = std::make_unique<ShaderProgram>(desc);

		desc.Vertex.Filepath = "../Core/Source/Core/Shader/Shaders/BasicVS.hlsl";
		desc.Pixel.Filepath = "../Core/Source/Core/Shader/Shaders/TransparentPS.hlsl";
		m_TransparentShaderProgram = std::make_unique<ShaderProgram>(desc);

		desc.Vertex.Filepath = "../Core/Source/Core/Shader/Shaders/ShadowVS.hlsl";
		desc.Pixel.Filepath = "";
		m_DSVShadowShaderProgram = std::make_unique<ShaderProgram>(desc);

		desc.Vertex.Filepath = "../Core/Source/Core/Shader/Shaders/ShadowVS.hlsl";
		desc.Pixel.Filepath = "../Core/Source/Core/Shader/Shaders/PointLightShadowPS.hlsl";
		m_PointLightShadowShaderProgram = std::make_unique<ShaderProgram>(desc);
	}

	void Core::Renderer::DestroyShaderPrograms()
	{
		m_OpaqueShaderProgram.reset();
		m_TransparentShaderProgram.reset();
		m_DSVShadowShaderProgram.reset();
		m_PointLightShadowShaderProgram.reset();
	}
}