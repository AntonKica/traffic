#pragma once
#include "BasicRoad.h"

#include <vector>
#include <stack>
#include <algorithm>

namespace PathFinding
{
	struct TravellRoad
	{
		BasicRoad* road;
		std::stack<BasicRoad*> toVisit;

		static TravellRoad createTravellRoad(BasicRoad* road, BasicRoad* comesFrom)
		{
			TravellRoad travellRoad;
			travellRoad.road = road;

			for (const auto& connection : road->getConnections())
			{
				if(connection.connected != comesFrom)
					travellRoad.toVisit.emplace(connection.connected);
			}

			return travellRoad;
		}
	};
	using TravelledRoads = std::vector<TravellRoad>;

	using pBasicRoad = BasicRoad*;
	using RoadRoute = std::vector<pBasicRoad>;
	using RoadRoutes = std::vector<RoadRoute>;

	struct TravellSegment
	{
		BasicRoad* road = nullptr;
		Path path = {};
		std::optional<Path> alternativeExitPath;
	};
	using TravellSegments = std::vector<TravellSegment>;

	static TravellSegments transferRoadRouteToTravellSegments(const RoadRoute& route)
	{
		TravellSegments travellSegments(route.size());
		std::transform(std::begin(route), std::end(route), std::begin(travellSegments), [](const pBasicRoad& road)
			{
				TravellSegment segment;
				segment.road = road;

				return segment;
			});

		return travellSegments;
	}

	static void keepOnlyPathsConnectingTo(std::vector<Path>& paths, BasicRoad* connectsToRoad)
	{
		paths.erase(std::remove_if(std::begin(paths), std::end(paths),
			[&connectsToRoad](const Path& path)
			{	
				return path.connectsTo != connectsToRoad;
			}), 
			paths.end());
	}

	static void keepOnlyPathsConnectingFrom(std::vector<Path>& paths, BasicRoad* connectFromRoad)
	{
		paths.erase(std::remove_if(std::begin(paths), std::end(paths),
			[&connectFromRoad](const Path& path)
			{
				return path.connectsFrom != connectFromRoad;
			}),
			paths.end());
	}
}

PathFinding::RoadRoutes startTravellToDestinationAndGetAllRoutes(PathFinding::TravellRoad routeStart, BasicRoad* destination)
{
	using namespace PathFinding;
	// here we store routes
	RoadRoutes routes;

	std::vector<TravellRoad> travelledRoads = { routeStart };
	TravellRoad* currentTravellRoad = &travelledRoads.back();

	bool foundAllRoutes = false;
	while (!foundAllRoutes)
	{
		// no more other paths here, try to go lower level,
		// we need to retrieve another road
		if (currentTravellRoad->toVisit.empty())
		{
			// if no more or last travelled road (which we would pop)
			// we have found all of them
			if (travelledRoads.empty() || travelledRoads.size() <= 1)
			{
				foundAllRoutes = true;
			}
			else // else get last
			{
				travelledRoads.pop_back();
				currentTravellRoad = &travelledRoads.back();
			}
		}
		else // process current
		{
			// get next road to visit
			BasicRoad* nextRoad = currentTravellRoad->toVisit.top();
			currentTravellRoad->toVisit.pop();

			// have we found it?
			// dont continue, but extract
			if (nextRoad == destination)
			{
				auto extractRoute = [](const std::vector<TravellRoad>& toExtract)
				{
					RoadRoute route;
					std::transform(std::begin(toExtract), std::end(toExtract), std::back_inserter(route),
						[](const TravellRoad& travellRoad) { return travellRoad.road; });

					return route;
				};
				// add next as well
				RoadRoute newRoute = extractRoute(travelledRoads);
				newRoute.emplace_back(nextRoad);

				routes.emplace_back(newRoute);
			}
			else // retrieve next
			{
				// check if we were on this road before so we wont loop forever
				bool alreadyTravelled = false;
				for (const auto& travelledRoad : travelledRoads)
				{
					if (travelledRoad.road == nextRoad)
					{
						alreadyTravelled = true;
						break;
					}
				}

				if (!alreadyTravelled)
				{
					// create road travell point and make it current
					auto nextTravellRoad = TravellRoad::createTravellRoad(nextRoad, currentTravellRoad->road);
					currentTravellRoad = &travelledRoads.emplace_back(nextTravellRoad);
				}
			}
		}
	}

	return routes;
}

PathFinding::RoadRoutes createRoadRoutes(BasicRoad* startRoad, BasicRoad* endRoad)
{
	// create start route
	PathFinding::TravellRoad routeStart = 
		PathFinding::TravellRoad::createTravellRoad(startRoad, nullptr);

	return startTravellToDestinationAndGetAllRoutes(routeStart, endRoad);
}

PathFinding::RoadRoutes findRoadPath(BasicRoad* startRoad, BasicRoad* endRoad)
{
	if (startRoad == endRoad)
		return { { startRoad } };


	return createRoadRoutes(startRoad, endRoad);
}

