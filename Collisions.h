#pragma once
#include "BasicGeometry.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>

namespace bg = boost::geometry;

#include <glm/glm.hpp>
BOOST_GEOMETRY_REGISTER_POINT_3D(glm::vec3, float, bg::cs::cartesian, x, y, z)
BOOST_GEOMETRY_REGISTER_POINT_2D(glm::vec2, float, bg::cs::cartesian, x, y)


namespace CollisionLiterals
{
	using PointXZ = boost::geometry::model::d2::point_xy<float>;
	//using PolygonXZ = boost::geometry::model::polygon<PointXZ>;
	//using PointXZ = glm::vec2;
	using PolygonXZ = boost::geometry::model::polygon<PointXZ>;
}
namespace CL = CollisionLiterals;

namespace Collision
{
	namespace details
	{
		static CL::PointXZ createPointXZFromPoint(const Point& point)
		{
			return { point.x, point.z };
		}

		static CL::PolygonXZ createXZPolygonFromPoints(const Points& points)
		{
			std::vector<CL::PointXZ> XZPoints(points.size());
			std::transform(std::begin(points), std::end(points), std::begin(XZPoints),
				[&](const Point& point)
				{
					return createPointXZFromPoint(point);
				});
			CL::PolygonXZ newPolygon;
			boost::geometry::assign_points(newPolygon, XZPoints);
			boost::geometry::correct(newPolygon);
			return newPolygon;
		}
	}

	static bool polygonPolygon(const CL::PolygonXZ& polygonOne, const CL::PolygonXZ& polygonTwo)
	{
		return boost::geometry::intersects(polygonOne, polygonTwo);
	}

	static bool pointPolygon(const CL::PointXZ& point, const CL::PolygonXZ& polygon)
	{
		return boost::geometry::within(point, polygon);
	}

	static bool XZPointsPoints(const Points& pointsOne, const Points& pointsTwo)
	{
		auto polyOne = details::createXZPolygonFromPoints(pointsOne);
		auto polyTwo = details::createXZPolygonFromPoints(pointsTwo);

		return polygonPolygon(polyOne, polyTwo);
	}

	static bool XZPointPoints(const Point& point, const Points& points)
	{
		return pointPolygon(details::createPointXZFromPoint(point), details::createXZPolygonFromPoints(points));
	}
}