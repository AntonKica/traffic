#include "BuildingCreator.h"
#include "ObjectManager.h"
#include "SimulationArea.h"

#include "GlobalObjects.h"
#include "House.h"

/*
namespace
{
	struct Rect
	{
		glm::vec3 p1, p2, p3, p4;
	};
	Rect deduceRectFromModel(const Model& model)
	{
		glm::vec3 max;
		glm::vec3 min;
		max.x = std::numeric_limits<float>::min();
		max.y = std::numeric_limits<float>::min();
		max.z = std::numeric_limits<float>::min();
		min.y= std::numeric_limits<float>::max();
		min.x = std::numeric_limits<float>::max();
		min.z = std::numeric_limits<float>::max();

		glm::vec3 average = {};
		uint32_t totalPoints = 0;
		for (const auto& mesh : model.meshes)
		{
			totalPoints += mesh.vertices.positions.size();
			for (const auto& p : mesh.vertices.positions)
			{
				average += p;
				max.x = std::max(max.x, p.x);
				max.y = std::max(max.y, p.y);
				max.z = std::max(max.z, p.z);
				min.x = std::min(min.x, p.x);
				min.y = std::min(min.y, p.y);
				min.z = std::min(min.z, p.z);
			}
		}
		average /= float(totalPoints);
		auto getGreater = [](float a, float b)
		{return a > b ? a : b; };

		float absX = getGreater(std::abs(min.x - average.x), std::abs(max.x- average.x));
		float absZ = getGreater(std::abs(min.z - average.z), std::abs(max.z - average.z));

		Rect rect = {
			{ -absX, 0.0f, -absZ},
			{ absX, 0.0f, -absZ},
			{ -absX, 0.0f, absZ},
			{ absX, 0.0f, absZ}
		};

		return rect;
	}
}

PlacementRectangle::PlacementRectangle(SimulationArea* pSimulationArea)
	:m_pSimulationArea(pSimulationArea)
{
	auto& pComp = getPhysicsComponent();
	pComp.setOtherCollisionTags({ "ROAD" });

	setActive(false);
}

void PlacementRectangle::update()
{
	updatePosition();

	if (m_changedSize) 
	{
		updateVertices();
		m_changedSize = false;
	}
}

void PlacementRectangle::setSize(glm::dvec2 newSize)
{
	m_rectangleSize = newSize;
	m_changedSize = true;
}

glm::dvec3 PlacementRectangle::getPaddedPosition() const
{
	return m_paddedPosition;
}

glm::dvec3 PlacementRectangle::getPaddedRotation() const
{
	return m_paddedRotation;
}

bool PlacementRectangle::paddedPosition() const
{
	return m_isPositionPadded;
}

void PlacementRectangle::updatePosition()
{
	auto optPosition = m_pSimulationArea->getMousePosition();
	if (optPosition)
	{
		enableComponents();
		setPosition(optPosition.value());

		updatePaddings();
	}
	else
	{
		disableComponents();
	}
}

void PlacementRectangle::updateVertices()
{
	float halfWidth = m_rectangleSize.x / 2.0;
	float halfHeight = m_rectangleSize.y / 2.0;

	m_vertices = { 
		Point(-halfWidth, 0.0f, -halfHeight), Point( halfWidth, 0.0f, -halfHeight),
		Point(-halfWidth, 0.0f,  halfHeight), Point( halfWidth, 0.0f,  halfHeight)
	};

	updateGraphicsData();
	updatePhysicsData();
}

void PlacementRectangle::updateGraphicsData()
{
	Mesh rectMesh;
	rectMesh.vertices.positions = VD::PositionVertices(m_vertices.begin(), m_vertices.end());
	rectMesh.vertices.colors = VD::ColorVertices(m_vertices.size(), VD::ColorVertex(0.0, 0.9, 0.8, 1.0));
	rectMesh.indices = { 0,1,2, 1,2,3 };

	Model model;
	model.meshes.push_back(rectMesh);


	Info::ModelInfo mInfo;
	mInfo.model = &model;

	setupModel(mInfo, getGraphicsComponent().isActive());
}

void PlacementRectangle::updatePhysicsData()
{
	Points pts(m_vertices.begin(), m_vertices.end());

	getPhysicsComponent().collider().setBoundaries(pts);
}


void PlacementRectangle::updatePaddings()
{
	
	auto& pComp = getPhysicsComponent();

	auto roads = pComp.getAllCollisionWith("ROAD");
	if (!roads.empty())
	{
		m_isPositionPadded = true;

		if (auto collisionRoad = dynamic_cast<Road*>(roads[0]))
		{
			auto axis = collisionRoad->getAxisPoints();
			const auto[p1, p2] = findTwoClosestPoints(axis, getPosition());

			{
				const glm::vec3 forward(0.0, 0.0, 1.0);
				glm::vec3 roadForward(glm::normalize(p1 - p2));

				// correction, flip direciton
				{
					auto dotP = glm::dot(forward, roadForward);
					if (glm::acos(dotP) > glm::half_pi<float>())
						roadForward = -roadForward;
				}

				// rotation
				{
					const auto myForward			= glm::vec3(glm::sin(getRotation().x), 0.0, glm::cos(getRotation().x));
					const auto myForwardForwardDot	= glm::dot(myForward, forward);
					const auto myForwardForwardDet	= forward.z * myForward.x - forward.x * myForward.z;
					const auto myForwardForwardAngle= std::atan2(myForwardForwardDet, myForwardForwardDot);

					const auto roadForwardDot	= glm::dot(forward, roadForward);
					const auto roadForwardDet	= forward.z * roadForward.x - forward.x * roadForward.z;
					const auto roadForwardAngle	= std::atan2(roadForwardDet, roadForwardDot);

					//const auto myForwardRoadDot = glm::dot(myForward, roadForward);
					//const auto myForwardRoadDet = roadForward.z * myForward.x - roadForward.x * myForward.z;
					//const auto myForwardRoadAngle = std::atan2(myForwardRoadDet, myForwardRoadDot);

					auto shouldRotateAngle = myForwardForwardAngle + roadForwardAngle;

					float rotateAngle = roadForwardAngle;
					if (shouldRotateAngle > glm::quarter_pi<float>())
						rotateAngle += glm::half_pi<float>();
					if (shouldRotateAngle < -glm::quarter_pi<float>())
						rotateAngle -= glm::half_pi<float>();

					if (rotateAngle > glm::pi<float>())
						rotateAngle -= glm::two_pi<float>();

					m_paddedRotation = glm::vec3(rotateAngle, 0.0, 0.0);
				}
				// position

				{
					auto myForward = glm::vec3(glm::sin(m_paddedRotation.x), 0.0, glm::cos(m_paddedRotation.x));
					float angle = glm::acos(glm::dot(roadForward, myForward));

					bool useHeight = glm::epsilonEqual(angle, glm::half_pi<float>(), 0.01f);

					const float sideWidth = useHeight ? m_rectangleSize.y : m_rectangleSize.x;
					const float roadWidth = collisionRoad->getWidth();

					const auto roadAxisPoint = getClosestPointToLineSegment(p1, p2, getPosition());
					auto dir = glm::normalize(getPosition() - roadAxisPoint);

					m_paddedPosition = roadAxisPoint + (dir * (sideWidth + roadWidth) / 2.0f);
				}
			}
		}
	}
	else
	{
		m_isPositionPadded = false;

		m_paddedPosition = getPosition();
		m_paddedRotation = getRotation();
	}
	
}

void BuildingCreatorUI::draw()
{

}

BC::Resource::Resource(std::string modelPath, std::string name, BasicBuilding::BuildingType type)
{
	this->name = name;
	this->type = type;
	switch (type)
	{
	case BasicBuilding::BuildingType::HOUSE:
		House house;
		house.create(glm::vec3(), modelPath);
		house.getPhysicsComponent().setOtherCollisionTags({ "BUILDING", "ROAD" });
		house.enableComponents();
		house.setActive(false);

		this->prototype = std::shared_ptr<BasicBuilding>(new House(house));
		break;
	}
	Rect rect = deduceRectFromModel(Model(modelPath));
	this->prototype->getPhysicsComponent().collider().setBoundaries({ rect.p1, rect.p2, rect.p3, rect.p4 });
}

BuildingCreator::BuildingCreator(ObjectManager* objManager)
	:BasicCreator(objManager), 
	m_placementRectangle(m_pObjectManager->m_pSimulationArea),
	m_parkingLotCreator(this, objManager->m_pSimulationArea)
{
}

void BuildingCreator::prepareResources()
{
	using namespace BC;
	// house
	BasicBuilding::BuildingType curType = BasicBuilding::BuildingType::HOUSE;
	std::vector<Resource> resources
	{
		{ Resource("resources/models/house/house.obj","House 1", curType)},
	};
	m_resources[curType] = resources;


	setResource(&m_resources.begin()->second.front());
}

void BuildingCreator::setResource(BC::Resource* resource)
{
	m_currentResource = resource;
	m_currentResource->prototype->setActive(m_active);

	auto vertices = m_currentResource->prototype->getPhysicsComponent().collider().getBoundaries();

	auto width = glm::length(vertices[0] - vertices[1]);
	auto height = glm::length(vertices[0] - vertices[2]);

	m_placementRectangle.setSize(glm::dvec2(width, height));
}

void BuildingCreator::update()
{
	if (!m_active)
		return;
	
	auto& building = *m_currentResource->prototype;
	if (m_currentMode == Creator::CreatorMode::CREATE)
	{

		building.setActive(m_placementRectangle.getGraphicsComponent().isActive());
		building.setPosition(m_placementRectangle.getPaddedPosition());
		building.setRotation(m_placementRectangle.getPaddedRotation());
	}
	else
	{
		building.setActive(false);
	}

	if (App::input.mouse.pressedButton(GLFW_MOUSE_BUTTON_LEFT) && 
		!UI::getInstance().mouseOverlap())
	{
		// check collisions,
		// basically if i in collision and if is padded but not same road, return
		if (building.getPhysicsComponent().inCollisionWith("ROAD"))
		{
			if (m_placementRectangle.paddedPosition() && 
				building.getPhysicsComponent().inCollision("ROAD") != m_placementRectangle.getPhysicsComponent().inCollision("ROAD"))
			{
				return;
			}
		}


		switch (m_currentResource->type)
		{
		case BasicBuilding::BuildingType::HOUSE:
			House house = *static_cast<House*>(&*m_currentResource->prototype);
			house.getPhysicsComponent().setOtherCollisionTags({});
			house.enablePhysics();

			m_pObjectManager->m_houses.add(house);
			break;
		}
	}
}

void BuildingCreator::setCreatorModeAction()
{
}

void BuildingCreator::setActiveAction()
{
	m_placementRectangle.setActive(m_active);
	m_currentResource->prototype->setActive(m_active);

	m_parkingLotCreator.setActive(m_active);
}

*/