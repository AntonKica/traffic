#pragma once
#include "Utilities.h"
#include <algorithm>
#include <chrono>

namespace Collisions
{
	template <class T> struct MinMax
	{
		T min, max;
	};

	template <class PointsContainer> static MinMax<float> projectPolygonOnAxis(const PointsContainer& polygonPoints, const glm::vec3& axis)
	{
		float dotProduct = glm::dot(axis, polygonPoints[0]);
		float min = dotProduct;
		float max = dotProduct;
		
		for (const auto& point : polygonPoints)
		{
			dotProduct = glm::dot(axis, point);
			min = std::min(min, dotProduct);
			max = std::max(max, dotProduct);
		}

		return MinMax<float>{ min, max };
	}

	static float intervalDistance(const MinMax<float>& firstInterval, const MinMax<float>& secondInterval)
	{
		if (firstInterval.min < secondInterval.min)
			return secondInterval.min - firstInterval.max;
		else
			return firstInterval.min - secondInterval.max;
	}

	namespace details
	{
		template <class PolygonType> bool polygonPolygonCollision(const PolygonType& polygonOne, const PolygonType& polygonTwo)
		{
			bool canDrawLineBetween = false;

			// with first triangle
			for (uint32_t index = 0, nextIndex = 1; index < polygonOne.size(); ++index, ++nextIndex)
			{
				if (canDrawLineBetween)
					break;

				if (nextIndex == polygonOne.size())
					nextIndex = 0;

				const glm::vec3 edge = polygonOne[index] - polygonOne[nextIndex];

				const glm::vec3 vectorUp(0.0, 1.0, 0.0);
				const auto perpAxis = glm::normalize(glm::cross(edge, vectorUp));

				const auto firstInterval = projectPolygonOnAxis(polygonOne, perpAxis);
				const auto secondInterval = projectPolygonOnAxis(polygonTwo, perpAxis);

				canDrawLineBetween = intervalDistance(firstInterval, secondInterval) > 0;
			}

			// for second triangle
			if (!canDrawLineBetween)
			{
				for (uint32_t index = 0, nextIndex = 1; index < polygonTwo.size(); ++index, ++nextIndex)
				{
					if (canDrawLineBetween)
						break;

					if (nextIndex == polygonTwo.size())
						nextIndex = 0;

					const glm::vec3 edge = polygonTwo[index] - polygonTwo[nextIndex];

					const glm::vec3 vectorUp(0.0, 1.0, 0.0);
					const auto perpAxis = glm::normalize(glm::cross(edge, vectorUp));

					const auto firstInterval = projectPolygonOnAxis(polygonOne, perpAxis);
					const auto secondInterval = projectPolygonOnAxis(polygonTwo, perpAxis);

					canDrawLineBetween = intervalDistance(firstInterval, secondInterval) > 0;
				}
			}

			return !canDrawLineBetween;
		}

		constexpr auto triangleTriangleCollision = polygonPolygonCollision<Triangle>;
	}

	/*
	*	Points must be odrered in this order in order to work:
	*   
	*	P1	P2
	*	P3	P4
	*	P5	...
	*
	*/

	class Timer
	{
	public:
		Timer(bool start = false)
		{
			started = false;

			if (start)
				fire();
		}

		void fire()
		{
			started = true;
			start = std::chrono::system_clock::now();
		}
		void stop()
		{
			if (started)
			{
				measured = std::chrono::system_clock::now() - start;
				started = false;
			}
		}

		using Duration = std::chrono::system_clock::duration;
		static void printDuration(std::chrono::system_clock::duration d)
		{
			std::cout << "Duration: Miliseconds " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << '\n';
		}

		Duration getMeasured() const
		{
			return measured;
		}
	private:
		bool started;
		Duration measured;
		std::chrono::system_clock::time_point start;
	};

