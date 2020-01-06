#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <algorithm>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "Transformations.h"

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
	const glm::vec3 defaultPosition = glm::vec3(0.0, 2.5, 0.0);
	const glm::vec3 defaultFront = Transformations::VectorForward;
	const double defaultYaw = 90.0f;
	const double defaultPitch = 0.0;

	const double defaultNear = 0.01f;
	const double defaultFar = 100.0f;

	const double defaultFov = 45.0f;

	const double defaultWidth = 1024;
	const double defaultHeight = 768;

	const double defaultSpeed = 5.0f;
}

class Camera
{
private:
	glm::dvec3 m_position;
	glm::dvec3 m_front;
	glm::dvec3 m_up;
	glm::dvec3 m_right;

	double m_yaw, m_pitch;
	double m_near, m_far;
	double m_fov;

	int m_width, m_height;
	double m_speed;

	bool m_rotateMode;

	glm::ivec2 m_mousePos;
	glm::dvec2 m_mouseOffset;
	glm::dvec3 m_mouseRay;
public:

	Camera();

	glm::dvec3 getUp() const;
	glm::dvec3 getRight() const;
	glm::dvec3 getFront() const;
	glm::dvec3 getMouseRay() const;
	glm::mat4 getProjection() const;
	glm::mat4 getView() const;
	glm::vec3 getPosition() const;
	void resizeView(int newWidth, int newHeight);
	void updateMouse(int newX, int newY);
	void updateMouse(glm::dvec2 newMousePos);
	void setRotateMode(bool val);
	void update();

	void updateMouseRay();
	void updatePosition(double deltaTime);
	void updateRotations(double deltaTime);
	void updateVectors();
};
#endif // !CAMERA_H
