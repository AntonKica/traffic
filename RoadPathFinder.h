#pragma once
#include "BasicRoad.h"

#include <vector>
#include <stack>
#include <algorithm>

namespace PathFinding
{
	struct TravellRoad
	{
		const BasicRoad* road;
		std::stack<const BasicRoad*> toVisit;

		static TravellRoad createTravellRoad(const BasicRoad* road, const BasicRoad* comesFrom)
		{
			TravellRoad travellRoad;
			travellRoad.road = road;

			for (const auto& connection : road->getConnections())
			{
				if (connection.connected != comesFrom)
					travellRoad.toVisit.emplace(connection.connected);
			}

			return travellRoad;
		}
	};
	using TravelledRoads = std::vector<TravellRoad>;

	//using pBasicRoad = BasicRoad*;
	using cpBasicRoad = const BasicRoad*;
	using RoadRoute = std::vector<cpBasicRoad>;
	using RoadRoutes = std::vector<RoadRoute>;

	struct TravellSegment
	{
		cpBasicRoad road = nullptr;
		Lane lane = {};
		std::optional<Lane> switchLane;

		bool canConnectoTo(const TravellSegment& other)
		{
			if (switchLane)
				return switchLane->leadsToLane(other.lane);
			else
				return lane.leadsToLane(other.lane);
		}
	};
	using TravellSegments = std::vector<TravellSegment>;


	namespace details
	{
		static TravellSegments transferRoadRouteToTravellSegments(const RoadRoute& route)
		{
			TravellSegments travellSegments(route.size());
			std::transform(std::begin(route), std::end(route), std::begin(travellSegments), [](const cpBasicRoad& road)
				{
					TravellSegment segment;
					segment.road = road;

					return segment;
				});

			return travellSegments;
		}

		static void keepOnlyLanesConnectingTo(std::vector<Lane>& lanes,  cpBasicRoad connectsToRoad)
		{
			lanes.erase(std::remove_if(std::begin(lanes), std::end(lanes),
				[&connectsToRoad](const Lane& lane)
				{
					return lane.connectsTo != connectsToRoad;
				}),
				lanes.end());
		}

		static void keepOnlyLanesConnectingFrom(std::vector<Lane>& lanes,  cpBasicRoad connectFromRoad)
		{
			lanes.erase(std::remove_if(std::begin(lanes), std::end(lanes),
				[&connectFromRoad](const Lane& lane)
				{
					return lane.connectsFrom != connectFromRoad;
				}),
				lanes.end());
		}


