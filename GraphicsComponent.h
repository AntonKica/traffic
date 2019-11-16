#pragma once
#include <vector>
#include <string>

#include "vulkanHelper/VulkanStructs.h"
#include "GraphicsObjects.h"
//#include "DescriptorManager.h"
namespace Info
{
	struct ModelInfo;
	struct DrawInfo;
	struct GraphicsComponentCreateInfo
	{
		const Info::DrawInfo* drawInfo;
		const Info::ModelInfo* modelInfo;
	};
};

struct ModelReference;
class GraphicsComponent
{
private:
	bool initialized = false;
	size_t dynamicBufferOffset;

public:
	const ModelReference* pModelReference;
	// create module
	const vkh::structs::Image* pTexture;
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

