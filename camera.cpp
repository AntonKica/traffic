#include "camera.h"
#include "GlobalObjects.h"
#include "Transformations.h"
#include <cmath>

Camera::Camera()
{
	using namespace CameraSettings;

	m_position = {};
	m_focusPosition = defaultFocusPosition;
	m_front = defaultFront;

	m_yaw = defaultYaw;
	m_pitch = defaultPitch;

	m_near = defaultNear;
	m_far = defaultFar;

	m_fov = defaultFov;

	m_viewWidth = Settings::Window::defaultWidth;
	m_viewHeight = Settings::Window::defaultHeight;

	m_speed = defaultSpeed;
	m_circleDistance = defaultCircleDistance;
	m_circleOffsetPosition = {};

	updateMouse(m_viewWidth / 2, m_viewHeight / 2);
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
	return m_matrices.projection;
}

glm::mat4 Camera::getView() const
{
	return m_matrices.view;
	//return glm::lookAt(glm::dvec3(2.0),glm::dvec3(0.0), Transformations::VectorUp);
}

glm::vec3 Camera::getPosition() const
{
	return m_position;
}

void Camera::updateMouse(double newX, double newY)
{
	updateMouse(glm::ivec2(newX, newY));
}

void Camera::updateMouse(glm::dvec2 newMousePos)
{
	m_mouseOffset = glm::dvec2(m_mousePos) - newMousePos;
	m_mousePos = newMousePos;

	if (m_mouseOffset.x != 0.0 || m_mouseOffset.y != 0)
	{
		updateMouseRay();
	}
}

void Camera::update()
{
	glm::dvec2 mousePosition;
	glfwGetCursorPos(App::window.getWindow(), &mousePosition.x, &mousePosition.y);

	// minimalized and null division precaution
	updateViewSize();

	if (m_viewWidth != 0 && m_viewHeight != 0)
	{
		if (App::input.mouse.heldButton(GLFW_MOUSE_BUTTON_MIDDLE))
		{
			updateRotations();
			App::window.disableCursor();

			m_speed = 0;
		}
		else
		{
			App::window.showCursor();

			m_speed = CameraSettings::defaultSpeed;
		}
		updatePosition();


		updateMatrices();
		updateMouse(mousePosition);
	}
}

void Camera::updateViewSize()
{
	if (!App::window.isResized())
	{
		auto windowRect = App::window.getWindowSize();
		m_viewWidth = windowRect.width;
		m_viewHeight = windowRect.height;
	}
}

void Camera::updateMatrices()
{
	{
		m_matrices.view = glm::lookAt(m_position, m_position + m_front, m_up);
	}

	{
		double ratio = 0;
		// minimalisation
		if (m_viewHeight > 0.0)
		{
			ratio = double(m_viewWidth) / double(m_viewHeight);
		}
		glm::mat4 projection = glm::perspective(m_fov, ratio, m_near, m_far);
		projection[1][1] *= -1;

		m_matrices.projection = projection;
	}
}

void Camera::updateMouseRay()
{
	double x = (2.0 * m_mousePos.x) / m_viewWidth - 1.0;
	double y = 1.0 - (2.0 * m_mousePos.y) / m_viewHeight;
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

void Camera::updatePosition()
{
	// wow, just trying new fatures
	if (UI::getInstance().mouseOverlap())
		return;
	updateFocusPosition();
	// circle dist
	m_position = m_focusPosition + m_circleOffsetPosition;
}

void Camera::updateFocusPosition()
{
	double xNorm = 2.0 * m_mousePos.x / m_viewWidth - 1.0;
	double yNorm = 2.0 * m_mousePos.y / m_viewHeight - 1.0;

	const double border = 0.2;
	double moveSpeed = m_speed * App::time.deltaTime();

	/* MAGIC */
	if (!isInRange(xNorm, -1.0 + border, 1.0 - border))
	{
		double distance = lerp(0, xNorm - border, moveSpeed);
		m_focusPosition.x -= sin(m_yaw) * distance;
		m_focusPosition.z += cos(m_yaw) * distance;
	}
	if (!isInRange(yNorm, -1.0 + border, 1.0 - border))
	{
		double distance = lerp(0, yNorm - border, moveSpeed);
		m_focusPosition.x -= cos(m_yaw) * distance;
		m_focusPosition.z -= sin(m_yaw) * distance;
	}
}

void Camera::updateRotations()
{
	m_yaw += m_mouseOffset.x * App::time.deltaTime();
	m_pitch -= m_mouseOffset.y * App::time.deltaTime();

	const auto pi = glm::pi<double>();
	if (m_yaw > pi) m_yaw = -pi;
	else if (m_yaw < -pi) m_yaw = pi;

	// constrain to 90 degrees
	m_pitch = std::clamp(m_pitch, -glm::half_pi<double>(), 0.0);

	updateVectors();
}

void Camera::updateVectors()
{
	glm::dvec3 front;
	front.x = cos(m_yaw) * cos(m_pitch);
	front.y = sin(m_pitch);
	front.z = sin(m_yaw) * cos(m_pitch);

	m_front =	glm::normalize(front);
	m_right =	glm::normalize(glm::cross(m_front, Transformations::VectorUp));
	m_up	=	glm::normalize(glm::cross(m_right, m_front));

	updateCircleOffsetPosition();
}

void Camera::updateCircleOffsetPosition()
{
	m_circleOffsetPosition = -m_front * m_circleDistance;
}
