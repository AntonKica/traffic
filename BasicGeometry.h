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

	using Line = std::array<Point, 2>;
	using Triangle = std::array<Point, 3>;
	using Rectangle = std::array<Point, 4>;
}
namespace BG = BasicGeometry;*/

using Point = glm::vec3;
using Points = std::vector<Point>;

using Line = std::array<Point, 2>;
using Lines = std::vector<Line>;

using Triangle = std::array<Point, 3>;
using Triangles = std::vector<Triangle>;

using Rectangle = std::array<Point, 4>;
using Rectangles = std::vector<Rectangle>;
