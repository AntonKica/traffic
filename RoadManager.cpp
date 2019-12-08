#include "RoadManager.h"
#include "GlobalObjects.h"
#include "SimpleCar.h"
#include <random>
#include <chrono>

RoadManager::RoadManager()
{
	roadCreator.initialize(this);
	pathVisualizer.initialize(this);
}


void RoadManager::update(float deltaTime)
{
	updateSelectedRoads();
	roadCreator.update();
	pathVisualizer.updateVisuals();

	auto cursor = App::Scene.m_simArea.getMousePosition();

	/*static std::vector<SimpleCar> simpleCars;
	if (changedRoads)
	{
		// recreateCars
		simpleCars.clear();
		for (const auto& road : roads)
		{
			static std::mt19937_64 engine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
			std::uniform_int_distribution<int> distributor(0, road.getLaneCount() - 1);

			SimpleCar car;
			car.create(road.getPath(distributor(engine)));

			simpleCars.push_back(car);
		}
	}

	for (auto& car : simpleCars)
		car.update(deltaTime);*/
	
	changedRoads = false;
}

void RoadManager::addRoad(Road road)
{
	roads.emplace_back(road);
	changedRoads = true;
}

void RoadManager::addRoads(const std::vector<Road>& insertRoads)
{
	roads.insert(std::end(roads), std::begin(insertRoads), std::end(insertRoads));
}

void RoadManager::removeRoad(Road* toRemove)
{
	std::list<Road>::iterator removeIter = roads.begin();
	for (removeIter = roads.begin(); removeIter != roads.end(); ++removeIter)
	{
		if (&(*removeIter) == toRemove)
			break;
	}

	roads.erase(removeIter);
	changedRoads = true;
}

void RoadManager::removeRoads(std::vector<Road*> toRemove)
{
	for (auto& road : toRemove)
		removeRoad(road);
	changedRoads = true;
}

bool RoadManager::somethingChanged() const
{
	return changedRoads;
}

std::optional<Road*> RoadManager::getSelectedRoad()
{
	return selectedRoad;
}



void RoadManager::updateSelectedRoads()
{
	auto cursor = App::Scene.m_simArea.getMousePosition();
	selectedRoad.reset();

	const glm::vec4 green = glm::vec4(0.47, 0.98, 0.0, 1.0);
	if (cursor)
	{
		for (auto& road : roads)
		{
			if (road.sitsOnRoad(cursor.value()))
			{
				selectedRoad = &road;
				road.m_graphicsComponent.setTint(green);
				//break;
			}
			else
			{
				road.m_graphicsComponent.setTint(glm::vec4());
			}
		}
	}
}

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
	Points arrowsVertices;
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

	return arrowsVertices;
}

void PathVisualizer::initialize(RoadManager* roadManager)
{
	this->roadManager = roadManager;
}

void PathVisualizer::updateVisuals()
{
	if (roadManager->somethingChanged())
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
}
