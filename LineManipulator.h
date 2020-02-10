#pragma once
#include "BasicGeometry.h"

namespace LineManipulator
{
	namespace details
	{
		static Point vectorIntersection(Point s1, Point e1, Point s2, Point e2)
		{
			float a1 = e1.z - s1.z;
			float b1 = s1.x - e1.x;
			float c1 = a1 * s1.x + b1 * s1.z;

			float a2 = e2.z - s2.z;
			float b2 = s2.x - e2.x;
			float c2 = a2 * s2.x + b2 * s2.z;

			// round if one degree
			const float minDegrees = 0.01f;
			const float minAngle = minDegrees * (glm::pi<float>() / 180.0f);
			float delta = a1 * b2 - a2 * b1;

			return std::abs(delta) <= minAngle ? s2 : Point((b2 * c1 - b1 * c2) / delta, 0.0f, (a1 * c2 - a2 * c1) / delta);
		}
	}

	static Points getShiftedPointsFromAxisToLineInPercetageDistance(const Points& axis, const Points& line, float percentageDistanceFromAxis)
	{
		Points newPoints(line);
		for (uint32_t index = 0; index < axis.size(); ++index)
		{
			const auto dir = glm::normalize(line[index] - axis[index]);
			const auto dist = glm::length(line[index] - axis[index]);

			newPoints[index] = axis[index] + (dir * dist * percentageDistanceFromAxis);
		}

		return newPoints;
	}

	static Points getShiftedPointsFromLineToAxisInPercetageDistance(const Points& axis, const Points& line, float percentageDistanceFromLine)
	{
		Points newPoints(line);
		for (uint32_t index = 0; index < axis.size(); ++index)
		{
			const auto dir = glm::normalize(axis[index] - line[index]);
			const auto dist = glm::length(axis[index] - line[index]);

			newPoints[index] = line[index] + (dir * dist * percentageDistanceFromLine);
		}

		return newPoints;
	}

	static Points getShiftedPointsFromAxisToLineInSetDistance(const Points& axis, const Points& line, float setDistanceFromAxis)
	{
		Points newPoints(line);
		for (uint32_t index = 0; index < axis.size(); ++index)
		{
			const auto dir = glm::normalize(line[index] - axis[index]);

			newPoints[index] = axis[index] + (dir * setDistanceFromAxis);
		}

		return newPoints;
	}

	static Points getShiftedPointsFromLineToAxisInSetDistance(const Points& axis, const Points& line, float setDistanceFromLine)
	{
		Points newPoints(line);
		for (uint32_t index = 0; index < axis.size(); ++index)
		{
			const auto dir = glm::normalize(axis[index] - line[index]);

			newPoints[index] = line[index] + (dir * setDistanceFromLine);
		}

		return newPoints;
	}

	static Points getShiftedPointsToRightFromLineInSetDistance(const Points& line, float setDistanceFromLine)
	{
		Points newPoints(line);

		glm::vec3 curDir = {};
		for (uint32_t indexOne = 0, indexTwo = 1; indexOne < line.size(); ++indexOne, ++indexTwo)
		{
			if (indexTwo != line.size())
				curDir = glm::normalize(line[indexTwo] - line[indexOne]);

			const glm::vec3 up(0.0, 1.0, 0.0);
			const auto rightDir = -glm::cross(curDir, up);

			newPoints[indexOne] = line[indexOne] + (rightDir * setDistanceFromLine);
		}

		return newPoints;
	}

