#include <format>
#include <algorithm>
#include <cmath>

#include "Model.h"
#include "ModelData.h"
#include "Core/Resource/ResourceManager.h"
#include "Core/Utility/Logger.h"
#include "ModelSystem.h"
#include "Core/Light/PointLight.h"
#include "Core/Light/DirectionalLight.h"

float ACESFilm(float x)
{
	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;

	return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float GammaCorrect(float x)
{
	return powf(x, 1.f / 2.2f);
}

float To255(float x)
{
	x = std::clamp(x, 0.f, 1.f);
	return x * 255.f;
}

DirectX::XMFLOAT3 ToneMapACES(const DirectX::XMFLOAT3& hdr, float exposure = 1.f)
{
	DirectX::XMFLOAT3 result;

	result.x = GammaCorrect(ACESFilm(hdr.x * exposure));
	result.y = GammaCorrect(ACESFilm(hdr.y * exposure));
	result.z = GammaCorrect(ACESFilm(hdr.z * exposure));

	return result;
}

namespace Core {
	Model::Model(const std::string& modelPath, const std::string& texturesPath)
	{
		Init(modelPath, texturesPath);
		ModelSystem::RegisterModel(this);
	}

	Model::Model(Model&& other) noexcept
		: m_pModelData(other.m_pModelData)
	{
		std::cout << "Model's move constructor..." << std::endl;
		other.m_pModelData = nullptr;
	}

	Model& Model::operator=(Model&& other) noexcept
	{
		std::cout << "Model's move assignment..." << std::endl;
		if (this != &other)
		{
			m_pModelData = other.m_pModelData;
			other.m_pModelData = nullptr;
		}
		return *this;
	}

	Model::~Model()
	{
		std::cout << "Model's destructor..." << std::endl;
		if (m_pModelData)
			Core::ResourceManager::Get()->UnloadModel(m_pModelData->m_ModelPath);
		ModelSystem::UnregisterModel(this);
	}

	void Model::Init(const std::string& modelPath, const std::string& texturesPath)
	{
		m_pModelData = Core::ResourceManager::Get()->LoadModel(modelPath, texturesPath);

		if (!m_pModelData)
		{
			std::string message = std::format("Failed to load model {}", modelPath);
			Logger::ShowMessageBox(message.c_str(), "Failed to load model!", MB_OK | MB_ICONERROR);
			std::abort(); // maybe this is overkill but it keeps the program safe
		}

		LoadLights();
	}

	void Model::LoadLights()
	{
		for (const auto& [light, transform] : m_pModelData->m_Lights)
		{
			DirectX::XMFLOAT3 color = { light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b };

			// TODO: Assimp's light color values expect a HDR workflow and have light colors higher than 255.
			// For the meantime, will tone map here rather than as a post process.
			color = ToneMapACES(color);
			
			switch (light.mType)
			{
			case aiLightSource_POINT:
			{
				DirectX::XMFLOAT3 attenuation = { light.mAttenuationQuadratic, light.mAttenuationLinear, light.mAttenuationConstant };

				std::unique_ptr<PointLight> pPointLight = std::make_unique<PointLight>(color, attenuation);
				pPointLight->SetParentModel(this);
				pPointLight->SetAccumTransform(transform);
				pPointLight->SetPosition(light.mPosition.x, light.mPosition.y, light.mPosition.z);
				pPointLight->SetIntensity(5.f); // TEMP

				m_Lights.push_back(std::move(pPointLight));
				break;
			}
			case aiLightSource_DIRECTIONAL:
			{
				DirectX::XMFLOAT3 dir = { light.mDirection.x, light.mDirection.y, light.mDirection.z };

				std::unique_ptr<DirectionalLight> pDirLight = std::make_unique<DirectionalLight>(color, dir);
				pDirLight->SetParentModel(this);

				m_Lights.push_back(std::move(pDirLight));
				break;
			}
			default:
				break;
			}
		}
	}
}
