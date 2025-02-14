#include "ShaderManager.h"

void ShaderManager::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
{
	m_shaders[name] = Shader(vertexPath.c_str(), fragmentPath.c_str());
}

Shader& ShaderManager::GetShader(const std::string& name)
{
	return m_shaders.at(name);
}
