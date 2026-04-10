#pragma once

#include <string>

namespace Core {
	struct ShaderProgramDesc
	{
	private:
		struct ShaderDesc
		{
			std::string Filepath = "";
			std::string Entry = "main";
		};

	public:
		ShaderDesc Vertex;
		ShaderDesc Hull;
		ShaderDesc Domain;
		ShaderDesc Geometry;
		ShaderDesc Pixel;
		ShaderDesc Compute;
	};
}