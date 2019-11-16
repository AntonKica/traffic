#include "GraphicsComponent.h"'

#include "DataManager.h"
#include "GraphicsObjects.h"
#include "PipelinesManager.h"
#include "GlobalObjects.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>


GraphicsModule::GraphicsModule()
{
}

void GraphicsModule::setBufferOffset(size_t offset)
{
	dynamicBufferOffset = offset;	
}

size_t GraphicsModule::getBufferOffset() const
{
	return dynamicBufferOffset;
}


GraphicsComponent::GraphicsComponent()
{
}

GraphicsComponent::~GraphicsComponent()
{
	freeGraphicsModule();
}

void GraphicsComponent::freeGraphicsModule()
{
	if(initialized())
	{
		App::Scene.vulkanBase->destroyGraphicsComponent(this);

		graphicsModule = {};
		init = false;
	}
}

void GraphicsComponent::recreateGraphics(const Info::GraphicsComponentCreateInfo& info)
{
	freeGraphicsModule();

	*this = *App::Scene.vulkanBase->createGrahicsComponent(info);
}

void GraphicsComponent::setPosition(const glm::vec3& pos)
{
	graphicsModule.position = pos;
}

void GraphicsComponent::setRotation(const glm::vec3& rotation)
{
	graphicsModule.rotation = rotation;
}

void GraphicsComponent::setGraphicsModule(const GraphicsModule& gModule)
{
	graphicsModule = gModule;
	init = true;
}

bool GraphicsComponent::initialized() const
{
	return init;
}

