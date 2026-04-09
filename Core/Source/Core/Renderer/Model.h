#pragma once

#include <string>

namespace Core {
	class ModelData;
	
	class Model
	{
	public:
		Model() {}
		Model(const std::string& modelPath, const std::string& texturesPath);
		Model(const Model& other) = delete;
		Model& operator=(const Model& other) = delete;
		Model(Model&& other) noexcept;
		Model& operator=(Model&& other) noexcept;
		~Model();

		ModelData* GetModelData() const { return m_pModelData; }

	private:
		void Init(const std::string& modelPath, const std::string& texturesPath);

	private:
		ModelData* m_pModelData = nullptr;
	};
}
