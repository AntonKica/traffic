#include "GraphicsComponent.h"'

#include "GraphicsObjects.h"
#include "PipelinesManager.h"
#include "GlobalObjects.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <utility>



GraphicsComponent::GraphicsComponent()
{
}

GraphicsComponent::~GraphicsComponent()
{
	freeGraphics();
}

static int otherCnt = 0;
GraphicsComponent::GraphicsComponent(const GraphicsComponent& other)
{
	if (this == &other)
		return;

	m_initalized		= other.m_initalized;
	m_active			= other.m_active;
	m_modelData			= other.m_modelData;
	m_transformations	= other.m_transformations;
	m_shaderInfo		= other.m_shaderInfo;
	//App::Scene.vulkanBase.copyGrahicsComponent(other, *this);

	updateActiveState();
}

GraphicsComponent::GraphicsComponent(GraphicsComponent&& other) noexcept
{
	if (this == &other)
		return;

	m_initalized		= std::move(other.m_initalized);
	m_active			= std::move(other.m_active);
	m_modelData			= std::move(other.m_modelData);
	m_transformations	= std::move(other.m_transformations);
	m_shaderInfo		= std::move(other.m_shaderInfo);

	other.freeGraphics();
	updateActiveState();
}
GraphicsComponent& GraphicsComponent::operator=(const GraphicsComponent& other)
{
	if (this == &other)
		return *this;

	m_initalized		= other.m_initalized;
	m_active			= other.m_active;
	m_modelData			= other.m_modelData;
	m_transformations	= other.m_transformations;
	m_shaderInfo		= other.m_shaderInfo;
		//App::Scene.vulkanBase.copyGrahicsComponent(other, *this);

	updateActiveState();

	return *this;
}

GraphicsComponent& GraphicsComponent::operator=(GraphicsComponent&& other) noexcept
{
	if (this == &other)
		return *this;

	m_initalized		= std::move(other.m_initalized);
	m_active			= std::move(other.m_active);
	m_modelData			= std::move(other.m_modelData);
	m_transformations	= std::move(other.m_transformations);
	m_shaderInfo		= std::move(other.m_shaderInfo);

	other.freeGraphics();
	updateActiveState();

	return *this;
}

void GraphicsComponent::createGraphicsComponent(const Info::GraphicsComponentCreateInfo& info)
{
	if (initialized())
		App::Scene.vulkanBase.recreateGrahicsComponent(*this, info);
	else
		*this = App::Scene.vulkanBase.createGrahicsComponent(info);
}

void GraphicsComponent::recreateGraphicsComponent(const Info::GraphicsComponentCreateInfo& info)
{
	if (initialized())
		App::Scene.vulkanBase.recreateGrahicsComponent(*this, info);
	else
		createGraphicsComponent(info);
}

void GraphicsComponent::setModelData(VD::ModelData modelData)
{
	m_modelData = modelData;
}

void GraphicsComponent::setInitialized(bool value)
{
	m_initalized = value;
}

void GraphicsComponent::setActive(bool value)
{
	if (m_active != value)
	{
		m_active = value;
		updateActiveState();
	}
}
void GraphicsComponent::setPosition(const glm::vec3& pos)
{
	m_transformations.position = pos;
}

void GraphicsComponent::setRotation(const glm::vec3& rotation)
{
	m_transformations.rotation = rotation;
}

void GraphicsComponent::setRotationX(float rotationX)
{
	m_transformations.rotation.x = rotationX;
}

void GraphicsComponent::setRotationY(float rotationY)
{
	m_transformations.rotation.y = rotationY;
}

void GraphicsComponent::setRotationZ(float rotationZ)
{
	m_transformations.rotation.z = rotationZ;
}

void GraphicsComponent::setSize(const glm::vec3& size)
{
	m_transformations.size = size;
}

void GraphicsComponent::setTint(const glm::vec4& tint)
{
	m_shaderInfo.tint = tint;
}

void GraphicsComponent::setTransparency(float transparency)
{
	m_shaderInfo.transparency = transparency;
}

glm::vec3 GraphicsComponent::getPosition() const
{
	return m_transformations.position;
}

glm::vec3 GraphicsComponent::getRotation() const
{
	return m_transformations.rotation;
}

float GraphicsComponent::getRotationX() const
{
	return m_transformations.rotation.x;
}

float GraphicsComponent::getRotationY() const
{
	return m_transformations.rotation.y;
}

float GraphicsComponent::getRotationZ() const
{
	return m_transformations.rotation.z;
}

glm::vec3 GraphicsComponent::getSize() const
{
	return m_transformations.size;
}

void GraphicsComponent::freeGraphics()
{
	App::Scene.vulkanBase.deactivateGraphicsComponent(this);
}

void GraphicsComponent::updateActiveState()
{

	if (m_active)
		App::Scene.vulkanBase.activateGraphicsComponent(this);
	else
		App::Scene.vulkanBase.deactivateGraphicsComponent(this);
}

bool GraphicsComponent::initialized() const
{
	return m_initalized;
}

