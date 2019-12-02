#include "SimpleCar.h"
#include "PipelinesManager.h"
#include "DataManager.h"
#include <numeric>
#include <glm/gtx/string_cast.hpp>

void SimpleCar::update(float deltaTime)
{
	currentlyTravelled += speed * deltaTime;

	{
		float thisLineLength = glm::length(pathToTake[currentLine] - pathToTake[currentLine + 1]);
		if (currentlyTravelled > thisLineLength)
		{
			++currentLine;
			if (currentLine == pathToTake.size() - 1)
				currentLine = 0;

			currentlyTravelled = 0;
		}

		glm::vec3 direction = glm::normalize(pathToTake[currentLine + 1] - pathToTake[currentLine]);
		glm::vec3 newPosition = pathToTake[currentLine] + (direction * currentlyTravelled);

		float xRotation = glm::degrees(glm::acos(glm::dot(direction, glm::vec3(0.0, 0.0, 1.0))));
		m_graphicsComponent.setPosition(newPosition);
		m_graphicsComponent.setRotation(glm::vec3(xRotation, 0.0, 0.0));
	}
}

void SimpleCar::create(const Points& pathPoints)
{
	pathToTake = pathPoints;

	Info::ModelInfo mInfo;
	mInfo.modelPath = "resources/models/Car/Car.obj";

	setupModel(mInfo, true);
	m_graphicsComponent.setSize(glm::vec3(0.1));
}