/*
* Each given itrator must be in range of container
* namely next segment
*/
void reverseSegmentsUntilAllPathsAreConnected(
	PathFinding::TravellSegments::iterator prevSegment, 
	PathFinding::TravellSegments::iterator curSegment, 
	PathFinding::TravellSegments::iterator nextSegment,
	const PathFinding::TravellSegments& travellSegments)
{
	if (prevSegment == travellSegments.end() ||
		curSegment == travellSegments.end() ||
		nextSegment == travellSegments.end())
	{
		// trying to reverse invalid 
		throw std::runtime_error("trying to reverse invalid, not supported by this function");
	}

	// set current segment
	{
		auto possiblePaths = curSegment->road->getAllPathsConnectingTwoRoads(prevSegment->road, nextSegment->road);
		if (possiblePaths.empty())
			throw std::runtime_error("Cannot make path from this route");

		if (curSegment->road->canSwitchLanes())
			// take first one
			curSegment->alternativeExitPath = possiblePaths.front();
		else // this is path we need to go
			curSegment->path = possiblePaths.front();
	}

	bool pathsCanConnect = false;
	while (!pathsCanConnect)
	{
		//check previous path if we can connect to curent path
		if (Path::pathLeadsToPath(prevSegment->path, curSegment->path))
		{
			pathsCanConnect = true;
		}
		else
		{
			auto possiblePaths = prevSegment->road->getAllPathsConnectingTo(curSegment->road);
			if (possiblePaths.empty())
				throw std::runtime_error("Cannot make path from this route");

			if (prevSegment->road->canSwitchLanes())
			{
				// take first one
				prevSegment->alternativeExitPath = possiblePaths.front();
				pathsCanConnect = true;
			}
			else // this is path we need to go back
			{
				prevSegment->path = possiblePaths.front();

				if (prevSegment == travellSegments.begin())
				{
					throw std::runtime_error("Cannot make path from this route");
				}
				else
				{
					--prevSegment;	--curSegment;	--nextSegment;
				}
			}
		}
	}
}

PathFinding::TravellSegments findPathOnRoute(const PathFinding::RoadRoute& route, Point startPoint, Point endPoint)
{
	// no prevention from one route road
	auto segmentsToTravell = PathFinding::transferRoadRouteToTravellSegments(route);

	// prepare first segment and last segment
	{
		auto& firstSegment = segmentsToTravell.front();
		firstSegment.path = firstSegment.road->getClosestPath(startPoint);
	}

	if (segmentsToTravell.size() >= 3)
	{
		// start from begin
		auto prevSegment = segmentsToTravell.begin();
		auto curSegment = prevSegment + 1;
		auto nextSegment = curSegment + 1;

		// step each segment to make path
		while (nextSegment != segmentsToTravell.end())
		{
			const auto& connectingPathFromPrevious = prevSegment->alternativeExitPath ?
				prevSegment->alternativeExitPath.value() : prevSegment->path;

			// get possible paths from current
			auto possiblePaths = curSegment->road->getSubsequentPathsFromConnectingPath(connectingPathFromPrevious);

			//filter them
			PathFinding::keepOnlyPathsConnectingTo(possiblePaths, nextSegment->road);

			// if there are any, take first
			if (!possiblePaths.empty())
			{
				curSegment->path = possiblePaths[0];
			}
			// we must changle lane
			else
			{
				reverseSegmentsUntilAllPathsAreConnected(prevSegment, curSegment, nextSegment, segmentsToTravell);
			}

			// step forward
			++prevSegment;	++curSegment;	++nextSegment;
		}
	}

	// close with last segment
	{
		auto& oneBeforeLastSegment = *(segmentsToTravell.end() - 2);
		auto& oneBeforeLastPath = oneBeforeLastSegment.alternativeExitPath ? 
			oneBeforeLastSegment.alternativeExitPath.value() : oneBeforeLastSegment.path;
		auto& lastSegment = segmentsToTravell.back();

		// get possible paths from previous road so we can decide if we can continue to next
		auto possiblePaths = lastSegment.road->getSubsequentPathsFromConnectingPath(oneBeforeLastPath);
		if (possiblePaths.empty())
			throw std::runtime_error("Cannot make path from this route");

		bool mustChangeLastPath = true;
		// check if can use same path
		auto closestPath = lastSegment.road->getClosestPath(endPoint);
		for (const auto& possiblePath : possiblePaths)
		{
			if (closestPath == possiblePath)
			{
				mustChangeLastPath = false;
				break;
			}
		}

		if (mustChangeLastPath)
		{
			// first
			oneBeforeLastSegment.path = possiblePaths[0];
			oneBeforeLastSegment.alternativeExitPath = closestPath;
		}
		else
		{
			oneBeforeLastSegment.path = closestPath;
		}
	}

	return segmentsToTravell;
}