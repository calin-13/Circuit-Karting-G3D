﻿#include "Model.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

Model::Model(string const& path, bool bSmoothNormals, bool gamma) : gammaCorrection(gamma)
{
    loadModel(path, bSmoothNormals);
	boundingSphereCenter = calculateBoundingSphereCenter();
	boundingSphereRadius = calculateBoundingSphereRadius();
}

void Model::Draw(Shader& shader)
{
    Shader copyShader = shader;
    for (unsigned int i = 0; i < meshes.size(); i++) {
        if (meshes[i].name == "obj5.008" || meshes[i].name == "obj5.005"|| meshes[i].name == "obj5.004") {
            shader.setMat4("model", nodeTransforms["obj5"]);
        }
        else {
            shader = copyShader;
        }

        meshes[i].Draw(shader);
    }
}

void Model::setNodeTransforms(const std::string& nodeName, glm::mat4 transform)
{
    nodeTransforms[nodeName] = transform;
}

void Model::setIsAnimated(bool animated)
{
    isAnimated = animated;
}

glm::vec3 Model::calculateBoundingSphereCenter() const {
    glm::vec3 overallCenter(0.0f);
    size_t totalVertexCount = 0;

    for (const Mesh& mesh : meshes) {
        glm::vec3 meshCenter = findMeshCenter(mesh);
        overallCenter += meshCenter * static_cast<float>(mesh.numVertices);
        totalVertexCount += mesh.numVertices;
    }

    return overallCenter / static_cast<float>(totalVertexCount);
}

float Model::calculateBoundingSphereRadius() const{
    glm::vec3 center = calculateBoundingSphereCenter();
    float maxRadius = 0.0f;

    for (const Mesh& mesh : meshes) {
        float radius = findMeshRadius(mesh, center);
        if (radius > maxRadius) {
            maxRadius = radius;
        }
    }

    return maxRadius;
}

void Model::loadModel(string const& path, bool bSmoothNormals)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | (bSmoothNormals ? aiProcess_GenSmoothNormals : aiProcess_GenNormals) | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }

    directory = path.substr(0, path.find_last_of("\\"));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(node->mName.C_Str(), mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }

}

Mesh Model::processMesh(std::string nodeName, aiMesh* mesh, const aiScene* scene)
{
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        else {
            int j = 0;
        }

        if (mesh->mTextureCoords[0]) 
        {
            glm::vec2 vec;

            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    
    return Mesh(nodeName, vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;  
                break;
            }
        }
        if (!skip)
        {   
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  
        }
    }
    return textures;
}

glm::vec3 Model::findMeshCenter(const Mesh& mesh) const
{
    glm::vec3 center(0.0f);
    for (unsigned int i = 0; i < mesh.numVertices; ++i) {
        center += mesh.vertices.get()[i].Position;
    }
    return center / static_cast<float>(mesh.numVertices);
}

float Model::findMeshRadius(const Mesh& mesh, glm::vec3 center) const
{
    float maxRadius = 0.0f;
    for (unsigned int i = 0; i < mesh.numVertices; ++i) {
        float distance = glm::distance(mesh.vertices.get()[i].Position, center);
        if (distance > maxRadius) {
            maxRadius = distance;
        }
    }
    return maxRadius;
}

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);

        // Op?ional: po?i crea o texturã albã implicitã.
        glBindTexture(GL_TEXTURE_2D, textureID);
        unsigned char whitePixel[3] = { 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
    }

    return textureID;
}
