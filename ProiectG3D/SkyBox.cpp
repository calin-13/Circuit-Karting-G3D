#include "SkyBox.h"

SkyBox::SkyBox(const std::string& Dir, const std::string& PosXFilename, const std::string& NegXFilename, const std::string& PosYFilename, const std::string& NegYFilename, const std::string& PosZFilename, const std::string& NegZFilename)
{
    cubemapTexture = new CubemapTexture(Dir, PosXFilename, NegXFilename, PosYFilename, NegYFilename, PosZFilename, NegZFilename);
    cubemapTexture->Load();

    float skyboxVertices[] = {
    -10.0f, -10.0f,  10.0f,
     10.0f, -10.0f,  10.0f,
     10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f, -10.0f,
    -10.0f,  10.0f,  10.0f,
     10.0f,  10.0f,  10.0f,
     10.0f,  10.0f, -10.0f,
    -10.0f,  10.0f, -10.0f,
    };


	unsigned int skyboxIndices[] = {
		//Right face
		1, 2, 6,
		6, 5, 1,
		//Left face
		0, 4, 7,
		7, 3, 0,
        // Inversare top face
        4, 7, 6,
        6, 5, 4,

        // Inversare bottom face
        0, 1, 2,
        2, 3, 0,
        //Back face
        0, 1, 5,
        5, 4, 0,
		//Front face
		3, 7, 6,
		6, 2, 3
	};

    glDisable(GL_CULL_FACE);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    this->shaderProgram = shaderProgram;
}

SkyBox::~SkyBox()
{
    delete cubemapTexture;
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void SkyBox::Render()
{
    glDepthMask(GL_FALSE);
    // Asociaza textura cubemap
    glActiveTexture(GL_TEXTURE0);
    cubemapTexture->Bind(GL_TEXTURE0);

    // Deseneaza cubul
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
}


