#include "ParkingLot.h"
#include <numeric>

void ParkingLot::construct(std::array<DirectedEdge, 2> creationEdges)
{
	if (creationEdges[0].position != creationEdges[1].position)
	{
		enableComponents();

		auto edgePoints = generateEdgePoints(creationEdges);

		// adjust them and position
		{
			glm::vec3 sumVec = std::accumulate(std::begin(edgePoints), std::end(edgePoints), glm::vec3{});
			auto newPosition = sumVec / 4.0f;

			for (auto& edgePoint : edgePoints)
				edgePoint -= newPosition;

			setPosition(newPosition);
		}


		Mesh mesh;
		mesh.vertices.positions = VD::PositionVertices(std::begin(edgePoints), std::end(edgePoints));
		mesh.vertices.colors = VD::ColorVertices(edgePoints.size(), glm::vec4(0.1, 0.6, 0.8, 1.0));
		mesh.indices = { 0, 1, 2, 1, 2, 3 };

		Model model;
		model.meshes.emplace_back(mesh);

		Info::ModelInfo mInfo;
		mInfo.model = &model;


		setupModel(mInfo, true);
	}
	else
	{
		disableComponents();
	}
}

std::array<Point, 4> ParkingLot::generateEdgePoints(const std::array<DirectedEdge, 2>& creationEdges)
{
	std::array<Point, 4> edgePoints;
	edgePoints[0] = creationEdges[0].position;
	edgePoints[1] = creationEdges[0].position + creationEdges[0].direction * 3.0f;
	edgePoints[2] = creationEdges[1].position;
	edgePoints[3] = creationEdges[1].position + creationEdges[1].direction * 3.0f;

	return edgePoints;
}
