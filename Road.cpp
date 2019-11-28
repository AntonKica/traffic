#include "Road.h"
#include "DataManager.h"

#include <glm/gtx/string_cast.hpp>
#include <numeric>


float calculateLength(const Points& points)
{
	float length = 0;

	auto pointIterator = points.begin();
	while (pointIterator != points.end() - 1)
	{
		length += glm::length(*pointIterator - *(pointIterator + 1));
		++pointIterator;
	}

	return length;
}

glm::vec3 getAveratePosition(const Points& points)
{
	glm::vec3 averatePosition = std::accumulate(std::begin(points), std::end(points), glm::vec3());

	return averatePosition;
}


bool polygonPointCollision(const Points& polygon, const Point& point)
{
	bool collision = false;
	glm::vec3 hitPos;
	for (auto vert = polygon.begin(); vert != polygon.end(); ++vert)
	{
		auto nextVert = (vert + 1) ==
			polygon.end() ? polygon.begin() : vert + 1;

		// z test
		if ( (vert->z >= point.z && nextVert->z < point.z) || (vert->z < point.z && nextVert->z >= point.z))
		{
			if (point.x < (nextVert->x - vert->x) * (point.z - vert->z) / (nextVert->z - vert->z) + vert->x)
			{
				collision = !collision;
				if (collision)
					hitPos = point;
			}
		}
	}
	if (collision)
		std::cout << "Hitted " << glm::to_string(hitPos) << '\n';

	return collision;
}

bool polygonPointCollision(const Points& vertices, float px, float py)
{
	bool collision = false;

	// go through each of the vertices, plus
	// the next vertex in the list
	int next = 0;
	for (int current = 0; current < vertices.size(); current++) {

		// get next vertex in list
		// if we've hit the end, wrap around to 0
		next = current + 1;
		if (next == vertices.size()) next = 0;

		// get the PVectors at our current position
		// this makes our if statement a little cleaner
		Point vc = vertices[current];    // c for "current"
		Point vn = vertices[next];       // n for "next"

		// compare position, flip 'collision' variable
		// back and forth
		if (((vc.z >= py && vn.z < py) || (vc.z < py && vn.z >= py)) &&
			(px < (vn.x - vc.x) * (py - vc.z) / (vn.z - vc.z) + vc.x)) 
		{
			collision = !collision;
		}
	}
	return collision;
}

bool polygonPolygonCollision(const Points& polygonOne, const Points& polygonTwo)
{
	bool collided = false;
	for (const auto& point : polygonOne)
	{
		if (polygonPointCollision(polygonTwo, point))
		{
			collided = true;
			break;
		}
	}

	return collided;
}

Points createShape(const Points& points, int width)
{
	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	Points leftPoints;
	Points rightPoints;
	glm::vec3 dirVec;
	for (int i = 0; i < points.size(); ++i)
	{
		// vertices
		{
			const auto& curPoint = points[i];
			//update direction
			if (i + 1 < points.size())
				dirVec = glm::normalize(points[i + 1] - points[i]);

			glm::vec3 rightVec = glm::normalize(glm::cross(dirVec, vectorUp));

			Point right = curPoint + rightVec * (width / 2.0f);
			Point left = curPoint - rightVec * (width / 2.0f);

			leftPoints.emplace_back(left);
			rightPoints.emplace_back(right);
		}
	}
	Points shapePoints;
	shapePoints.insert(std::end(shapePoints), std::begin(leftPoints), std::end(leftPoints));
	shapePoints.insert(std::end(shapePoints), std::rbegin(rightPoints), std::rend(rightPoints));

	return shapePoints;
}