		static PathFinding::RoadRoutes startTravellToDestinationAndGetAllRoutes(PathFinding::TravellRoad routeStart, const BasicRoad& destination)
		{
			using namespace PathFinding;
			// here we store routes
			RoadRoutes routes;

			std::vector<TravellRoad> travelledRoads = { routeStart };
			TravellRoad* currentTravellRoad = &travelledRoads.back();

			bool foundAllRoutes = false;
			while (!foundAllRoutes)
			{
				// no more other lanes here, try to go lower level,
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
					cpBasicRoad nextRoad = currentTravellRoad->toVisit.top();
					currentTravellRoad->toVisit.pop();

					// have we found it?
					// dont continue, but extract
					if (nextRoad == &destination)
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

		// lane is setup
		static void handleOneRoadSegment(PathFinding::TravellSegments& travellSegments, Lane startLane, Lane endLane)
		{
			auto& onlySegment = travellSegments[0];

			// assign and work with it
			onlySegment.lane = startLane;

			// need to switch lanes
			if (startLane != endLane)
				onlySegment.switchLane = endLane;
		}

		/*
		* Segment first and second segments need to have
		* set up lanes 
		*/
		static void handleTwoRoadSegments(PathFinding::TravellSegments& travellSegments, Lane startLane, Lane endLane)
		{
			auto& startSegment = travellSegments[0];
			auto& endSegment = travellSegments[1];

			// assign and work with it
			startSegment.lane = startLane;
			endSegment.lane = endLane;

			const auto& startRoad = startSegment.road;
			const auto& endRoad = endSegment.road;

			auto& startSegmentLane = startSegment.lane;
			auto& endSegmentLane = endSegment.lane;

			// if theyre not just right
			// we have to change switch lane in one of segments

			bool segmentsCompatible = startSegmentLane.leadsToLane(endSegmentLane);
			if(segmentsCompatible)
			{
				// start from start
				if (startSegment.road->canSwitchLanes())
				{
					auto possibleRoads = startRoad->getAllLanesConnectingTo(endRoad);

					// extists
					if (!possibleRoads.empty())
					{
						startSegment.switchLane = possibleRoads[0];
						segmentsCompatible = true;
					}
				}
				// still not compatible
				if (endSegment.road->canSwitchLanes() && !segmentsCompatible)
				{
					auto possibleRoads = endRoad->getAllLanesConnectingFrom(startRoad);
					// exists
					if (!possibleRoads.empty())
					{
						// we need to make switch lane previokus 
						// and set lane to possible
						endSegment.switchLane = endSegment.lane;
						endSegment.lane = possibleRoads[0];
						segmentsCompatible = true;
					}
				}
			}
		}

		/* Checks if this segments 
		* can connect to next segment
		* check if lanes are connected and if not
		* check for next segment wether it can change lane otherwise 
		* need to roll back
		* ... 
		* Can handle end segment too
		*/
		static bool segmentLanesConnectingTo(
			PathFinding::TravellSegment& currentSegment,
			PathFinding::TravellSegment& nextSegment, 
			const BasicRoad* nextSegmentsShouldLeadTo)
		{
			const auto& currentLane = currentSegment.switchLane ?
				currentSegment.switchLane.value() : currentSegment.lane;

			// get all roads from next segment which comes from
			// current's segment road
			if (currentLane.empty())
				return false;
			auto nextSegmentPossibleLanes = nextSegment.road->getSubsequentLanesConnectingFromLane(currentLane);
			// no possible lanes
			if (nextSegmentPossibleLanes.empty())
				return false;


			// are we handling end segment
			if (!nextSegmentsShouldLeadTo)
			{
				// find if we have lane hit
				auto& nextLane = nextSegment.lane;
				bool containsSameLane = false;
				for (const auto& possibleLane : nextSegmentPossibleLanes)
				{
					if (nextLane == possibleLane)
					{
						containsSameLane = true;
						break;
					}
				}

				// do we have hit?
				// no need to change
				if (containsSameLane)
				{
					return true;
				}
				else // check if we can switch lines
				{
					if (nextSegment.road->canSwitchLanes())
					{
						nextSegment.switchLane = nextSegmentPossibleLanes[0];
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			else
			{
				// filter them so they only connect to that next road
				keepOnlyLanesConnectingTo(nextSegmentPossibleLanes, nextSegmentsShouldLeadTo);

				// if no possible lanes
				if (nextSegmentPossibleLanes.empty())
				{
					return false;
				}
				else
				{
					//take first possible
					nextSegment.lane = nextSegmentPossibleLanes[0];

					return true;
				}
			}
		}

		/* Checks if this segments
		* can connect to next segment
		* this function supposes that current lane
		* couldnt change lane
		* ...
		* Can handle start segment too
		*/
		static bool repairSegmentPathsConnectingFrom(
			const BasicRoad* previousShouldSegmentConnectsFrom,
			PathFinding::TravellSegment& previousSegment,
			PathFinding::TravellSegment& currentSegment)
		{
			const auto& currentLane = currentSegment.lane;

			// get all roads from next segment which comes from

			// current's segment road
			auto previousSegmentPossibleLanes = previousSegment.road->getSubsequentLanesConnectingToLane(currentLane);
			// no possible lanes
			if (previousSegmentPossibleLanes.empty())
				return false;

			// are we on the start
			if (!previousShouldSegmentConnectsFrom)
			{
				// find if we have lane hit
				auto& previousLane = previousSegment.lane;
				bool containsSameLane = false;
				for (const auto& possibleLane : previousSegmentPossibleLanes)
				{
					if (previousLane == possibleLane)
					{
						containsSameLane = true;
						break;
					}
				}

				// do we have hit?
				// no need to change
				if (containsSameLane)
				{
					return true;
				}
				else // check if we can switch lines
				{
					if (previousSegment.road->canSwitchLanes())
					{
						// take frist possible
						previousSegment.switchLane = previousSegmentPossibleLanes[0];
						return true;
					}
					else
					{
						return false;
					}
				}
			}
			else
			{
				// filter them so they only connect to that next road
				keepOnlyLanesConnectingFrom(previousSegmentPossibleLanes, previousShouldSegmentConnectsFrom);

				// if no possible lanes
				if (previousSegmentPossibleLanes.empty())
				{
					return false;
				}
				else
				{
					//take first possible
					previousSegment.lane = previousSegmentPossibleLanes[0];

					return true;
				}
			}
		}

		static void handleMoreThanThreeRoadSegments(PathFinding::TravellSegments& travellSegments, Lane startLane, Lane endLane)
		{
			if (travellSegments.size() < 3)
				throw std::runtime_error("Too few segments to handle");

			// initialize start and end segment
			{
				auto& frontSegment = travellSegments.front();
				auto& backSegment = travellSegments.back();

				// assign and work with it
				frontSegment.lane = startLane;
				backSegment.lane = endLane;
			}

			{
				auto currentSegmentIt = travellSegments.begin();
				auto nextSegmentIt = currentSegmentIt + 1;

				bool finishedFinding = false;
				while (!finishedFinding)
				{
					bool succesfullySteppedForward;
					// check for end segment so we pass right arguments
					if (nextSegmentIt + 1 != travellSegments.end())
						succesfullySteppedForward =
							segmentLanesConnectingTo(*currentSegmentIt, *nextSegmentIt, (nextSegmentIt + 1)->road);
					else 
						succesfullySteppedForward = 
							segmentLanesConnectingTo(*currentSegmentIt, *nextSegmentIt, nullptr);


					// repair if not correct
					if (!succesfullySteppedForward)
					{
						if(currentSegmentIt == travellSegments.begin())
							throw std::runtime_error("Cannot repair this segments");

						// keep going backwards till we have repaired
						auto previousRepairSegmentIt = currentSegmentIt - 1;
						auto currentRepairSegmentIt = currentSegmentIt;

						bool finishedRepairing = false;
						while (!finishedRepairing)
						{
							bool succesfullyRepaired;
							if (previousRepairSegmentIt == travellSegments.begin())
							{
								succesfullyRepaired =	repairSegmentPathsConnectingFrom(nullptr, *previousRepairSegmentIt,
										*currentRepairSegmentIt);

								if (!succesfullyRepaired)
									throw std::runtime_error("Couldnt fix travell segments");
							}
							else
							{
								succesfullyRepaired =	repairSegmentPathsConnectingFrom((previousRepairSegmentIt - 1)->road,
									*previousRepairSegmentIt, *currentRepairSegmentIt);
							}

							if (succesfullyRepaired)
							{
								finishedRepairing = true;
							}
							else
							{
								--previousRepairSegmentIt;
								--currentRepairSegmentIt;
							}
						}
					}

					// step forward
					++currentSegmentIt;
					++nextSegmentIt;

					// check if finished
					finishedFinding = nextSegmentIt == travellSegments.end();
				}
			}
		}
	}


	static PathFinding::RoadRoutes createRoadRoutes(const BasicRoad& startRoad, const BasicRoad& endRoad)
	{
		if (&startRoad == &endRoad)
		{
			return { { &startRoad } };
		}

		// create start route
		PathFinding::TravellRoad routeStart =
			PathFinding::TravellRoad::createTravellRoad(&startRoad, nullptr);

		return details::startTravellToDestinationAndGetAllRoutes(routeStart, endRoad);
	}

	static PathFinding::TravellSegments findLaneOnRoute(const PathFinding::RoadRoute& route, Lane startLane, Lane endLane)
	{
		// no prevention from one route road
		auto segmentsToTravell = details::transferRoadRouteToTravellSegments(route);


		if (segmentsToTravell.size() == 1)
		{
			details::handleOneRoadSegment(segmentsToTravell, startLane, endLane);
		}
	   	else if (segmentsToTravell.size() == 2)
		{
			details::handleTwoRoadSegments(segmentsToTravell, startLane, endLane);
		}
		if (segmentsToTravell.size() >= 3)
		{
			details::handleMoreThanThreeRoadSegments(segmentsToTravell, startLane, endLane);
		}

		return segmentsToTravell;
	}
}