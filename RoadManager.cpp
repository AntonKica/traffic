#include "RoadManager.h"
#include "GlobalObjects.h"
#include "SimpleCar.h"
#include <random>
#include <chrono>

Points generateArrow(Point tail, Point head)
{
	tail.y = 0.1;
	head.y = 0.1;
	glm::vec3 tailHeadDir = glm::normalize(head - tail);
	glm::vec3 headTailDir = glm::normalize(tail - head);

	const glm::vec3 vectorUp(0.0, 1.0, 0.0);

	glm::vec3 orthoVec = glm::normalize(glm::cross(tailHeadDir, vectorUp));
	glm::vec3 reverseOrthoVec = glm::normalize(glm::cross(headTailDir, vectorUp));

	const float tailWidth = 0.08f;
	Point tailLeft = tail - orthoVec * tailWidth;
	Point tailRight = tail + orthoVec * tailWidth;

	const float neckOffset = 0.30f;
	Point neckPosition = (head + headTailDir * neckOffset);
	Point neckLeft = neckPosition - reverseOrthoVec * tailWidth;
	Point neckRight = neckPosition + reverseOrthoVec * tailWidth;

	const float headWidth = 0.20f;
	Point thickerNeckLeft = neckPosition - reverseOrthoVec * headWidth;
	Point thickerNeckRight = neckPosition + reverseOrthoVec * headWidth;

	Points retPoints = {
		tailLeft, tailRight, neckLeft,
		neckLeft, neckRight, tailLeft,
		neckPosition, thickerNeckLeft, head,
		neckPosition, head, thickerNeckRight
	};

	return retPoints;
}

Points PathVisualizer::generateArrows()
{
	/*Points arrowsVertices;
	for (const auto& road : roadManager->roads)
	{
		for (int i = 0; i < road.getParameters().laneCount; ++i)
		{
			//if (i)
			//	break;
			auto path = road.m_paths[i];
			for (int i = 0; i < path.size() - 1; ++i)
			{
				Points arrowPoints = generateArrow(path[i], path[i + 1]);
				arrowsVertices.insert(arrowsVertices.end(), arrowPoints.begin(), arrowPoints.end());
			}
		}
	}

	return arrowsVertices;*/
	return {};
}

void PathVisualizer::updateVisuals()
{
	/*if (roadManager->somethingChanged())
	{
		Info::DrawInfo dInfo{};
		dInfo.polygon = VK_POLYGON_MODE_FILL;
		dInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


		VD::PositionVertices vertices;

		Points arrowVertices = generateArrows();
		vertices.resize(arrowVertices.size());

		for (int i = 0;  i < arrowVertices.size(); ++i)
			vertices[i] = arrowVertices[i];

		const glm::vec4 redColor(0.44, 0.18, 0.21, 1.0);
		VD::ColorVertices colors(vertices.size(), redColor);

		Mesh mesh;
		mesh.vertices.positions = vertices;
		mesh.vertices.colors = colors;

		Model model;
		model.meshes.push_back(mesh);

		Info::ModelInfo mInfo{};
		mInfo.model = &model;

		Info::GraphicsComponentCreateInfo gInfo{};
		gInfo.drawInfo = &dInfo;
		gInfo.modelInfo = &mInfo;

		graphics.recreateGraphicsComponent(gInfo);
		graphics.setActive(true);
		graphics.setTransparency(0.5);
	}
	*/
}