std::pair<GO::TypedVertices, GO::Indices> createTexturedShape(const Points& points, int width)
{
	GO::Indices indices;
	GO::TypedVertices typedVts;
	auto& [type, vertices] = typedVts;
	type = GO::VertexType::TEXTURED;

	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	float shapeLength = calculateLength(points);
	float textureDistance = 0;

	glm::vec3 dirVec;
	for (int i = 0; i < points.size(); ++i)
	{
		// vertices
		std::array<GO::VariantVertex, 2> variantVertex;
		{
			const auto& curPoint = points[i];
			//update direction
			if (i + 1 < points.size())
				dirVec = glm::normalize(points[i + 1] - points[i]);

			glm::vec3 rightVec = glm::normalize(glm::cross(dirVec, vectorUp));

			Point right = curPoint + rightVec * (width / 2.0f);
			Point left = curPoint - rightVec * (width / 2.0f);
			variantVertex[0].texturedVertex.position = left;
			variantVertex[1].texturedVertex.position = right;
		}
		// textures
		{
			if (i != 0)
				textureDistance += glm::length(points[i - 1] - points[i]);

			glm::vec2 rightCoord = glm::vec2(1, textureDistance);
			glm::vec2 leftCoord = glm::vec2(0, textureDistance);

			variantVertex[0].texturedVertex.texCoord = leftCoord;
			variantVertex[1].texturedVertex.texCoord = rightCoord;
		}
		vertices.insert(vertices.end(), { variantVertex[0], variantVertex[1] });
	}

	// indices
	for (int i = 0; i < vertices.size(); ++i)
	{
		if (i > 1)
		{
			std::array<uint32_t, 3> triplets = { i - 2, i - 1, i };
			indices.insert(std::end(indices), std::begin(triplets), std::end(triplets));
		}
	}

	return std::make_pair(typedVts, indices);
}

std::vector<Point> createOutline(const std::vector<Point>& points, float outlineSize)
{
	if (points.size() < 2)
		return points;

	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	std::vector<Point> leftPoints;
	std::vector<Point> rightPoints;

	glm::vec3 dirVec;
	for (int i = 0; i < points.size(); ++i)
	{
			//update normal
		if (i + 1 < points.size())
			dirVec = glm::normalize(points[i + 1] - points[i]);

		glm::vec3 rightVec = glm::normalize(glm::cross(dirVec, vectorUp));

		const auto& curPoint = points[i];

		Point right = curPoint + rightVec * (0.5f);
		Point left = curPoint - rightVec * (0.5f);

			// dont duplicate first and last
		if (i == 0 || i == points.size() - 1)
		{
			leftPoints.emplace_back(left);
			rightPoints.emplace_back(right);
		}
		else
		{
			leftPoints.insert(leftPoints.end(), { left, left });
			rightPoints.insert(rightPoints.end(), { right, right });
		}
	}
	std::vector<Point> shapePoints(leftPoints.size() + rightPoints.size());
	auto insertIt = std::copy(std::begin(leftPoints), std::end(leftPoints), shapePoints.begin());
	insertIt = std::copy(std::begin(rightPoints), std::end(rightPoints), insertIt);

	std::vector<Point> outline(shapePoints.size());
	auto outIt = outline.begin();
	for (auto& point : shapePoints)
		*outIt++ = point * outlineSize;
	
	return outline;
}


Points Road::createLocalPoints(const Points& points, const glm::vec3& position)
{
	Points localPoints(points.size());
	std::transform(std::begin(points), std::end(points), std::begin(localPoints),
		[&position](const Point& point)
		{
			return point - position;
		});

	return localPoints;
}


void Road::construct(const Points& points, uint32_t laneCount, float width, std::string texture)
{
	m_position = getAveratePosition(points);

	parameters.axis = createLocalPoints(points, m_position);
	parameters.model = createShape(points, width);
	parameters.laneCount = laneCount;
	parameters.width = width;
	parameters.texture = texture;

	const auto [typedVertices, indices] = createTexturedShape(parameters.axis, parameters.width);

	Info::ModelInfo modelInfo;
	modelInfo.vertices = &typedVertices;
	modelInfo.indices = &indices;
	modelInfo.texturePath = parameters.texture;

	setupModel(modelInfo, true);
}

Road::RoadParameters Road::getRoadParameters() const
{
	return parameters;
}
