#include "GraphicsComponent.h"'

#include "GraphicsObjects.h"
#include "PipelinesManager.h"
#include "GlobalObjects.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <utility>



GraphicsComponent* const GraphicsComponent::createGraphicsComponent()
{
	return App::vulkanBase.createGrahicsComponent();
}

GraphicsComponent* const GraphicsComponent::copyGraphicsComponent(const pGraphicsComponent& copyGraphicsComponent)
{
	return App::vulkanBase.copyGraphicsComponent(copyGraphicsComponent);
}
void GraphicsComponent::destroyGraphicsComponent(pGraphicsComponent& graphicsComponent)
{
	App::vulkanBase.deactivateGraphicsComponent(graphicsComponent);
}

GraphicsComponent::GraphicsComponent()
{
}

GraphicsComponent::~GraphicsComponent()
{
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

	return *this;
}

void GraphicsComponent::updateGraphicsComponent(const Info::GraphicsComponentCreateInfo& info)
{
	GraphicsComponent* cmp = this;
	App::vulkanBase.updateGrahicsComponent(cmp, info);
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
	m_active = value;
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

bool GraphicsComponent::initialized() const
{
	return m_initalized;
}

