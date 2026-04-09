#include "ModelSystem.h"
#include "Model.h"

namespace Core {
	std::unordered_map<ModelData*, std::unordered_set<Model*>> ModelSystem::s_Models;

	void ModelSystem::RegisterModel(Model* model)
	{
		s_Models[model->GetModelData()].insert(model);
	}

	void ModelSystem::UnregisterModel(Model* model)
	{
		s_Models[model->GetModelData()].erase(model);
	}

	const std::unordered_set<Model*>& ModelSystem::GetModelsFromModelData(ModelData* pModelData)
	{
		return s_Models[pModelData];
	}
}