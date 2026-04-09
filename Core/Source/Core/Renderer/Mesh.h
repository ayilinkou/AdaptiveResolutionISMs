#pragma once

#include <string>

struct aiMesh;

typedef unsigned int UINT;

namespace Core {
	class ModelData;
	class Material;
	
	class Mesh
	{
	public:
		void Init(ModelData* pModelData, aiMesh* mesh);

		bool IsValid() const { return m_bIsValid; }

		UINT GetIndexCount() const { return m_IndexCount; }
		UINT GetIndexOffset() const { return m_IndexOffset; }

		Material* GetMaterial() const { return m_Material; }

	private:
		ModelData* m_pModelData = nullptr;
		std::string m_Name;
		bool m_bIsValid = false;

		UINT m_IndexCount = 0u;
		UINT m_IndexOffset = 0u;
		Material* m_Material;
	};
}
