#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <algorithm>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

template<typename T> bool isInRange(const T& val, const T& min, const T& max)
{
	return val >= min && val <= max;
}

static double lerp(double min, double max, double val)
{
	return min * (1.0 - val) + max * val;
}

static glm::dvec3 rotate(const glm::dvec3 vector, const glm::dvec3 axis, double theta)
{
	double cosTheta = cos(theta);
	double sinTheta = sin(theta);

	glm::dvec3 rotated = (vector * cosTheta) + (glm::cross(axis, vector) * sinTheta) + (axis * glm::dot(axis, vector)) * (1 - cosTheta);

	return rotated;
}

namespace CameraSettings
{
	const glm::vec3 defaultFocusPosition = glm::dvec3(0.0, 0.0, 0.0);
	const glm::vec3 defaultFront = glm::dvec3(0.0, 0.0, 1.0);
	const double defaultYaw = 0.0;
	const double defaultPitch = -glm::quarter_pi<double>();

	const double defaultNear = 0.01f;
	const double defaultFar = 100.0f;

	const double defaultFov = 45.0f;

	const double defaultSpeed = 25.0f;

	const double defaultCircleDistance = 25.0f;
}

class Camera
{
private:
	glm::dvec3 m_position;
	glm::dvec3 m_focusPosition;
	glm::dvec3 m_front;
	glm::dvec3 m_up;
	glm::dvec3 m_right;

	double m_yaw, m_pitch;
	double m_near, m_far;
	double m_fov;

	int m_viewWidth, m_viewHeight;
	double m_speed;
	double m_circleDistance;

	glm::dvec3 m_circleOffsetPosition;

	glm::ivec2 m_mousePos;
	glm::dvec2 m_mouseOffset;
	glm::dvec3 m_mouseRay;

	struct
	{
		glm::mat4 view;
		glm::mat4 projection;
	} m_matrices;

public:

	Camera();

	glm::dvec3 getUp() const;
	glm::dvec3 getRight() const;
	glm::dvec3 getFront() const;
	glm::dvec3 getMouseRay() const;
	glm::mat4 getProjection() const;
	glm::mat4 getView() const;
	glm::vec3 getPosition() const;
	void updateMouse(int newX, int newY);
	void updateMouse(glm::dvec2 newMousePos);
	void update();

private:
	void updateViewSize();
	void updateMatrices();

	void updateMouseRay();
	void updatePosition();
	void updateFocusPosition();
	void updateRotations();
	void updateVectors();
	void updateCircleOffsetPosition();
};
#endif // !CAMERA_H
