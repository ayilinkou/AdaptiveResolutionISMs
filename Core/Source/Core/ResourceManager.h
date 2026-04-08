#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <cassert>

#include "d3d11.h"
#include "d3dcompiler.h"

#include "Resource.h"
#include "Shader/ShaderResource.h"
#include "Shader/ShaderCreateInfo.h"
#include "Renderer.h"
#include "MyMacros.h"
#include "Logger.h"

namespace Core {
	using namespace Microsoft::WRL;
	
	class ResourceManager
	{
	public:
		ResourceManager(HWND hwnd, ComPtr<ID3D11Device> device)
			: m_hWnd(hwnd), m_Device(device) { s_Instance = this; }
		~ResourceManager();

		static ResourceManager* Get() { return s_Instance; }

	private:
		static ResourceManager* s_Instance;

	public:
		void Shutdown();

		// these must NOT be stored with a ComPtr and should be unloaded using UnloadTexture when no longer needed
		ID3D11ShaderResourceView* LoadTexture(const std::string& filepath);
		template <typename T>
		T* LoadShader(const std::string& filepath, const std::string& entry = "main");
		template <typename T>
		T* LoadShader(const std::string& filepath, const std::string& entry, ComPtr<ID3D10Blob>& bytecode);

		UINT UnloadTexture(const std::string& filepath);
		template <typename T>
		UINT UnloadShader(const std::string& filepath, const std::string& entry = "main");

		std::unordered_map<std::string, std::unique_ptr<Resource>>& GetTexturesMap() { return m_TexturesMap; }
		std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<ShaderResource>>>& GetShadersMap() { return m_ShadersMap; }

	private:
		ID3D11ShaderResourceView* Internal_LoadTexture(const char* filepath);
		template <typename T>
		T* Internal_LoadShader(const char* filepath, const char* entry, ComPtr<ID3D10Blob>& bytecode);

		void Internal_UnloadTexture(const std::string& filepath);
		template <typename T>
		void Internal_UnloadShader(const std::string& filepath, const std::string& entry);

	private:
		std::unordered_map<std::string, std::unique_ptr<Resource>> m_TexturesMap;
		std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<ShaderResource>>> m_ShadersMap;

		const HWND m_hWnd;
		ComPtr<ID3D11Device> m_Device;
	};

	template<typename T>
	inline T* ResourceManager::LoadShader(const std::string& filepath, const std::string& entry)
	{
		auto it = m_ShadersMap.find(filepath);
		if (it != m_ShadersMap.end())
		{
			auto iter = it->second.find(entry);
			if (iter != it->second.end())
			{
				iter->second->ShaderRes->AddRef();
				return static_cast<T*>(iter->second->ShaderRes->m_pData);
			}
		}

		m_ShadersMap[filepath][entry] = std::make_unique<ShaderResource>();
		T* pData = Internal_LoadShader<T>(filepath.c_str(), entry.c_str(), m_ShadersMap[filepath][entry]->Bytecode);
		if (!pData)
		{
			return nullptr;
		}

		m_ShadersMap[filepath][entry]->ShaderRes = std::make_unique<Resource>(pData);

		return pData;
	}

	template<typename T>
	inline T* ResourceManager::LoadShader(const std::string& filepath, const std::string& entry, ComPtr<ID3D10Blob>& bytecode)
	{
		T* ptr = LoadShader<T>(filepath, entry);
		assert(ptr);
		bytecode = m_ShadersMap[filepath][entry]->Bytecode;
		return ptr;
	}

	template<typename T>
	inline UINT ResourceManager::UnloadShader(const std::string& filepath, const std::string& entry)
	{
		auto& pShader = m_ShadersMap[filepath][entry];
		if (!pShader.get() || !pShader->ShaderRes.get())
		{
			__debugbreak(); // attempting to unload a shader which isn't loaded
		}

		Resource* pResourceToUnload = pShader->ShaderRes.get();
		if (!pResourceToUnload)
		{
			__debugbreak(); // attempting to unload a shader which isn't loaded
			m_ShadersMap[filepath].erase(entry);
			if (m_ShadersMap[filepath].empty())
			{
				m_ShadersMap.erase(filepath);
			}
			return 0u;
		}

		pResourceToUnload->RemoveRef();
		if (pResourceToUnload->m_RefCount > 0)
		{
			return pResourceToUnload->m_RefCount;
		}

		Internal_UnloadShader<T>(filepath, entry);
		return 0u;
	}

	template<typename T>
	inline T* ResourceManager::Internal_LoadShader(const char* filepath, const char* entry, ComPtr<ID3D10Blob>& bytecode)
	{
		HRESULT hResult;
		ComPtr<ID3D10Blob> errorMessage;

		std::wstring wideString;
		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, filepath, -1, nullptr, 0);
		wideString.resize(sizeNeeded - 1);
		MultiByteToWideChar(CP_UTF8, 0, filepath, -1, &wideString[0], sizeNeeded);

		const WCHAR* wideFilepath = wideString.c_str();

		UINT compileFlags = D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		hResult = D3DCompileFromFile(wideFilepath, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, ShaderCreateInfo<T>::Target, compileFlags, 0, &bytecode, &errorMessage);
		if (FAILED(hResult))
		{
			if (errorMessage.Get())
			{
				Logger::OutputShaderErrorMessage(errorMessage.Get(), m_hWnd, filepath);
			}
			else
			{
				MessageBox(m_hWnd, filepath, "Missing shader file!", MB_OK);
			}

			return nullptr;
		}

		T* shaderPtr;
		std::string path(filepath);
		ASSERT_NOT_FAILED(ShaderCreateInfo<T>::Create(Renderer::Get()->GetDevice().Get(), bytecode.Get(), &shaderPtr));
		NAME_D3D_RESOURCE(shaderPtr, (path + " " + entry + ShaderCreateInfo<T>::Suffix).c_str());

		return shaderPtr;
	}

	template<typename ShaderType>
	inline void ResourceManager::Internal_UnloadShader(const std::string& filepath, const std::string& entry)
	{
		ShaderType* shader = static_cast<ShaderType*>(m_ShadersMap[filepath][entry]->ShaderRes->m_pData);
		shader->Release();
		m_ShadersMap[filepath].erase(entry);

		if (m_ShadersMap[filepath].empty())
		{
			m_ShadersMap.erase(filepath);
		}
	}
}
