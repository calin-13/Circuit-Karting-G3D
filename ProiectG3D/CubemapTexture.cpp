#include "CubemapTexture.h"


static const GLenum types[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

CubemapTexture::CubemapTexture(const std::string& Dir, const std::string& PosXFilename, const std::string& NegXFilename, const std::string& PosYFilename, const std::string& NegYFilename, const std::string& PosZFilename, const std::string& NegZFilename)
{
	std::string::const_iterator it = Dir.end();
	it--;
	std::string BaseDir = *it == '/' ? Dir : Dir + "/";

	m_fileNames[0] = BaseDir + PosXFilename;
	m_fileNames[1] = BaseDir + NegXFilename;
	m_fileNames[2] = BaseDir + PosYFilename;
	m_fileNames[3] = BaseDir + NegYFilename;
	m_fileNames[4] = BaseDir + PosZFilename;
	m_fileNames[5] = BaseDir + NegZFilename;

	m_textureObj = 0;
}

CubemapTexture::~CubemapTexture()
{
	if (m_textureObj != 0)
	{
		glDeleteTextures(1, &m_textureObj);
	}
}

bool CubemapTexture::Load()
{
	glGenTextures(1, &m_textureObj);
	if (m_textureObj == 0) {
		std::cerr << "Failed to generate texture object for cubemap!" << std::endl;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	int width, height;
	void* pData = nullptr;
	int BPP;

	for (int i = 0; i < 6; i++) 
	{
		pData = nullptr;
		stbi_set_flip_vertically_on_load(false);
		unsigned char* pImageData = stbi_load(m_fileNames[i].c_str(), &width, &height, &BPP, 0);
		if (pImageData == nullptr) {
			std::cerr << "Failed to load texture: " << m_fileNames[i] << std::endl;
			return false;
		}

		pData = pImageData;

		GLenum format = (BPP == 3) ? GL_RGB : GL_RGBA;
		glTexImage2D(types[i], 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pData);

		stbi_image_free(pImageData);
	}

	
	return true;
}

void CubemapTexture::Bind(GLenum TextureUnit)
{
	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);
}
