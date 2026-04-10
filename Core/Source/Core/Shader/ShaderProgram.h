#pragma once

#include "wrl.h"

#include "ShaderProgramData.h"
#include "ShaderProgramDesc.h"

struct ID3D10Blob;
struct ID3D11VertexShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11GeometryShader;
struct ID3D11PixelShader;
struct ID3D11ComputeShader;

namespace Core {	
	class ShaderProgram
	{
	public:
		ShaderProgram() = delete;
		ShaderProgram(ShaderProgramDesc& desc);
		~ShaderProgram();

		void Bind();

	private:
		const ShaderProgramData m_ShaderProgramData;

	public:
		ID3D11VertexShader* GetVertexShader() const;
		ID3D11HullShader* GetHullShader() const;
		ID3D11DomainShader* GetDomainShader() const;
		ID3D11GeometryShader* GetGeometryShader() const;
		ID3D11PixelShader* GetPixelShader() const;
		ID3D11ComputeShader* GetComputeShader() const;

		ID3D10Blob* GetVertexShaderBlob() const;
	};
}