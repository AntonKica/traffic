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
public:
	std::shared_ptr<ModelReference> pModelReference = nullptr;
	// create module
	const vkh::structs::Image* pTexture = nullptr;
	GO::ID m_descriptorSetReference = {};
	GO::ID m_pipelineReference = {};

	size_t dynamicBufferOffset;
	struct
	{
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 size = { 1.0, 1.0, 1.0 };
	} transformations;

	struct
	{
		glm::vec4 tint = {};
		float transparency = 0;
	} shaderInfo;
};
using pGraphicsModule = GraphicsModule*;

class GraphicsComponent
{
	// stupid
	friend class VulkanBase;
private:
	// stupid to have pointer
	pGraphicsModule graphicsModule = nullptr;
	bool active = false;
	//Info::GraphicsComponentCreateInfo createInfo;

	void freeGraphics();
	void setGraphicsModule(const pGraphicsModule& gModule);
	void updateActiveState();
public:
	GraphicsComponent();
	~GraphicsComponent();
	GraphicsComponent(const GraphicsComponent& other);
	GraphicsComponent(GraphicsComponent&& other) noexcept;
	GraphicsComponent& operator=(const GraphicsComponent& other);
	GraphicsComponent& operator=(GraphicsComponent&& other) noexcept;

	void createGraphicsComponent(const Info::GraphicsComponentCreateInfo& info);
	void recreateGraphicsComponent(const Info::GraphicsComponentCreateInfo& info);

	//void recreateGraphics(const Info::GraphicsComponentCreateInfo& info);
	const GraphicsModule& getGraphicsModule() const;
	void setBufferOffset(size_t offset);
	size_t getBufferOffset() const;

	void setActive(bool value);
	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::vec3& rotation);
	void setSize(const glm::vec3& size);
	void setTint(const glm::vec4& tint);
	void setTransparency(const float transparency);
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	glm::vec3 getSize() const;
	//stupid
	bool initialized() const;
};
//using pGraphicsComponent;