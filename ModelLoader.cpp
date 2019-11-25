#include "ModelLoader.h"
#include <filesystem>

using namespace Models;
Model ModelLoader::loadModel(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

	if (!scene)
	{
		std::cerr << "Failed to load model!\n" << ERROR_PREFIX << importer.GetErrorString();
		throw std::runtime_error("Unknfown texture path: " + path);
	}
	Model newModel;
	newModel.directory = path.substr(0, path.find_last_of('/'));

	processNode(scene, scene->mRootNode, newModel);

	return newModel;
}

//just one node
void ModelLoader::processNode(const aiScene* scene, aiNode* node, Model& model)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		processMesh(scene, mesh, model);
		const auto& [type, vertices] = model.vertices;
		if (vertices.size())
			return;
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(scene, node->mChildren[i], model);
	}

	//if (node->mNumChildren)
	//	std::cerr << '\t' << node->mNumChildren << " were left unprocessed!\n";
}

void ModelLoader::processMesh(const aiScene* scene, aiMesh* mesh, Model& model)
{
	GO::VertexType vertexType = VertexType::DEFAULT;
	GO::ByteVertices vertices;
	GO::Indices indices;

	bool containsTexture = mesh->mTextureCoords[0] != nullptr;
	if (containsTexture)
	{
		vertexType = VertexType::TEXTURED;
	}
	auto vertexSize = getVertexSize(vertexType);

	vertices.type = vertexType;
	vertices.vertices.resize(mesh->mNumVertices * vertexSize);
	auto vData = vertices.vertices.data();

	for (size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		glm::vec3 vector;
		// vertices
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;

		VariantVertex variantVertex;
		variantVertex.vertex.position = vector;

		// normals

		// textures
		if (containsTexture) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			variantVertex.texturedVertex.texCoord = vec;
		}

		std::memcpy(vData, &variantVertex, vertexSize);
		vData += vertexSize;
	}
	model.vertices = vertices;

	// indices
	for (size_t i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
	}
	model.indices = indices;

	// texture
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		for (int i = aiTextureType_DIFFUSE; i <= aiTextureType_UNKNOWN; ++i)
		{
			std::string path = getTexturePath(material, static_cast<aiTextureType>(i));
			if (!path.empty())
			{
				model.texturePath = model.directory.value() + '/' + path;
				break;
			}
		}
	}
	if (containsTexture)
	{
		std::cout << '\n';
	}
}

std::string ModelLoader::getTexturePath(aiMaterial* mat, aiTextureType type)
{
	std::string texturePath;

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
