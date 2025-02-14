#pragma once

#include <string>
#include <GL/glew.h>
#include <GLM.hpp>
#include "CubemapTexture.h"

class SkyBox
{
    GLuint VAO, VBO, EBO;
    CubemapTexture* cubemapTexture;
    GLuint shaderProgram;

public:

	SkyBox(const std::string& Dir,
		const std::string& PosXFilename,
		const std::string& NegXFilename,
		const std::string& PosYFilename,
		const std::string& NegYFilename,
		const std::string& PosZFilename,
		const std::string& NegZFilename);

	~SkyBox();

	void Render();
};

