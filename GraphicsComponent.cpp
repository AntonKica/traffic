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
	m_core = App::vulkanBase.createGrahicsComponentCore();
}

GraphicsComponent::~GraphicsComponent()
{
	if (m_core)
		App::vulkanBase.deactivateGraphicsComponentCore(m_core);
}

GraphicsComponent::GraphicsComponent(const GraphicsComponent& copy)
{
	if (this == &copy)
		return;

	if(copy.m_core)
		m_core = App::vulkanBase.copyCreateGraphicsComponentCore(copy.m_core);
}

GraphicsComponent::GraphicsComponent(GraphicsComponent&& move) noexcept
{
	if (this == &move)
		return;

	m_core = move.m_core;	move.m_core = nullptr;
}
GraphicsComponent& GraphicsComponent::operator=(const GraphicsComponent& copy)
{
	if (this == &copy)
		return *this;

	if (m_core)
	{
		if (copy.m_core)
			App::vulkanBase.copyGraphicsComponentCore(copy.m_core, m_core);
		else
			App::vulkanBase.deactivateGraphicsComponentCore(m_core);
	}
	else if (copy.m_core) 
	{
		if (copy.m_core)
			App::vulkanBase.copyCreateGraphicsComponentCore(copy.m_core);
	}

	return *this;
}

GraphicsComponent& GraphicsComponent::operator=(GraphicsComponent&& move) noexcept
{
	if (this == &move)
		return *this;

	if(m_core)
		App::vulkanBase.deactivateGraphicsComponentCore(m_core);
	m_core = move.m_core;	move.m_core = nullptr;

	return *this;
}

void GraphicsComponent::updateGraphicsComponent(const Info::GraphicsComponentCreateInfo& info)
{
	App::vulkanBase.updateGrahicsComponentCore(m_core, info);
}


void GraphicsComponent::setActive(bool value)
{
	m_core->active = value;
}
void GraphicsComponent::setPosition(const glm::vec3& pos)
{
	m_core->transformations.position = pos;
}

void GraphicsComponent::setRotation(const glm::vec3& rotation)
{
	m_core->transformations.rotation = rotation;
}

void GraphicsComponent::setRotationX(float rotationX)
{
	m_core->transformations.rotation.x = rotationX;
}

void GraphicsComponent::setRotationY(float rotationY)
{
	m_core->transformations.rotation.y = rotationY;
}

void GraphicsComponent::setRotationZ(float rotationZ)
{
	m_core->transformations.rotation.z = rotationZ;
}

void GraphicsComponent::setSize(const glm::vec3& size)
{
	m_core->transformations.size = size;
}

void GraphicsComponent::setTint(const glm::vec4& tint)
{
	m_core->shaderInfo.tint = tint;
}

void GraphicsComponent::setTransparency(float transparency)
{
	m_core->shaderInfo.transparency = transparency;
}

bool GraphicsComponent::isActive() const
{
	return m_core->active;
}

glm::vec3 GraphicsComponent::getPosition() const
{
	return m_core->transformations.position;
}

glm::vec3 GraphicsComponent::getRotation() const
{
	return m_core->transformations.rotation;
}

float GraphicsComponent::getRotationX() const
{
	return m_core->transformations.rotation.x;
}

float GraphicsComponent::getRotationY() const
{
	return m_core->transformations.rotation.y;
}

float GraphicsComponent::getRotationZ() const
{
	return m_core->transformations.rotation.z;
}

glm::vec3 GraphicsComponent::getSize() const
{
	return m_core->transformations.size;
}
