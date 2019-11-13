#pragma once

#include <vector>
#include <string>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Models.h"

#define ERROR_PREFIX "[ERROR::ASSIMP] - "


namespace ModelLoader
{
	Models::Model loadModel(const std::string& path);

	//just one node
	void processNode(const aiScene* scene, aiNode* node, Models::Model& model);

	void processMesh(const aiScene* scene, aiMesh* mesh, Models::Model& model);
	std::string getTexturePath(aiMaterial* mat, aiTextureType type);
}