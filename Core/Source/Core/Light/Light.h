#pragma once

#include "d3d11.h"
#include "DirectXMath.h"
#include "wrl.h"

namespace Core {
	class Model;

	class Light
	{
	public:
		virtual ~Light() {}

		virtual void RenderControls() = 0;

		bool IsActive();

		void SetActive(bool bNewActive) { m_bActive = bNewActive; }
		void SetParentModel(Model* pParent) { m_pParent = pParent; }
		virtual void SetColor(float r, float g, float b) = 0;
		virtual void SetSpecularPower(float power) = 0;
		virtual void SetIntensity(float intensity) = 0;

	protected:
		Light() {}

	protected:
		bool m_bActive = true;
		Model* m_pParent = nullptr;
	};
}