#include "GraphicsComponent.h"'

#include "DataManager.h"
#include "GraphicsObjects.h"
#include "PipelinesManager.h"
#include "GlobalObjects.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>



GraphicsComponent::GraphicsComponent()
{
	static int count = 0;
}

GraphicsComponent::~GraphicsComponent()
{
	freeGraphics();
}

static int otherCnt = 0;
GraphicsComponent::GraphicsComponent(const GraphicsComponent& other)
{
	App::Scene.vulkanBase.copyGrahicsComponent(other, *this);
	setActive(other.active);
	graphicsModule->transformations = other.graphicsModule->transformations;
}

GraphicsComponent::GraphicsComponent(GraphicsComponent&& other) noexcept
{
	graphicsModule = other.graphicsModule;
	setActive(other.active);

	other.setActive(false);
	other.graphicsModule = {};
}
GraphicsComponent& GraphicsComponent::operator=(const GraphicsComponent& other)
{
	App::Scene.vulkanBase.copyGrahicsComponent(other, *this);
	setActive(other.active);
	graphicsModule->transformations = other.graphicsModule->transformations;

	return *this;
}

GraphicsComponent& GraphicsComponent::operator=(GraphicsComponent&& other) noexcept
{
	graphicsModule = other.graphicsModule;
	setActive(other.active);

	other.setActive(false);
	other.graphicsModule = {};

	return *this;
}

void GraphicsComponent::createGraphicsComponent(const Info::GraphicsComponentCreateInfo& info)
{
	*this = App::Scene.vulkanBase.createGrahicsComponent(info);
}

void GraphicsComponent::recreateGraphicsComponent(const Info::GraphicsComponentCreateInfo& info)
{
	// tune up
	//std::cout << "Tune up recreateGraphicsComponent\n";i
	if (initialized())
		App::Scene.vulkanBase.recreateGrahicsComponent(*this, info);
	else
		createGraphicsComponent(info);
}

const GraphicsModule& GraphicsComponent::getGraphicsModule() const
{
	return *graphicsModule;
}

void GraphicsComponent::setBufferOffset(size_t offset)
{
	graphicsModule->dynamicBufferOffset = offset;
}

size_t GraphicsComponent::getBufferOffset() const
{
	return graphicsModule->dynamicBufferOffset;
}

void GraphicsComponent::setActive(bool value)
{
	if (active != value)
	{
		active = value;
		updateActiveState();
	}
}
void GraphicsComponent::setPosition(const glm::vec3& pos)
{
	graphicsModule->transformations.position = pos;
}

void GraphicsComponent::setRotation(const glm::vec3& rotation)
{
	graphicsModule->transformations.rotation = rotation;
}

void GraphicsComponent::setTint(const glm::vec4& tint)
{
	graphicsModule->shaderInfo.tint = tint;
}

void GraphicsComponent::setTransparency(const float transparency)
{
	graphicsModule->shaderInfo.transparency = transparency;
}

glm::vec3 GraphicsComponent::getPosition() const
{
	return graphicsModule->transformations.position;
}

glm::vec3 GraphicsComponent::getRotation() const
{
	return graphicsModule->transformations.rotation;
}

void GraphicsComponent::freeGraphics()
{
	if (initialized())
	{
		App::Scene.vulkanBase.destroyGraphicsComponent(*this);
	}
	setActive(false);
}

void GraphicsComponent::setGraphicsModule(const pGraphicsModule& gModule)
{
	// aware of copy
	graphicsModule = gModule;
}

void GraphicsComponent::updateActiveState()
{
	if (active)
		App::Scene.vulkanBase.activateGraphicsComponent(this);
	else
		App::Scene.vulkanBase.deactivateGraphicsComponent(this);
}

bool GraphicsComponent::initialized() const
{
	return graphicsModule != nullptr;
}

