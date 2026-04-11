#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Core/Component/Transform.h"
#include "Core/Light/Light.h"

namespace Core {
	class ModelData;

	class Model
	{
	public:
		Model() {}
		Model(const std::string_view modelPath, const std::string_view texturesPath);
		Model(const std::string& modelPath, const std::string& texturesPath = "");
		Model(const Model& other) = delete;
		Model& operator=(const Model& other) = delete;
		Model(Model&& other) noexcept;
		Model& operator=(Model&& other) noexcept;
		~Model();

		Transform& GetTransform() { return m_Transform; }
		ModelData* GetModelData() const { return m_pModelData; }
		bool ShouldRender() const { return m_bShouldRender; }

		void SetShouldRender(bool bNewShouldRender) { m_bShouldRender = bNewShouldRender; }

	private:
		void Init(const std::string& modelPath, const std::string& texturesPath);
		void LoadLights();

	private:
		Transform m_Transform;
		ModelData* m_pModelData = nullptr;
		bool m_bShouldRender = true;

		std::vector<std::unique_ptr<Light>> m_Lights;
	};
}
