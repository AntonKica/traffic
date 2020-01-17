#include "Mouse.h"

bool Mouse::pressedButton(IL::GLFW_KEYCODE button) const
{
	const auto& record = m_buttonRecords[button];

	return record.pressedCurrentFrame && !record.pressedLastFrame;
}

bool Mouse::releasedButton(IL::GLFW_KEYCODE button) const
{
	const auto& record = m_buttonRecords[button];

	return !record.pressedCurrentFrame && record.pressedLastFrame;
}

bool Mouse::heldButton(IL::GLFW_KEYCODE button) const
{
	const auto& record = m_buttonRecords[button];

	return record.pressedCurrentFrame && record.pressedLastFrame;
}

void Mouse::initialize()
{
	initializeButtons();
}

void Mouse::initializeButtons()
{
	for (auto& buttonRecord : m_buttonRecords)
		buttonRecord = IL::Record();
}

void Mouse::setButtonValue(IL::GLFW_KEYCODE button, IL::GLFW_KEY_STATE state)
{
	m_buttonRecords[button].pressed = (state == GLFW_PRESS || state == GLFW_REPEAT);
}

void Mouse::updateRecords()
{
	for (auto& record : m_buttonRecords)
	{
		record.pressedLastFrame = record.pressedCurrentFrame;
		record.pressedCurrentFrame = record.pressed;
	}
}
