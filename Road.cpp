#include "Road.h"
#include "DataManager.h"
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


std::pair<GO::TypedVertices, GO::Indices> createSimpleShape(const Points& points, int width)
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
	parameters.laneCount = laneCount;
	parameters.width = width;
	parameters.texture = texture;

	const auto [typedVertices, indices] = createSimpleShape(parameters.axis, parameters.width);

	Info::ModelInfo modelInfo;
	modelInfo.vertices = &typedVertices;
	modelInfo.indices = &indices;
	modelInfo.texturePath = parameters.texture;

	setupModel(modelInfo, true);
}
