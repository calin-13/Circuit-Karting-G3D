#pragma once

#include <iostream>
#include <string>
#include <GL/glew.h>
#include <stb_image.h>

class CubemapTexture
{
	std::string m_fileNames[6];
	GLuint m_textureObj;

public:
	CubemapTexture(const std::string& Dir,
		const std::string& PosXFilename,
		const std::string& NegXFilename,
		const std::string& PosYFilename,
		const std::string& NegYFilename,
		const std::string& PosZFilename,
		const std::string& NegZFilename);

	~CubemapTexture();

	bool Load();

	void Bind(GLenum TextureUnit);
};

