#pragma once
#include <vector>
#include <string>

#include "vulkanHelper/VulkanStructs.h"
#include "VulkanDataManager.h"

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

class GraphicsComponent
{
	// stupid
	friend class VulkanBase;
private:
	// stupid to have pointer
	bool m_initalized = false;
	bool m_active = false;
	//Info::GraphicsComponentCreateInfo createInfo;

	void freeGraphics();
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

	void setModelData(VD::ModelData modelData);
	void setInitialized(bool value);
	void setActive(bool value);
	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::vec3& rotation);
	void setRotationX(float rotationX);
	void setRotationY(float rotationY);
	void setRotationZ(float rotationZ);

	void setSize(const glm::vec3& size);
	void setTint(const glm::vec4& tint);
	void setTransparency(float transparency);

	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	float getRotationX() const;
	float getRotationY() const;
	float getRotationZ() const;
	glm::vec3 getSize() const;
	//stupid
	bool initialized() const;

private:
	VD::ModelData m_modelData = {};

	/*
	* Rotation is by default in radians
	*/
	struct
	{
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 size = { 1.0, 1.0, 1.0 };
	} m_transformations;

	struct
	{
		glm::vec4 tint = {};
		float transparency = 0;
	} m_shaderInfo;
};
//using pGraphicsComponent;