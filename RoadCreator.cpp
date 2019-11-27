#include "RoadCreator.h"
#include "RoadManager.h"

#include "GlobalObjects.h"
#include <glm/gtx/perpendicular.hpp>
#include <numeric>

constexpr int pointForLine = 2;
std::vector<Point> genTriangles(const std::vector<Point>& points)
{
	if (points.size() <= 3)
		return points;

	//formula
	std::vector<Point> shapeTrianglesPoints;

	// duplicate last two points
	for (int i = 0; i + 3 <= points.size(); ++i)
	{
		auto copyPos = std::begin(points) + i;
		std::copy(copyPos, copyPos + 3, std::back_inserter(shapeTrianglesPoints));
	}

	return shapeTrianglesPoints;
}

std::vector<Point> buildSimpleShapeOutline(const std::vector<Point>& points, int width)
{
	if (points.size() < 2)
		return points;

	const glm::vec3 vectorUp(0.0f, 1.0f, 0.0f);

	std::vector<Point> leftPoints;
	std::vector<Point> rightPoints;

	glm::vec3 dirVec;
	for (int i =0; i < points.size(); ++i)
	{	
		//update normal
		if (i + 1 < points.size())
			dirVec = glm::normalize(points[i + 1] - points[i]);

		glm::vec3 rightVec = glm::normalize(glm::cross(dirVec, vectorUp));

		const auto& curPoint = points[i];

		Point right = curPoint + rightVec * (width / 2.0f);
		Point left = curPoint - rightVec * (width / 2.0f);

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

	return shapePoints;
}
struct MinMaxCoords
{
	float minX = std::numeric_limits<float>::max(), maxX =  std::numeric_limits<float>::min();
	float minY = std::numeric_limits<float>::max(), maxY =  std::numeric_limits<float>::min();
	float minZ = std::numeric_limits<float>::max(), maxZ =  std::numeric_limits<float>::min();
};

MinMaxCoords getMinMaxCoordsFromPoints(const std::vector<Point>& points)
{
	MinMaxCoords minMax{};

	auto updateMinMax = [](float& min, float& max, const float& val)
	{
		min = std::min(min, val);
		max = std::max(max, val);
	};

	for (const auto& point : points)
	{
		updateMinMax(minMax.minX, minMax.maxX, point.x);
		updateMinMax(minMax.minY, minMax.maxY, point.y);
		updateMinMax(minMax.minZ, minMax.maxZ, point.z);
	}

	return minMax;
}

std::vector<Point> generateCurveFromThreePoints(const std::array<Point, 3>& points)
{
	// on same line
	if (points[0] == points[1] && points[1] == points[2])
		return std::vector(points.begin(), points.end());

	const float precision = 20;
	std::vector<Point> curvePoints(precision + 1);

	const glm::vec3 firstDir = glm::normalize(points[1] - points[0]);
	const float firstLength = glm::length(points[1] - points[0]);
	const glm::vec3 secondtDir = glm::normalize(points[2] - points[1]);
	const float secondtLenght = glm::length(points[2] - points[1]);

	for (int i = 0; i <= precision; ++i)
	{
		float distanceInPercentage = i / precision;

		const float firstMagnitude = 1.0 - distanceInPercentage;
		const float firstDistance = std::lerp(0, firstLength, distanceInPercentage);
		const glm::vec3 firstPoint = (points[0] + firstDir * firstDistance) * firstMagnitude;
		const float secondMagnitude = distanceInPercentage;
		const float secondDistance = std::lerp(0, secondtLenght, distanceInPercentage);
		const glm::vec3 secondPoint = (points[1] + secondtDir * secondDistance) * secondMagnitude;

		const glm::vec3 curvePoint = firstPoint + secondPoint;
		curvePoints[i] = curvePoint;
	}

	return curvePoints;
}
void RoadCreator::setupPrototypes()
{
	hardcodedRoadPrototypes[0] = { "Basic 2-lane road", 2, 1.0f, "resources/materials/road.png" };
	hardcodedRoadPrototypes[1] = { "Basic 4-lane road", 4, 2.0f, "resources/materials/road2.png" };
	currentPrototypeID = 0;
}
void RoadCreator::setPoint()
{
	auto pointPos = App::Scene.m_simArea.getSelectedPointPos();

	if (pointPos)
	{
		currentPoints.push_back(pointPos.value());

		createRoadIfPossible();

		visualizer.setDraw(currentPoints, hardcodedRoadPrototypes[currentPrototypeID].width);
	}
}

void RoadCreator::createRoadIfPossible()
{
	// do not construct from one poin
	if (currentPoints.size() <= 1)
		return;

	Points creationPoints;
	if (creatorMode == Mode::STRAIGHT_LINE && currentPoints.size() == 2)
	{
		creationPoints = currentPoints;
	}
	else if (creatorMode == Mode::CURVED_LINE && currentPoints.size() == 3)
	{
		std::array<Point, 3> curvePoints = { currentPoints[0], currentPoints[1], currentPoints[2] };
		creationPoints = generateCurveFromThreePoints(curvePoints);
	}
	if (creationPoints.size())
	{
		const auto& currentPrototype = hardcodedRoadPrototypes[currentPrototypeID];
		Road newRoad;
		newRoad.construct(creationPoints, currentPrototype.laneCount, currentPrototype.width, currentPrototype.texture);

		roadManager->addRoad(newRoad);

		currentPoints.clear();
	}
}

void RoadCreator::initialize(RoadManager* roadManager)
{
	this->roadManager = roadManager;
	setupPrototypes();
}

void RoadCreator::update()
{
	visualizer.update();
}

void RoadCreator::clickEvent()
{
	setPoint();
}


std::vector<std::string> RoadCreator::getRoadNames() const
{
	auto extractFun = [](const std::map<int, ::Prototypes>& prototypes)
	{
		std::vector<std::string> names;
		for (const auto& [id, prototype] : prototypes)
			names.push_back(prototype.name);

		return names;
	};

	static std::vector<std::string> names = extractFun(hardcodedRoadPrototypes);
	
	return names;
}

void RoadCreator::setMode(int mode)
{
	creatorMode = static_cast<Mode>(mode);
}

void RoadCreator::setPrototype(int prototype)
{
	currentPrototypeID = prototype;
}

void CreatorVisualizer::update()
{
	auto mousePos = App::Scene.m_simArea.getSelectedPointPos();

	if (mousePos)
	{
		mousePoint = mousePos.value();
	}

	updateGraphics();
}

void CreatorVisualizer::setDraw(const std::vector<Point>& points, float width)
{
	pointToDraw = points;
	this->width = width;
}

std::vector<glm::vec3> CreatorVisualizer::generateLines()
{
	std::vector<Point> points;
	for (const auto& point : pointToDraw)
		points.push_back(point);

	if(mousePoint)
		points.push_back(mousePoint.value());

	return points;
}

std::vector<glm::vec3> CreatorVisualizer::generatePoints()
{
	std::vector<Point> genPoints;
	auto generateCircle = []()
	{
		const int step = 45;
		std::vector<Point> points(3* 360 / step);
		auto ptsIt = points.begin();
		for (float i = 360; i > 0; i -= step)
		{
			Point point1;
			point1.x = cos(glm::radians(i));
			point1.y = 0;
			point1.z = sin(glm::radians(i));
			point1 *= 0.08;

			Point point2;
			point2.x = cos(glm::radians(i + step));
			point2.y = 0;
			point2.z = sin(glm::radians(i + step));
			point2 *= 0.08;

			Point point3 = glm::vec3(0);

			std::array<Point, 3> pts {point1, point2, point3 };
			ptsIt = std::copy(std::begin(pts), std::end(pts), ptsIt);
		}
		return points;
	};
		

	static const auto circePoints = generateCircle();

	auto ptsToDraw = pointToDraw;
	if (mousePoint)
		ptsToDraw.push_back(mousePoint.value());

	genPoints.resize(ptsToDraw.size() * circePoints.size());
	auto genIt = genPoints.begin();
	for (const auto point : ptsToDraw)
	{
		genIt = std::transform(std::begin(circePoints), std::end(circePoints), genIt,
			[&point](const Point& circlePoint)
			{
				return point + circlePoint;
			});
	}

	return genPoints;
}

void CreatorVisualizer::updateGraphics()
{
	glm::vec3 color = glm::vec3(0.1, 0.2, 1.0);
	auto transformPointToVtx = [&color](const Point& pt)
	{
		GO::VariantVertex vvtx;
		vvtx.coloredVertex.position = pt;
		vvtx.coloredVertex.color = color;
		return vvtx;
	};

	GO::TypedVertices typedVts;
	auto& [type, vvts] = typedVts;

	type = GO::VertexType::COLORED;

	auto points = pointToDraw;
	if (mousePoint)
		points.push_back(mousePoint.value());

	if (points.size() > 1)
	{
		std::vector<Point> drawPoints;

		if (points.size() == 2)
		{
			drawPoints = buildSimpleShapeOutline(points, width);
		}
		else if (points.size() == 3)
		{
			auto temp = generateCurveFromThreePoints({ points[0], points[1], points[2] });
			drawPoints = buildSimpleShapeOutline(temp, width);
		}

		lineGraphics.setActive(true);
		std::transform(std::begin(drawPoints), std::end(drawPoints), std::back_inserter(vvts), transformPointToVtx);

		Info::DrawInfo dInfo;
		dInfo.lineWidth = 2.0f;
		dInfo.polygon = VK_POLYGON_MODE_LINE;
		dInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		Info::ModelInfo mInfo;
		mInfo.vertices = &typedVts;

		Info::GraphicsComponentCreateInfo gInfo;
		gInfo.drawInfo = &dInfo;
		gInfo.modelInfo = &mInfo;

		lineGraphics.recreateGraphicsComponent(gInfo);
	}
	else
	{
		lineGraphics.setActive(false);
	}

	points = generatePoints();
	if (points.size() > 0)
	{
		pointGraphics.setActive(true);
		color = glm::vec3(0.1, 0.8, 0.1);

		Info::DrawInfo dInfo;
		dInfo.lineWidth = 1.0f;
		dInfo.polygon = VK_POLYGON_MODE_FILL;
		dInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		vvts.clear();
		std::transform(std::begin(points), std::end(points), std::back_inserter(vvts), transformPointToVtx);

		Info::ModelInfo mInfo;
		mInfo.vertices = &typedVts;

		Info::GraphicsComponentCreateInfo gInfo;
		gInfo.drawInfo = &dInfo;
		gInfo.modelInfo = &mInfo;

		pointGraphics.recreateGraphicsComponent(gInfo);
	}
	else
	{
		pointGraphics.setActive(false);
	}
}
