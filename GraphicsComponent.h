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

// consider deleting
class GraphicsModule
{
private:
	size_t dynamicBufferOffset;

public:
	const ModelReference* pModelReference;
	// create module
	const vkh::structs::Image* pTexture;
	GO::ID m_descriptorSetReference = {};
	GO::ID m_pipelineReference = {};

	glm::vec3 position = {};
	glm::vec3 rotation = {};
	glm::vec3 size = { 1.0, 1.0, 1.0 };
public:

	GraphicsModule();
	void setBufferOffset(size_t offset);
	size_t getBufferOffset() const;
	//void processNode
};

class GraphicsComponent
{
	// stupid
	friend class VulkanBase;
private:
	GraphicsModule graphicsModule;
	bool init = false;
	//Info::GraphicsComponentCreateInfo createInfo;
	void freeGraphicsModule();
public:
	/*GraphicsComponent() = delete;
	GraphicsComponent(const GraphicsComponent& other) = delete;
	GraphicsComponent(GraphicsComponent&& other);
	GraphicsComponent& operator=(const GraphicsComponent& other) = delete;
	GraphicsComponent& operator=(GraphicsComponent&& other);
	*/
	GraphicsComponent();
	~GraphicsComponent();

	void recreateGraphics(const Info::GraphicsComponentCreateInfo& info);
	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::vec3& rotation);
	//stupid
	void setGraphicsModule(const GraphicsModule& gModule);
	bool initialized() const;
};
using pGraphicsComponent = GraphicsComponent*;
