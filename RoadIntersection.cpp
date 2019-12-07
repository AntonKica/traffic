#include "RoadIntersection.h"
#include "Utilities.h"
#include <glm/gtx/string_cast.hpp>

inline VD::Indices createPseudoTriangleFanIndices(const Points& points)
{
	VD::Indices indices;

	for (int i = 0; i <= points.size(); ++i)
	{
		std::vector<uint32_t> fanIndices
		{
			0, 
			static_cast<uint32_t>(i), 
			static_cast<uint32_t>((i + 1) % points.size())
		};

		indices.insert(indices.end(), fanIndices.begin(), fanIndices.end());
	}

	return indices;
}
void RoadIntersection::construct(std::array<Road*, 3> roads, Point intersectionPoint)
{
	m_position = intersectionPoint;
	centre = glm::vec3(0);
	width = roads[0]->getRoadParameters().width;


	float requiredDistance = width / 2.0f;
	for (auto& road : roads)
	{
		auto parameters = road->getRoadParameters();
		connectionPoints.push_back(road->shorten(intersectionPoint, requiredDistance) - m_position);
	}

	Points sidePoints;
	Points commonPoints;
	for (int i = 0; i < roads.size(); ++i)
	{
		auto& cps = connectionPoints;
		//cps.size() == 3
		// find neigbbour axes
		const auto [first, second] = findTwoClosestPoints({ cps[(i + 1) % cps.size()],  cps[(i + 2) % cps.size()] }, cps[i]);

		auto& currentPoint = cps[i];
		glm::vec3 dirCurrentToCentre = glm::normalize(centre - currentPoint);
		glm::vec3 dirCentreToFirst = glm::normalize(first - centre);
		glm::vec3 dirCentreToSecond = glm::normalize(second - centre);


		{
			const auto [left, right] = getSidePoints(dirCurrentToCentre, dirCurrentToCentre, currentPoint, currentPoint, centre, width);

			sidePoints.insert(sidePoints.end(), { left, right });
		}
		
		{
			const auto [left, _] = getSidePoints(dirCurrentToCentre, dirCentreToFirst, centre, first, first, width);
			const auto [_2, right] = getSidePoints(dirCurrentToCentre, dirCentreToSecond, centre, second, second, width);

			commonPoints.insert(commonPoints.end(), { left, right });
		}
	}
	for (const auto& glm : sidePoints)
	{
		std::cout << glm::to_string(glm) << '\n';
	}
	//remove uniqes
	auto overWriteIter = commonPoints.begin();
	auto currentIter = commonPoints.begin();
	for (const auto& point : commonPoints)
	{
		auto count = std::count(currentIter++, std::end(commonPoints), point);
		if (count > 1)
			*(overWriteIter++) = point;
	}
	// check befrore remove, shouldnt happen tho
	if(overWriteIter != commonPoints.end())
		commonPoints.erase(overWriteIter + 1, commonPoints.end());

	size_t totalPoints = sidePoints.size() + commonPoints.size();
	VD::PositionVertices shapePoints(totalPoints);
	auto pointsIter = shapePoints.begin();
	for (int sideIndex = 1, commonIndex = 0; sideIndex < sidePoints.size(); sideIndex +=2, ++commonIndex)
	{
		// common, side, side, common
		*(pointsIter + 1) = sidePoints[sideIndex - 1];
		*(pointsIter + 2) = sidePoints[sideIndex];

		// in case there are odd common points
		*(pointsIter) = commonPoints[commonIndex];

		// vertices added
		std::advance(pointsIter, 3);
	}



	VD::Indices indices = createPseudoTriangleFanIndices(shapePoints);
	Mesh mesh;
	mesh.vertices.positions = shapePoints;
	mesh.indices = indices;

	Model model;
	model.meshes.push_back(mesh);

	Info::ModelInfo mInfo;
	mInfo.model = &model;

	setupModel(mInfo, true);
}
