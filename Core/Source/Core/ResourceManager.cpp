#include "ResourceManager.h"

namespace Core {
	ResourceManager* ResourceManager::s_Instance = nullptr;

	ResourceManager::~ResourceManager()
	{
		Shutdown();
	}

	void ResourceManager::Shutdown()
	{
		if (!m_ShadersMap.empty())
		{
			__debugbreak(); // Attempting to shutdown when resources are still loaded!
		}

		m_ShadersMap.clear();
		s_Instance = nullptr;
	}
}
