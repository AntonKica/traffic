#pragma once
#include <vector>
#include <string>
#include "GraphicsObjects.h"
#include "PipelinesManager.h"

//#include "DescriptorManager.h"
namespace Info
{
	struct ModelInfo
	{
		// model
		std::optional<std::string> modelPath;
		// or raw
		const GO::TypedVertices* vertices = nullptr;
		const GO::Indices* indices = nullptr;
		std::string texturePath;
	};
	struct GraphicsComponentCreateInfo
	{
		const DrawInfo* drawInfo;
		const ModelInfo* modelInfo;
	};
};
class GraphicsComponent
{
private:
	bool initialized = false;
	size_t dynamicBufferOffset;

public:
	GO::ID m_modelReference;
	// create module
	struct ImageRef
	{
		std::string textureFile;
		GO::ID imageID;
	};
	std::optional<GO::ID> textureRefID;
	GO::ID m_descriptorSetReference = {};
	GO::ID m_pipelineReference = {};

	glm::vec3 position = {};
	glm::vec3 rotation = {};
	glm::vec3 size = {1.0, 1.0, 1.0};
public:

	GraphicsComponent();
	void setBufferOffset(size_t offset);
	size_t getBufferOffset() const;
	//void processNode
};

