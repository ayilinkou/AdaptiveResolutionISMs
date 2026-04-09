#pragma once

#include <string>

#include "Core/Component/Transform.h"

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

		const Transform& GetTransform() const { return m_Transform; }
		ModelData* GetModelData() const { return m_pModelData; }
		bool ShouldRender() const { return m_bShouldRender; }

		void SetPosition(float x, float y, float z) { m_Transform.Position = { x, y, z }; }
		void SetShouldRender(bool bNewShouldRender) { m_bShouldRender = bNewShouldRender; }

	private:
		void Init(const std::string& modelPath, const std::string& texturesPath);

	private:
		Transform m_Transform;
		ModelData* m_pModelData = nullptr;
		bool m_bShouldRender = true;
	};
}
