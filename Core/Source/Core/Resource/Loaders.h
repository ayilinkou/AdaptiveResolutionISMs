#pragma once

#include <utility>

#include "Vendor/stb/stb_image.h"
#include "DirectXTex.h"

#include "d3d11.h"
#include "wrl.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "Core/Model/ModelData.h"
#include "Core/Model/Node.h"
#include "Core/Renderer/Texture.h"
#include "Core/Renderer/TextureData.h"
#include "Core/Utility/MyMacros.h"
#include "Core/Utility/Utility.h"

namespace Core::Loaders {
	class TextureLoader
	{
	private:
		TextureLoader() {}

		friend class ResourceManager;

		static TextureData* Load(const char* filepath, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		{
			HRESULT hResult;
			ComPtr<ID3D11Texture2D> texture;
			bool bNeedsAlpha;
			unsigned char* imageData = nullptr;
			unsigned char* imageDataRgba = nullptr;

			// if .dds file, use DirectXTex, else use stb_image
			bool bUseDirectXTex = StaticUtils::GetFileExtension(filepath) == "dds";
			if (bUseDirectXTex)
			{
				std::wstring wideStr;
				StaticUtils::ToWideString(filepath, wideStr);
				const wchar_t* widePath = wideStr.c_str();
				DirectX::TexMetadata metadata;
				DirectX::ScratchImage image;
				ASSERT_NOT_FAILED(DirectX::LoadFromDDSFile(widePath, DirectX::DDS_FLAGS_NONE, &metadata, image));

				// check for invalid 1x1 BC texture (e.g. 1x1 BC5)
				bool bInvalidBC = DirectX::IsCompressed(metadata.format) && (metadata.width == 1 || metadata.height == 1);

				DirectX::ScratchImage finalImage;
				DirectX::TexMetadata finalMeta = {};

				// in case of invalid BC texture, have to make an uncompressed version instead
				// TODO: probably makes more sense to store these as a XMFLOAT3 instead
				if (bInvalidBC)
				{
					// decompress to a readable format
					DirectX::ScratchImage decompressed;
					ASSERT_NOT_FAILED(DirectX::Decompress(
						image.GetImages(),
						image.GetImageCount(),
						metadata,
						DXGI_FORMAT_R8G8B8A8_UNORM,
						decompressed)
					);

					// read the decoded pixel value
					const DirectX::Image* src = decompressed.GetImage(0, 0, 0);
					const uint8_t* srcPixel = src->pixels; // RGBA

					// create a new valid 4x4 texture
					ASSERT_NOT_FAILED(finalImage.Initialize2D(
						DXGI_FORMAT_R8G8B8A8_UNORM,
						1, 1,
						1, 1)
					);

					const DirectX::Image* dst = finalImage.GetImage(0, 0, 0);
					uint8_t* dstPixels = dst->pixels;

					// copy the pixel data
					dstPixels[0] = srcPixel[0];
					dstPixels[1] = srcPixel[1];
					dstPixels[2] = srcPixel[2];
					dstPixels[3] = srcPixel[3];

					finalMeta.width = 1;
					finalMeta.height = 1;
					finalMeta.depth = 1;
					finalMeta.arraySize = 1;
					finalMeta.mipLevels = 1;
					finalMeta.format = DXGI_FORMAT_R8G8B8A8_UNORM;
					finalMeta.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
				}
				else
				{
					// texture is already valid
					finalImage = std::move(image);
					finalMeta = metadata;
				}

				ComPtr<ID3D11Resource> resource;
				ASSERT_NOT_FAILED(DirectX::CreateTextureEx(
					pDevice,
					finalImage.GetImages(),
					finalImage.GetImageCount(),
					finalMeta,
					D3D11_USAGE_IMMUTABLE,
					D3D11_BIND_SHADER_RESOURCE,
					0u,
					0u,
					DirectX::CREATETEX_DEFAULT,
					&resource)
				);

				ASSERT_NOT_FAILED(resource.As(&texture));
			}
			else
			{
				int width, height, channels;
				std::string fileString(filepath);
				imageData = stbi_load(filepath, &width, &height, &channels, 0);
				assert(imageData);

				imageDataRgba = imageData;
				bNeedsAlpha = channels == 3; // there's no R8G8B8 format so have to use a format with an alpha

				D3D11_TEXTURE2D_DESC texDesc = {};
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.MipLevels = 0; // generate all mip levels
				texDesc.ArraySize = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.Usage = D3D11_USAGE_DEFAULT;
				texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; // RT bind needed to generate mips
				texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

				switch (channels)
				{
				case 1:
				{
					texDesc.Format = DXGI_FORMAT_R8_UNORM;
					break;
				}
				case 2:
				{
					texDesc.Format = DXGI_FORMAT_R8G8_UNORM;
					break;
				}
				case 3:
				{
					imageDataRgba = new unsigned char[width * height * 4];

					for (int i = 0; i < width * height; i++)
					{
						imageDataRgba[i * 4 + 0] = imageData[i * 3 + 0];
						imageDataRgba[i * 4 + 1] = imageData[i * 3 + 1];
						imageDataRgba[i * 4 + 2] = imageData[i * 3 + 2];
						imageDataRgba[i * 4 + 3] = 255;
					}

					channels = 4;
					texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				}
				default:
				{
					texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				}
				}

				ASSERT_NOT_FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &texture));
				pContext->UpdateSubresource(texture.Get(), 0u, nullptr, imageDataRgba, width* channels, 0u);

				if (bNeedsAlpha)
				{
					delete[] imageDataRgba;
				}
				stbi_image_free(imageData);
			}

			ID3D11ShaderResourceView* textureView = nullptr;
			ASSERT_NOT_FAILED(pDevice->CreateShaderResourceView(texture.Get(), nullptr, &textureView));

			NAME_D3D_RESOURCE(texture, (std::string(filepath) + " texture").c_str());
			NAME_D3D_RESOURCE(textureView, (std::string(filepath) + " texture SRV").c_str());

			if (!bUseDirectXTex)
				pContext->GenerateMips(textureView);

			return new TextureData(filepath, textureView);
		}
		
	};

	class ModelLoader
	{
	private:
		ModelLoader() {}

		friend class ResourceManager;

		static ModelData* Load(const char* modelPath, const char* texturesRoot, ID3D11Device* device)
		{
			Assimp::Importer Importer;
			const aiScene* scene = Importer.ReadFile(modelPath,
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_GenSmoothNormals |
				aiProcess_ConvertToLeftHanded
			);

			if (!scene)
				return nullptr;

			std::vector<Material> materials = LoadMaterials(scene, texturesRoot);
			
			Node* rootNode = new Node(nullptr, nullptr); // TODO: convert to unique_ptr and move into pModelData
			ModelData* pModelData = new ModelData(modelPath, texturesRoot, scene->mName.C_Str(), std::move(materials), rootNode, scene->mNumMeshes);
			rootNode->SetModelData(pModelData);
			rootNode->ProcessNode(scene->mRootNode, scene, DirectX::XMMatrixIdentity());
			pModelData->Init();
			return pModelData;
		}

		static std::vector<Material> LoadMaterials(const aiScene* scene, const std::string& texturesRoot)
		{
			std::vector<Material> materials;
			materials.reserve(scene->mNumMaterials);
			for (size_t i = 0; i < scene->mNumMaterials; i++)
			{
				materials.emplace_back(scene->mMaterials[i], texturesRoot);
			}
			return materials;
		}
	};
}