	static bool boolPolygonsCollide(const Points& polygonOnePoints, const Points& polygonTwoPoints)
	{
#ifdef  BENCHMARKING
		auto findCentreAndRadiusOfPoints = [](const Points& points)
		{
			float maxX = std::numeric_limits<float>::min();
			float minX = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();
			float minZ = std::numeric_limits<float>::max();

			Point average = {};
			for (const auto& p : points)
			{
				average += p;
				maxX = std::max(maxX, p.x);
				minX = std::min(minX, p.x);
				maxY = std::max(maxY, p.y);
				minY = std::min(minY, p.y);
				maxZ = std::max(maxZ, p.z);
				minZ = std::min(minZ, p.z);
			}
			average /= float(points.size());
			auto getGreater = [](float a, float b)
			{return a > b ? a : b; };

			float absX = getGreater(std::abs(minX - average.x), std::abs(maxX - average.x));
			float absY = getGreater(std::abs(minY - average.y), std::abs(maxY - average.y));
			float absZ = getGreater(std::abs(minZ - average.z), std::abs(maxZ - average.z));

			Point radiusPoint(absX, absY, absZ);

			return std::make_pair(average,glm::length(radiusPoint));
		};

		static Timer::Duration possibleSavedTimeWithPolygonal = {};
		static Timer::Duration possibleSavedTimeWithRadial = {};
		static Timer::Duration possibleWastedTimeWithPolygonal = {};
		static Timer::Duration possibleWastedTimeWithRadial = {};

		static Timer::Duration triagonalDuration;
		static Timer::Duration triagonalMaxDuration = std::chrono::milliseconds(std::numeric_limits<uint32_t>::min());
		static Timer::Duration triagonalMinDuration = std::chrono::milliseconds(std::numeric_limits<uint32_t>::max());
		static uint32_t colCount = 0;
		++colCount;

		// can improve performance in several occasions idk tho,
		// premature optimalisation it is

		Timer first, second;
		constexpr const uint32_t minPCountToVerify = 14;
		bool circleIntersect = false;
		bool polygonIntersect = false;
		if (true)
		{
			static Timer::Duration radialDuration = {};
			static Timer::Duration radialMaxDuration = std::chrono::milliseconds(std::numeric_limits<uint32_t>::min());
			static Timer::Duration radialMinDuration = std::chrono::milliseconds(std::numeric_limits<uint32_t>::max());

			static uint32_t radialWrongCount = 0;
			static uint32_t radialRightCount = 0;
			const auto [c1, r1] = findCentreAndRadiusOfPoints(polygonOnePoints);
			const auto [c2, r2] = findCentreAndRadiusOfPoints(polygonTwoPoints);

			const float circleDistances = glm::length(c1 - c2);

			first.fire();
			circleIntersect = circleDistances <= std::abs(r1 + r2);
			first.stop();

			radialDuration += first.getMeasured();
			radialMaxDuration = std::max(radialMaxDuration, first.getMeasured());
			radialMinDuration = std::min(radialMinDuration, first.getMeasured());
			if (circleIntersect)
			{
				++radialRightCount;
				//return false;
			}
			else
			{
				++radialWrongCount;
			}
	

			static Timer::Duration polygonalDuration;
			static Timer::Duration polygonalMaxDuration = std::chrono::milliseconds(std::numeric_limits<uint32_t>::min());
			static Timer::Duration polygonalMinDuration = std::chrono::milliseconds(std::numeric_limits<uint32_t>::max());
			static uint32_t polygonalWrongCount = 0;
			static uint32_t polygonalRightCount = 0;

			second.fire();
			polygonIntersect = details::polygonPolygonCollision(polygonOnePoints, polygonTwoPoints);
			second.stop();

			polygonalDuration += second.getMeasured();
			polygonalMaxDuration = std::max(polygonalMaxDuration, second.getMeasured());
			polygonalMinDuration = std::min(polygonalMinDuration, second.getMeasured());
			if (polygonIntersect)
			{
				++polygonalRightCount;
				//return false;
			}
			else
			{
				++polygonalWrongCount;
			}

			if (radialRightCount + radialWrongCount > 100'0000 / 2)
			{
				std::cout << "Radial results:"
					<< "\n\t Right count: " << radialRightCount
					<< "\n\t Wrong count: " << radialWrongCount
					<< "\n\t Min duration:" << std::chrono::duration_cast<std::chrono::microseconds>(radialMinDuration).count() << "ms"
					<< "\n\t Max duration:" << std::chrono::duration_cast<std::chrono::microseconds>(radialMaxDuration).count() << "ms"
					<< "\n\t Average duration:" << std::chrono::duration_cast<std::chrono::microseconds>(radialDuration).count() / float(radialRightCount + radialWrongCount) << "ms"
					<< "\n\t Total duration:" << std::chrono::duration_cast<std::chrono::microseconds>(radialDuration).count() << "ms"
					<< "\n\t Possible time saved: " << std::chrono::duration_cast<std::chrono::microseconds>(possibleSavedTimeWithRadial).count() << "ms"
					<< "\n\t Possible time wasted: " << std::chrono::duration_cast<std::chrono::microseconds>(possibleWastedTimeWithRadial).count() << "ms"
					<< "\n\n";
				std::cout << "Polygonal results:"
					<< "\n\t Right count: " << polygonalRightCount
					<< "\n\t Wrong count: " << polygonalWrongCount
					<< "\n\t Min duration:" << std::chrono::duration_cast<std::chrono::microseconds>(polygonalMinDuration).count() << "ms"
					<< "\n\t Max duration:" << std::chrono::duration_cast<std::chrono::microseconds>(polygonalMaxDuration).count() << "ms"
					<< "\n\t Average duration:" << std::chrono::duration_cast<std::chrono::microseconds>(polygonalDuration).count() / float(polygonalRightCount + polygonalWrongCount) << "ms"
					<< "\n\t Total duration:" << std::chrono::duration_cast<std::chrono::microseconds>(polygonalDuration).count() << "ms"
					<< "\n\t Possible time saved: " << std::chrono::duration_cast<std::chrono::microseconds>(possibleSavedTimeWithPolygonal).count() << "ms"
					<< "\n\t Possible time wasted: " << std::chrono::duration_cast<std::chrono::microseconds>(possibleWastedTimeWithPolygonal).count() << "ms"
					<< "\n\n";
				std::cout << "Triagonal results:"
					<< "\n\t Min duration:" << std::chrono::duration_cast<std::chrono::microseconds>(triagonalMinDuration).count() << "ms"
					<< "\n\t Max duration:" << std::chrono::duration_cast<std::chrono::microseconds>(triagonalMaxDuration).count() << "ms"
					<< "\n\t Average duration:" << std::chrono::duration_cast<std::chrono::microseconds>(triagonalDuration).count() / float(colCount) << "ms"
					<< "\n\t Total duration:" << std::chrono::duration_cast<std::chrono::microseconds>(triagonalDuration).count() << "ms"
					<< "\n\n";

				exit(0);
			}
		}


		Timer third(true);
		bool found = false;
		// +2 for triangle
		for (uint32_t firstIndex = 0; firstIndex + 2 < polygonOnePoints.size(); ++firstIndex)
		{
			const Triangle polygonOneTriangle =
			{ polygonOnePoints[firstIndex],  polygonOnePoints[firstIndex + 1] ,polygonOnePoints[firstIndex + 2] };

			// 2+ as well
			for (uint32_t secondIndex = 0; secondIndex + 2 < polygonTwoPoints.size(); ++secondIndex)
			{
				const Triangle polygonTwoTriangle =
				{ polygonTwoPoints[secondIndex],  polygonTwoPoints[secondIndex + 1] ,polygonTwoPoints[secondIndex + 2] };

				if (details::triangleTriangleCollision(polygonOneTriangle, polygonTwoTriangle))
				{
					found = true;
					break;
				}
			}

			if (found)
				break;
		}

		third.stop();
		if (!found)
		{
			if (circleIntersect)
				possibleSavedTimeWithRadial += third.getMeasured() - first.getMeasured();
			if (polygonIntersect)
				possibleSavedTimeWithPolygonal += third.getMeasured() - second.getMeasured();
		}
		if(found)
		{
			if (!circleIntersect)
				possibleWastedTimeWithRadial += third.getMeasured() - first.getMeasured();
			if (!polygonIntersect)
				possibleWastedTimeWithPolygonal += third.getMeasured() - second.getMeasured();
		}


		triagonalDuration += third.getMeasured();
		triagonalMaxDuration = std::max(triagonalMaxDuration, third.getMeasured());
		triagonalMinDuration = std::min(triagonalMinDuration, third.getMeasured());

		return found;
#else

		auto findCentreAndRadiusOfPoints = [](const Points& points)
		{
			float maxX = std::numeric_limits<float>::min();
			float minX = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();
			float minZ = std::numeric_limits<float>::max();

			Point average = {};
			for (const auto& p : points)
			{
				average += p;
				maxX = std::max(maxX, p.x);
				minX = std::min(minX, p.x);
				maxY = std::max(maxY, p.y);
				minY = std::min(minY, p.y);
				maxZ = std::max(maxZ, p.z);
				minZ = std::min(minZ, p.z);
			}
			average /= float(points.size());
			auto getGreater = [](float a, float b)
			{return a > b ? a : b; };

			float absX = getGreater(std::abs(minX - average.x), std::abs(maxX - average.x));
			float absY = getGreater(std::abs(minY - average.y), std::abs(maxY - average.y));
			float absZ = getGreater(std::abs(minZ - average.z), std::abs(maxZ - average.z));

			Point radiusPoint(absX, absY, absZ);

			return std::make_pair(average, glm::length(radiusPoint));
		};

		// can improve performance in several occasions idk tho,
		// premature optimalisation it is

		
		{
			const auto [c1, r1] = findCentreAndRadiusOfPoints(polygonOnePoints);
			const auto [c2, r2] = findCentreAndRadiusOfPoints(polygonTwoPoints);

			const float circleDistances = glm::length(c1 - c2);

			auto circleIntersect = circleDistances <= std::abs(r1 + r2);

			if (circleIntersect)
			{
				return false;
			}
		}

		// +2 for triangle
		for (uint32_t firstIndex = 0; firstIndex + 2 < polygonOnePoints.size(); ++firstIndex)
		{
			const Triangle polygonOneTriangle =
			{ polygonOnePoints[firstIndex],  polygonOnePoints[firstIndex + 1] ,polygonOnePoints[firstIndex + 2] };

			// 2+ as well
			for (uint32_t secondIndex = 0; secondIndex + 2 < polygonTwoPoints.size(); ++secondIndex)
			{
				const Triangle polygonTwoTriangle =
				{ polygonTwoPoints[secondIndex],  polygonTwoPoints[secondIndex + 1] ,polygonTwoPoints[secondIndex + 2] };

				if (details::triangleTriangleCollision(polygonOneTriangle, polygonTwoTriangle))
				{
					return true;
				}
			}
		}

		return false;
#endif
	}

}