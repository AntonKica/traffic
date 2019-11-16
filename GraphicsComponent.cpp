#include "GraphicsComponent.h"'

#include "DataManager.h"
#include "GraphicsObjects.h"
#include "PipelinesManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>


GraphicsComponent::GraphicsComponent()
{
}

void GraphicsComponent::setBufferOffset(size_t offset)
{
	dynamicBufferOffset = offset;	
}

size_t GraphicsComponent::getBufferOffset() const
{
	return dynamicBufferOffset;
}

