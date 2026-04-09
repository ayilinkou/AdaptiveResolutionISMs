#pragma once

#include <string>
	
struct ID3D11ShaderResourceView;

namespace Core {	
	class TextureData;
	
	class Texture
	{
	public:
		Texture() = delete;
		Texture(const std::string& filepath);
		Texture(const Texture& other);
		Texture& operator=(const Texture& other);
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;
		~Texture();

		ID3D11ShaderResourceView* GetSRV();

	private:
		void Init(const std::string& filepath);

	private:
		TextureData* m_pTextureData = nullptr;
	};
}
