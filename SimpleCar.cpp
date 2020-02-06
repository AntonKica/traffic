#include "SimpleCar.h"
#include "PipelinesManager.h"
#include <numeric>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "GlobalObjects.h"

namespace
{
	float kilometersPerSecondToMeters(float mps)
	{
		return mps / 3.6f;
	}
}
void SimpleCar::update()
{
	currentlyTravelled += kilometersPerSecondToMeters(speed) * App::time.deltaTime();

	{
		float thisLineLength = glm::length(pathToTake[currentLine] - pathToTake[currentLine + 1]);
		if (currentlyTravelled > thisLineLength)
		{
			++currentLine;
			if (currentLine == pathToTake.size() - 1)
				currentLine = 0;

			currentlyTravelled = 0;
		}

		m_currentDirection = glm::normalize(pathToTake[currentLine + 1] - pathToTake[currentLine]);
		glm::vec3 newPosition = pathToTake[currentLine] + (m_currentDirection * currentlyTravelled);
		newPosition.y += 0.3f * 3;

		const float xRotation = std::atan2(m_currentDirection.x, m_currentDirection.z) + glm::half_pi<float>();

		setPosition(newPosition);
		setRotation({ xRotation, 0.0, 0.0 });
	}

	handleNearbyCars();
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
	for (auto begin = ptsToDrive.begin(); begin + 1 != ptsToDrive.end();)
	{
		if (approxSamePoints(*begin, *(begin + 1)))
			begin = ptsToDrive.erase(begin);
		else
			++begin;
	}

	pathToTake = ptsToDrive;
	speed = 50.0f;

	Info::ModelInfo mInfo;
	mInfo.model = "resources/models/car2/car2.obj";

	setupModel(mInfo, true);
	getGraphicsComponent().setSize(glm::vec3(3));

	setupCollision();
}

void SimpleCar::setupCollision()
{
	m_size = glm::vec2(2, 3);
	m_collisionSizeMultiplier = 3.0f;
	auto halfSize = (m_size / 2.0f) * m_collisionSizeMultiplier;

	Points collisionPoints{ 
		Point(-halfSize.x, 0.0f, -halfSize.y),  Point(halfSize.x, 0.0f, -halfSize.y),
		Point(halfSize.x, 0.0f, halfSize.y) ,  Point(-halfSize.x, 0.0f, halfSize.y)
	};

	auto& physicsComponent = getPhysicsComponent();
	physicsComponent.collider().setBoundaries(collisionPoints);
	physicsComponent.setSelfCollisionTags({ "CAR" });
	physicsComponent.setOtherCollisionTags({ "CAR" });
	enablePhysics();
}

void SimpleCar::handleNearbyCars()
{
	auto& physicsComponent = getPhysicsComponent();
	auto possibleCollision = physicsComponent.getAllCollisionWith("CAR");
	// we know its car
	for(const auto& simObject : possibleCollision)
	{
		auto otherCar = static_cast<SimpleCar*>(simObject);

		auto angleWithOther = glm::acos(glm::dot(m_currentDirection, otherCar->m_currentDirection));

		if (angleWithOther <= glm::quarter_pi<float>())
		{
			speed = 0.0f;
		}
		else
		{
			speed = 50.0f;

		}
	}
}
