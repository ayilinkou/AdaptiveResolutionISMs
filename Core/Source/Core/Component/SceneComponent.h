#pragma once

#include "Transform.h"

namespace Core {
	class SceneComponent
	{
	public:
		Transform& GetTransform() { return m_Transform; }

	protected:
		Transform m_Transform;

	};
}