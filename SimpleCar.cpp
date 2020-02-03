#include "SimpleCar.h"
#include "PipelinesManager.h"
#include <numeric>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "GlobalObjects.h"

void SimpleCar::update()
{
	currentlyTravelled += speed * App::time.deltaTime();

	{
		float thisLineLength = glm::length(pathToTake[currentLine] - pathToTake[currentLine + 1]);
		if (currentlyTravelled > thisLineLength)
		{
			++currentLine;
			if (currentLine == pathToTake.size() - 1)
				currentLine = 0;

			currentlyTravelled = 0;
		}

		const glm::vec3 direction = glm::normalize(pathToTake[currentLine + 1] - pathToTake[currentLine]);
		glm::vec3 newPosition = pathToTake[currentLine] + (direction * currentlyTravelled);
		newPosition.y += 0.3f * 3;

		const float xRotation = std::atan2(direction.x, direction.z) + glm::half_pi<float>();

		getGraphicsComponent().setPosition(newPosition);
		getGraphicsComponent().setRotationX(xRotation);
	}
}

void SimpleCar::drive(PathFinding::TravellSegments travellSegments)
{
	Points ptsToDrive;
	for (const auto& travellSegment : travellSegments)
	{
		if (travellSegment.switchLane)
		{
			auto lane = travellSegment.switchLane.value();
			ptsToDrive.insert(ptsToDrive.end(), lane.points.begin(), lane.points.end());
		}
		else
		{
			auto lane = travellSegment.lane;
			ptsToDrive.insert(ptsToDrive.end(), lane.points.begin(), lane.points.end());
		}
	}
	removeDuplicates(ptsToDrive);

	pathToTake = ptsToDrive;
	speed = 7.0f;

	Info::ModelInfo mInfo;
	mInfo.model = "resources/models/car2/car2.obj";

	setupModel(mInfo, true);
	getGraphicsComponent().setSize(glm::vec3(3));
}
