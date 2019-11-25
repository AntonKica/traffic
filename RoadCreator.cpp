#include "RoadCreator.h"
#include "GlobalObjects.h"
#include <glm/gtx/perpendicular.hpp>

constexpr int pointForLine = 2;
std::vector<Point> genTriangles(const std::vector<Point>& points)
{
	if (points.size() <= 3)
		return points;

	//formula
	uint32_t triangleCount = 3 * (points.size() - 2);
	std::vector<Point> shapeTrianglesPoints(triangleCount);
	auto insertIter = shapeTrianglesPoints.begin();

	for (int i = 2; i < points.size(); ++i)
	{
		// copy two previsous points and current
		insertIter = shapeTrianglesPoints.insert(std::end(shapeTrianglesPoints), insertIter + i - 2, insertIter + i);
	}

	return shapeTrianglesPoints;
}

std::vector<Point> buildSimpleShape(const std::vector<Point>& points, int width)
{
	if (points.size() < 2)
		return points;

	std::vector<Point> leftPoints;
	std::vector<Point> rightPoints;
	glm::vec3 normal = glm::normalize(points[0] - points[1]);
	for (int i =0; i < points.size(); ++i)
	{	
		//update normal
		if(i + 1 < points.size())
			normal = glm::normalize(points[i + 1] - points[i]);

		std::cout << i << ' ' <<glm::to_string(normal) << '\n';
		const auto& curPoint = points[i];

		const auto perpVec = glm::normalize(glm::perp(curPoint, normal));

		Point right = curPoint + perpVec * (width / 2.0f);
		Point left = curPoint - perpVec * (width / 2.0f);

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

std::vector<Point> generateCurveFromThreePoints(const std::array<Point, 3>& points)
{
	if (points[0] != points[1] && points[1] != points[2])
		std::cout << '\n';

	const float precision = 5;
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
void RoadCreator::setPoint()
{
	auto pointPos = App::Scene.m_simArea.getSelectedPointPos();

	if (pointPos)
	{
		currentPoints.push_back(pointPos.value());

		if (currentPoints.size() == 3)
			createRoadFromCurrent();

		visualizer.setPoints(currentPoints);
	}
}

void RoadCreator::createRoadFromCurrent()
{
	Road newRoad;
	newRoad.createGraphics(std::vector(std::begin(currentPoints), std::end(currentPoints)));

	tempRoads.push_back(newRoad);

	currentPoints.clear();
}

void RoadCreator::update()
{
	visualizer.update();
}

void RoadCreator::clickEvent()
{
	setPoint();
}

Road::Road()
{
}

void Road::createGraphics(const std::vector<Point>& pts)
{
	GO::TypedVertices typedVts;
	auto& [type, vvts] = typedVts;

	type = GO::VertexType::COLORED;
	glm::vec3 color = glm::vec3(1.0, 0.2, 0.1);
	auto transformPointToVtx = [&color](const Point& pt)
	{
		GO::VariantVertex vvtx;
		vvtx.coloredVertex.position = pt;
		vvtx.coloredVertex.color = color;
		return vvtx;
	};
	std::vector<Point> drawPoints;
	if (pts.size() == 2)
	{
		drawPoints = buildSimpleShape(pts, 1);
	}
	else if(pts.size() == 3)
	{
		drawPoints = buildSimpleShape(generateCurveFromThreePoints({ pts[0], pts[1], pts[2] }), 1);
	}
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

	graphics = App::Scene.vulkanBase.createGrahicsComponent(gInfo);
	graphics.setActive(true);
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

void CreatorVisualizer::setPoints(const std::vector<Point>& points)
{
	pointToDraw = points;
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
			drawPoints = buildSimpleShape(points, 1.0);
		}
		else if (points.size() == 3)
		{
			auto temp = generateCurveFromThreePoints({ points[0], points[1], points[2] });
			drawPoints = buildSimpleShape(temp, 1);
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
