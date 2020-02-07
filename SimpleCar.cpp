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
	handleNearbyCars();

	advancePath();
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

	m_pathToTake = ptsToDrive;

	Info::ModelInfo mInfo;
	mInfo.model = "resources/models/car2/car2.obj";

	setupModel(mInfo, true);
	getGraphicsComponent().setSize(glm::vec3(3));

	setupDetection();
}

bool SimpleCar::finishedDriving() const
{
	return !m_pathToTake.empty();
}

void SimpleCar::advancePath()
{
	// get current frame travell distance
	float distanceLeftToTravell = kilometersPerSecondToMeters(m_speed) * App::time.deltaTime();

	// move last point, we may pass several points in one step
	while(true)
	{
		// have we finished path?
		if (m_pathToTake.size() < 2)
		{
			m_pathToTake.clear();
			break;
		}

		//otherwise try advancing to next
		auto& currentPoint = m_pathToTake.front();
		auto& nextPoint = *(m_pathToTake.begin() + 1);

		float distanceToNextPoint = glm::length(currentPoint - nextPoint);

		// we travell past the next
		if (distanceToNextPoint < distanceLeftToTravell)
		{
			// pop toe front point
			m_pathToTake.erase(m_pathToTake.begin());

			// update path to travell to next point
			distanceLeftToTravell -= distanceToNextPoint;
		}
		else
		{
			auto currentSegmentDir = glm::normalize(nextPoint - currentPoint);
			currentPoint = currentPoint + currentSegmentDir * distanceLeftToTravell;

			break;
		}
	}

	updatePosition();
	updateDirectionAndRotation();
}

void SimpleCar::updateDirectionAndRotation()
{
	if (m_pathToTake.size() >= 2)
	{
		const LineSegment currentSegment = { m_pathToTake[0], m_pathToTake[1] };
		m_currentDirection = glm::normalize(currentSegment[1] - currentSegment[0]);

		const float xRotation = std::atan2(m_currentDirection.x, m_currentDirection.z) + glm::half_pi<float>();
		setRotation({ xRotation, 0.0, 0.0 });
	}
	else
	{

		//setRotation({});
	}
}

void SimpleCar::updatePosition()
{
	if (m_pathToTake.size() >= 2)
	{
		// then position, its the firs point of path to take
		glm::vec3 newPosition = m_pathToTake.front();;
		// no to be dug inthe ground
		newPosition.y += 0.3f * 3;
		setPosition(newPosition);
	}
	else
	{
		//setPosition({});
	}
}

void SimpleCar::setupDetection()
{
	const glm::vec3 forward(0.0, 0.0, 1.0);
	const glm::vec3 right(1.0, 0.0, 0.0);

	const float visionWidth = 25.0f;
	const float visionDistance = 75.0f;		// meters

	// set size
	m_size = glm::vec2(2.0, 3.0);
	auto collisionDimensions = (m_size * 0.5f);
	collisionDimensions.x += visionWidth * 0.5f;
	collisionDimensions.y += visionDistance;

	Points collisionPoints{
		// left points
		Point(-collisionDimensions.x, 0.0f, -collisionDimensions.y),
		Point(-collisionDimensions.x, 0.0f, collisionDimensions.y),
		// rewersed right points
		Point(collisionDimensions.x, 0.0f, collisionDimensions.y),
		Point(collisionDimensions.x, 0.0f, -collisionDimensions.y),
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

	bool waitForPassingCar = false;
	for(const auto& simObject : possibleCollision)
	{
		// we know its car
		auto& otherCar = *static_cast<SimpleCar*>(simObject);

		if (!inAlertDistance(otherCar))
			continue;
		if (!isInFrontOf(otherCar))
			continue;
		if (!isOnMyFuturePath(otherCar))
			continue;

		if (glm::length(getPosition() - otherCar.getPosition()) < 1.2f * m_size.y)
		{
			waitForPassingCar = true;
			break;
		}
	}

	// update speed
	if (waitForPassingCar)
		m_speed = 0.0f;
	else
		m_speed = 50.0f;
}

bool SimpleCar::inAlertDistance(const SimpleCar& other)
{
	const float minDistance = 40.0f; // 40 meters
	float distance = glm::length(getPosition() - other.getPosition());

	return distance < minDistance;
}

bool SimpleCar::isInFrontOf(const SimpleCar& other)
{
	// in 180 degree in front of car itself
	const auto maxAngleForBeingForward = glm::half_pi<float>();
	const auto dirToOther = glm::normalize(other.getPosition() - getPosition());
	const auto angleWithOther = glm::acos(glm::dot(m_currentDirection, dirToOther));
	bool isInFront = angleWithOther < maxAngleForBeingForward;

	return angleWithOther < maxAngleForBeingForward;
}

bool SimpleCar::isOnMyFuturePath(const SimpleCar& other)
{
	// safety check
	if (m_pathToTake.empty())
		return false;

	// check 50 m ahead
	const float distanceToCheck = 50.0f;
	const auto [trailToCheck, _] = travellDistanceOnTrailFromPointInDirection(distanceToCheck, m_pathToTake, m_pathToTake[0], false);

	if (trailToCheck.size() >= 2)
	{
		// get uf is car on my future path
		auto otherPosition = other.getPosition();
		// flatten ypos
		otherPosition.y = 0;

		return pointSitsOnTrail(otherPosition, trailToCheck);
	}

	return false;
}

