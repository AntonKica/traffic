#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <array>

// Not in a namespace
/*
namespace BasicGeometry
{
	using Point = glm::vec3;
	using Points = std::vector<Point>;

	using LineSegment = std::array<Point, 2>;
	using Triangle = std::array<Point, 3>;
	using Rectangle = std::array<Point, 4>;
}
namespace BG = BasicGeometry;*/

using Point = glm::vec3;
using Points = std::vector<Point>;

using LineSegment = std::array<Point, 2>;
using LineSegments = std::vector<LineSegment>;

using Triangle = std::array<Point, 3>;
using Triangles = std::vector<Triangle>;

using Quadrangle = std::array<Point, 4>;
using Quadrangles = std::vector<Quadrangle>;

using Rectangle = std::array<Point, 4>;
using Rectangles = std::vector<Rectangle>;
