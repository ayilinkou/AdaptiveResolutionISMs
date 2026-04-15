#pragma once

#include <string>

#include "d3d11.h"
#include "wrl.h"

namespace Core {	
	
	class TextureData
	{
		friend class Texture;

	public:
		TextureData(const std::string& filepath, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV)
			: m_Filepath(filepath), m_SRV(SRV) {}

	private:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
		const std::string m_Filepath;
	};
}
