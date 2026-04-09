#pragma once

#include "Transform.h"

namespace Core {
	class SceneComponent
	{
	public:
		const Transform& GetTransform() const { return m_Transform; }

	protected:
		Transform m_Transform;

	};
}