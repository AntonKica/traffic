#include "camera.h"
#include "GlobalObjects.h"

Camera::Camera()
{
	using namespace CameraSettings;

	m_position = defaultPosition;
	m_front = defaultFront;

	m_yaw = defaultYaw;
	m_pitch = defaultPitch;

	m_near = defaultNear;
	m_far = defaultFar;

	m_fov = defaultFov;

	m_width = defaultWidth;
	m_height = defaultHeight;

	m_speed = defaultSpeed;
	m_rotateMode = false;

	updateMouse(m_width / 2, m_height / 2);
	updateVectors();
}

glm::dvec3 Camera::getUp() const
{
	return m_up;
}

glm::dvec3 Camera::getRight() const
{
	return m_right;
}

glm::dvec3 Camera::getFront() const
{
	return m_front;
}

glm::dvec3 Camera::getMouseRay() const
{
	return m_mouseRay;
}

glm::mat4 Camera::getProjection() const
{
	double ratio = 0;
	// minimalisation
	if (m_height > 0.0)
	{
		ratio = double(m_width) / double(m_height);
	}
	glm::mat4 projection = glm::perspective(m_fov, ratio, m_near, m_far);
	projection[1][1] *= -1;
	return projection;
}

glm::mat4 Camera::getView() const
{
	return glm::lookAt(m_position, m_position + m_front, m_up);
	//return glm::lookAt(glm::dvec3(2.0),glm::dvec3(0.0), Transformations::VectorUp);
}

glm::vec3 Camera::getPosition() const
{
	return m_position;
}

void Camera::resizeView(int newWidth, int newHeight)
{
	m_width = newWidth;
	m_height = newHeight;
}

void Camera::updateMouse(int newX, int newY)
{
	updateMouse(glm::ivec2(newX, newY));
}

void Camera::updateMouse(glm::dvec2 newMousePos)
{
	m_mouseOffset = glm::dvec2(m_mousePos) - newMousePos;
	m_mousePos = newMousePos;

	if (m_mouseOffset.x != 0.0 && m_mouseOffset.y != 0)
	{
		updateMouseRay();
	}
}

void Camera::setRotateMode(bool val)
{
	m_rotateMode = val;
}

void Camera::update()
{
	glm::dvec2 mousePosition;
	glfwGetCursorPos(App::vulkanBase.getWindow(), &mousePosition.x, &mousePosition.y);
	// minimalized and null division precaution
	if (m_width == 0 || m_height == 0)
		return;

	if (m_rotateMode)
	{
		updateRotations(App::time.deltaTime());
	}
	else
	{
		updatePosition(App::time.deltaTime());
	}

	updateMouse(mousePosition);
}

void Camera::updateMouseRay()
{
	double x = (2.0 * m_mousePos.x) / m_width - 1.0;
	double y = 1.0 - (2.0 * m_mousePos.y) / m_height;
	double z = 1.0;
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, -1.0);
	glm::mat4 projMat = getProjection();

	glm::vec4 ray_eye = glm::inverse(projMat) * ray_clip;
	ray_eye = glm::vec4(-ray_eye.x, ray_eye.y, ray_eye.z, 0.0);//ray_eye.z na -1.0 //ray_eye.x neagtivne lebo inac je flipnute horizontalne

	glm::mat4 viewMat = getView();
	auto temp = glm::inverse(viewMat) * ray_eye;
	glm::vec ray_world = glm::normalize(glm::vec3(temp.x, temp.y, temp.z));


	m_mouseRay = ray_world;
}

void Camera::updatePosition(double deltaTime)
{
	double xNorm = 2.0 * m_mousePos.x / m_width - 1.0;
	double yNorm = 2.0 * m_mousePos.y / m_height - 1.0;

	const double border = 0.2;
	double moveSpeed = m_speed * deltaTime;

	/* MAGIC */
	if (!isInRange(xNorm, -1.0 + border, 1.0 - border))
	{
		double distance = lerp(0, xNorm - border, moveSpeed);
		m_position.x -= sin(glm::radians(m_yaw)) * distance;
		m_position.z += cos(glm::radians(m_yaw)) * distance;
	}
	if (!isInRange(yNorm, -1.0 + border, 1.0 - border))
	{
		double distance = lerp(0, yNorm - border, moveSpeed);
		m_position.x -= cos(glm::radians(m_yaw)) * distance;
		m_position.z -= sin(glm::radians(m_yaw)) * distance;
	}
}

void Camera::updateRotations(double deltaTime)
{
	m_yaw += m_mouseOffset.x * deltaTime * 100;
	m_pitch -= m_mouseOffset.y * deltaTime * 100;

	if (m_yaw > 180.0) m_yaw = -180.0;
	else if (m_yaw < -180.0) m_yaw = 180.0;

	m_pitch = std::clamp(m_pitch, -60.0, 60.0);

	updateVectors();
}

void Camera::updateVectors()
{
	glm::dvec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = glm::normalize(front);
	m_right = glm::normalize(glm::cross(m_front, Transformations::VectorUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));
}
