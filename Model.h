#pragma once
#include "Mesh.h"
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model() = default;
	Model(std::string file);

	void loadModel(std::string file);

	std::string directory;
	std::vector<Mesh> meshes;
private:
	void processNode(const aiScene* scene, aiNode* node);
	Mesh processMesh(const aiScene* scene, aiMesh* mesh);

	std::optional<std::string> getTexturePath(aiMaterial* mat, aiTextureType type);
};

