#pragma once
#include "unordered_map"
#include "Shader.h"

class ShaderManager
{
private:
	std::unordered_map<std::string, Shader> m_shaders;
public:
    void LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

    Shader& GetShader(const std::string& name);
};

