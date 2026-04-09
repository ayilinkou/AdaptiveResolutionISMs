#include "ShaderProgram.h"
#include "Core/ResourceManager.h"

namespace Core {
	ShaderProgram::ShaderProgram(ShaderProgramDesc& desc)
		: m_ShaderProgramData(Core::ResourceManager::Get()->LoadShaderProgram(desc))
	{
	}

	ShaderProgram::~ShaderProgram()
	{
		Core::ResourceManager::Get()->UnloadShaderProgram(m_ShaderProgramData.m_Desc);
	}

	ID3D11VertexShader* ShaderProgram::GetVertexShader() const
	{
		return m_ShaderProgramData.m_VertexShader;
	}

	ID3D11HullShader* ShaderProgram::GetHullShader() const
	{
		return m_ShaderProgramData.m_HullShader;
	}

	ID3D11DomainShader* ShaderProgram::GetDomainShader() const
	{
		return m_ShaderProgramData.m_DomainShader;
	}

	ID3D11GeometryShader* ShaderProgram::GetGeometryShader() const
	{
		return m_ShaderProgramData.m_GeometryShader;
	}

	ID3D11PixelShader* ShaderProgram::GetPixelShader() const
	{
		return m_ShaderProgramData.m_PixelShader;
	}

	ID3D10Blob* ShaderProgram::GetVertexShaderBlob() const
	{
		return m_ShaderProgramData.m_vsBlob;
	}
}