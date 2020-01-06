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


struct GraphicsComponentCore
{
	VD::ModelData modelData = {};

	// Rotation is by default in radians
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

	bool active = false;
};
using pGraphicsComponentCore = GraphicsComponentCore*;

class GraphicsComponent
{
	// neccesary
	friend class VulkanBase;
	friend class SimulationObject;

public:
	GraphicsComponent();
	~GraphicsComponent();
	GraphicsComponent(const GraphicsComponent& other);
	GraphicsComponent(GraphicsComponent&& other) noexcept;
	GraphicsComponent& operator=(const GraphicsComponent& other);
	GraphicsComponent& operator=(GraphicsComponent&& other) noexcept;

	void updateGraphicsComponent(const Info::GraphicsComponentCreateInfo& info);
	void setActive(bool value);
//private:
	//void createGraphicsComponent(const Info::GraphicsComponentCreateInfo& info);
	//void recreateGraphicsComponent(const Info::GraphicsComponentCreateInfo& info);

	void setPosition(const glm::vec3& pos);
	void setRotation(const glm::vec3& rotation);
	void setRotationX(float rotationX);
	void setRotationY(float rotationY);
	void setRotationZ(float rotationZ);

	void setSize(const glm::vec3& size);
	void setTint(const glm::vec4& tint);
	void setTransparency(float transparency);

	bool isActive() const;
	glm::vec3 getPosition() const;
	glm::vec3 getRotation() const;
	float getRotationX() const;
	float getRotationY() const;
	float getRotationZ() const;
	glm::vec3 getSize() const;

private:
	pGraphicsComponentCore m_core = nullptr;
};