	// keeps distance between edges
	static Points getShiftedLineToLeftFromLineInSetDistance(const Points& line, float setDistanceFromLine)
	{
		if (line.size() <= 1)
			throw std::runtime_error("Passed line with less than two points");

		Points newPoints(line);
		if (line.size() == 2)
		{
			const glm::vec3 up(0.0, 1.0, 0.0);
			auto direction = glm::normalize(line[1] - line[0]);
			auto perpDir = glm::cross(direction, up);

			newPoints[0] = line[0] + perpDir * setDistanceFromLine;
			newPoints[1] = line[1] + perpDir * setDistanceFromLine;
		}
		else
		{
			const glm::vec3 up(0.0, 1.0, 0.0);
			// set start
			{
				auto direction = glm::normalize(line[1] - line[0]);
				auto perpDirLeft = glm::cross(direction, up);

				newPoints[0] = line[0] + perpDirLeft * setDistanceFromLine;
			}
			// and end
			{
				auto direction = glm::normalize(line[line.size() - 1] - line[line.size() - 2]);
				auto perpDirLeft = glm::cross(direction, up);

				newPoints.back() = line.back() + perpDirLeft * setDistanceFromLine;
			}

			for (uint32_t previousIndex = 0, currentIndex = 1, nextIndex = 2; 
				nextIndex < line.size(); ++ previousIndex, ++currentIndex, ++nextIndex)
			{
				const auto& previousPoint = line[previousIndex];
				const auto& currentPoint = line[currentIndex];
				const auto& nextPoint = line[nextIndex];

				const auto dirPreviousToCurrent = glm::normalize(currentPoint - previousPoint);
				const auto dirCurenntToNext = glm::normalize(nextPoint - currentPoint);

				const auto dot = glm::dot(dirPreviousToCurrent, dirCurenntToNext);
				if (glm::epsilonEqual(dot, 1.0f, 0.01f))
				{
					const auto direction = (dirPreviousToCurrent + dirCurenntToNext) / 2.0f;
					const auto perpDirLeft = glm::cross(direction, up);

					newPoints[currentIndex] = currentPoint + perpDirLeft * setDistanceFromLine;
				}
				else
				{
					auto perpDirLeftFromPrevious = glm::cross(dirPreviousToCurrent, up);
					auto perpDirLeftToNext = glm::cross(dirCurenntToNext, up);

					const Point oneStart = previousPoint + perpDirLeftFromPrevious * setDistanceFromLine;
					const Point oneEnd = currentPoint + perpDirLeftFromPrevious * setDistanceFromLine;
					const Point twoStart = currentPoint + perpDirLeftToNext * setDistanceFromLine;
					const Point twoEnd = nextPoint + perpDirLeftToNext * setDistanceFromLine;

					newPoints[currentIndex] = details::vectorIntersection(oneStart, oneEnd, twoStart, twoEnd);
				}
			}
		}
		return newPoints;
	}
	// didnt know where to put this
	static void joinPositionVerticesAndIndices(VD::PositionVertices& joinVerticesTo, VD::Indices& joinIndicesTo,
		const VD::PositionVertices& verticesToJoin, const VD::Indices& indicesToJoin)
	{
		// self explaining
		const uint32_t indicesIndexOffset = joinVerticesTo.size();

		// vertices wii be only copied
		joinVerticesTo.insert(joinVerticesTo.end(), verticesToJoin.begin(), verticesToJoin.end());
		// and apply indexOffset
		std::transform(std::begin(indicesToJoin), std::end(indicesToJoin), std::back_inserter(joinIndicesTo),
			[&indicesIndexOffset](const VD::Index& index)
			{
				return index + indicesIndexOffset;
			});
	}


	template <class PointsType> auto //std::tuple<typename PointsType::iterator, typename PointsType::iterator, float> 
		travellDistanceOnPoints(float distance, PointsType& points)
	{
		float travelledDistance = 0;
		auto currentPoint = points.begin();
		auto nextPoint = currentPoint + 1;
		while (nextPoint != points.end())
		{
			travelledDistance += glm::length(*currentPoint - *nextPoint);

			if (travelledDistance >= distance)
				break;
			else
				currentPoint = nextPoint++;
		}

		return std::make_tuple(currentPoint, nextPoint, travelledDistance);
	}

 //std::tuple<typename PointsType::iterator, typename PointsType::iterator, float> 
	static auto cutPointsInDistance(Points& points, float distanceToTravell)
		-> std::pair<Points, std::optional<float>>
	{
		Points travelled = points.size() ? Points{points[0]} : Points{};
		std::optional<float> distanceLeft;

		if (points.size() >= 2)
		{
			float travelledDistance = 0;

			auto currentPoint = points.begin();
			auto nextPoint = currentPoint + 1;
			while (nextPoint != points.end())
			{
				// go to closest point past 
				travelledDistance += glm::length(*currentPoint - *nextPoint);

				if (travelledDistance < distanceToTravell)
				{
					travelled.emplace_back(*currentPoint);

					++currentPoint;
					++nextPoint;
				}
				else
				{
					// we came past
					break;
				}
			}

			// we didnt travell enough
			if (nextPoint == points.end())
			{
				distanceLeft = distanceToTravell - travelledDistance;
				points.clear();
			}
			else //we've travelled enough
			{
				const auto curDir = glm::normalize(*nextPoint - *currentPoint);
				const auto curLength = glm::length(*nextPoint - *currentPoint);
				const auto overstepped = glm::length(travelledDistance - distanceToTravell);

				// calculate point
				const float newDistance = curLength - overstepped;
				Point newPoint = *currentPoint + curDir * newDistance;

				travelled.emplace_back(newPoint);

				// update points
				*currentPoint = newPoint;
				points.erase(points.begin(), currentPoint);
			}
		}
		else
		{
			distanceLeft = distanceToTravell;
		}

		return std::make_pair(travelled, distanceLeft);
	}
};