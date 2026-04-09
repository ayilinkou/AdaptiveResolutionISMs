#include <format>

#include "Model.h"
#include "ModelData.h"
#include "Core/Resource/ResourceManager.h"
#include "Core/Utility/Logger.h"
#include "ModelSystem.h"

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
	}
}
