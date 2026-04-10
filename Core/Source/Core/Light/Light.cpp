#include "Light.h"
#include "Core/Model/Model.h"

namespace Core {
	bool Light::IsActive()
	{
		if (m_pParent && !m_pParent->ShouldRender())
			return false;
		return m_bActive;
	}
}