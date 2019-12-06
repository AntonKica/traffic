#include "Model.h"
#include <iostream>
#include <filesystem>

constexpr const char* ERROR_PREFIX = "ASSIMP::LOADER::ERROR";

Model::Model(FilePath file)
{
	loadModel(file);
}

void Model::loadModel(FilePath file)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate);

	if (!scene)
	{
		std::cerr << "Failed to load model!\n" << ERROR_PREFIX << importer.GetErrorString();
		throw std::runtime_error("Unknfown texture path: " + file);
	}

	directory = file.substr(0, file.find_last_of('/'));

	processNode(scene, scene->mRootNode);
}

void Model::processNode(const aiScene* scene, aiNode* node)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		meshes.push_back(processMesh(scene, mesh));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(scene, node->mChildren[i]);
	}
}

Mesh Model::processMesh(const aiScene* scene, aiMesh* mesh)
{
	const uint64_t verticesCount = mesh->mNumVertices;
	const bool hasColor = mesh->HasVertexColors(0);
	const bool hasNormals = mesh->HasNormals();
	const bool hasTextureCoords = mesh->HasTextureCoords(0);

	Mesh newMesh;
	newMesh.vertices.resize(verticesCount);

	if (hasColor)			newMesh.colorVertices.resize(verticesCount);
	if (hasNormals)			newMesh.normalVertices.resize(verticesCount);
	if (hasTextureCoords)	 newMesh.textureVertices.resize(verticesCount);

	for (size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		// vertex
		{
			glm::vec3 vertex;
			vertex.x = mesh->mVertices[i].x;
			vertex.y = mesh->mVertices[i].y;
			vertex.z = mesh->mVertices[i].z;

			newMesh.vertices[i] = vertex;
		}
		// colors
		if (hasColor)
		{
			// ignore alpha value
			glm::vec4 color;
			color.r = mesh->mColors[0][i].r;
			color.g = mesh->mColors[0][i].g;
			color.b = mesh->mColors[0][i].b;
			color.a = mesh->mColors[0][i].a;

			newMesh.normalVertices[i] = color;
		}

		// normals
		if(hasNormals)
		{
			glm::vec3 normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;

			newMesh.normalVertices[i] = normal;
		}
		// textures
		if (hasTextureCoords) // does the mesh contain texture coordinates?
		{
			glm::vec2 textureCoords;
			textureCoords.x = mesh->mTextureCoords[0][i].x;
			textureCoords.y = mesh->mTextureCoords[0][i].y;
			newMesh.textureVertices[i] = textureCoords;
		}
	}

	// indices
	if (mesh->mNumFaces)
	{
		newMesh.indices.resize(mesh->mNumFaces * 3);
		auto indicesIter = newMesh.indices.begin();
		for (size_t i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];

			indicesIter = std::copy(face.mIndices, face.mIndices + face.mNumIndices, indicesIter);
		}
	}

	// texture
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::optional<FilePath> path;
		TextureType currentTexture = TextureType::DIFFUSE;
		{
			path = getTexturePath(material, aiTextureType_DIFFUSE);
			if (path)
			{
				std::string texturePath = directory.empty() ?
					path.value() : directory + '/' + path.value();

				newMesh.textures[currentTexture] = texturePath;
			}
		}

		currentTexture = TextureType::SPECULAR;
		{
			path = getTexturePath(material, aiTextureType_SPECULAR);
			if (path)
			{
				std::string texturePath = directory.empty() ?
					path.value() : directory + '/' + path.value();

				newMesh.textures[currentTexture] = texturePath;
			}
		}

		currentTexture = TextureType::AMBIENT;
		{
			path = getTexturePath(material, aiTextureType_AMBIENT);
			if (path)
			{
				std::string texturePath = directory.empty() ?
					path.value() : directory + '/' + path.value();

				newMesh.textures[currentTexture] = texturePath;
			}
		}

		currentTexture = TextureType::UNKNOWN;
		{
			path = getTexturePath(material, aiTextureType_UNKNOWN);
			if (path)
			{
				std::string texturePath = directory.empty() ?
					path.value() : directory + '/' + path.value();

				newMesh.textures[currentTexture] = texturePath;
			}
		}
	}

	return newMesh;
}

std::optional<FilePath> Model::getTexturePath(aiMaterial* mat, aiTextureType type)
{
	std::optional<FilePath> texturePath;

	// get first texture
	int textCount = mat->GetTextureCount(type);
	if (textCount > 0)
	{
		aiString str;
		mat->GetTexture(type, 0, &str);

		texturePath = str.C_Str();
	}

	return texturePath;
}
