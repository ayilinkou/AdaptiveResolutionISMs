#pragma once

#include <iostream>

#include "d3d11.h"

#include "ShaderProgramDesc.h"

namespace Core {
	class ShaderProgramData
	{
	public:
		ShaderProgramData() = delete;
		ShaderProgramData(ID3D11VertexShader* vs, ID3D11HullShader* hs, ID3D11DomainShader* ds, ID3D11GeometryShader* gs, ID3D11PixelShader* ps,
			ID3D10Blob* vsBlob, const ShaderProgramDesc& desc) : m_VertexShader(vs), m_HullShader(hs), m_DomainShader(ds), m_GeometryShader(gs),
			m_PixelShader(ps), m_vsBlob(vsBlob), m_Desc(desc) { std::cout << "ShaderProgramData being created..." << std::endl; }

	private:
		friend class ShaderProgram;

		ID3D11VertexShader* m_VertexShader;
		ID3D11HullShader* m_HullShader;
		ID3D11DomainShader* m_DomainShader;
		ID3D11GeometryShader* m_GeometryShader;
		ID3D11PixelShader* m_PixelShader;

		ID3D10Blob* m_vsBlob;

		ShaderProgramDesc m_Desc;
	};
}