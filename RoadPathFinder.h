#pragma once
#include "BasicRoad.h"

#include <vector>
#include <stack>

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

	using RoadRoute = std::vector<BasicRoad*>;
	using RoadRoutes = std::vector<RoadRoute>;
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