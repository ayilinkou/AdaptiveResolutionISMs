#pragma once

#include <unordered_map>
#include <unordered_set>

namespace Core {
	class Model;
	class ModelData;
	
	class ModelSystem
	{
	public:
		static void RegisterModel(Model* model);
		static void UnregisterModel(Model* model);

		static const std::unordered_map<ModelData*, std::unordered_set<Model*>>& GetAllModels() { return s_Models; }
		static const std::unordered_set<Model*>& GetModelsFromModelData(ModelData* pModelData);

	private:
		static std::unordered_map<ModelData*, std::unordered_set<Model*>> s_Models;
	};
}