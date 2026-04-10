#include "ShaderProgram.h"
#include "Core/Resource/ResourceManager.h"
#include "Core/Renderer/Renderer.h"

namespace Core {
	ShaderProgram::ShaderProgram(ShaderProgramDesc& desc)
		: m_ShaderProgramData(Core::ResourceManager::Get()->LoadShaderProgram(desc))
	{
	}

	ShaderProgram::~ShaderProgram()
	{
		Core::ResourceManager::Get()->UnloadShaderProgram(m_ShaderProgramData.m_Desc);
	}

	void ShaderProgram::Bind()
	{
		ID3D11DeviceContext* pContext = Core::Renderer::Get()->GetContext().Get();
		pContext->VSSetShader(m_ShaderProgramData.m_VertexShader, nullptr, 0u);
		pContext->GSSetShader(m_ShaderProgramData.m_GeometryShader, nullptr, 0u);
		pContext->HSSetShader(m_ShaderProgramData.m_HullShader, nullptr, 0u);
		pContext->DSSetShader(m_ShaderProgramData.m_DomainShader, nullptr, 0u);
		pContext->PSSetShader(m_ShaderProgramData.m_PixelShader, nullptr, 0u);
		pContext->CSSetShader(m_ShaderProgramData.m_ComputeShader, nullptr, 0u);
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

	ID3D11ComputeShader* ShaderProgram::GetComputeShader() const
	{
		return m_ShaderProgramData.m_ComputeShader;
	}

	ID3D10Blob* ShaderProgram::GetVertexShaderBlob() const
	{
		return m_ShaderProgramData.m_vsBlob;
	}
}