#include "Mouse.h"

bool Mouse::pressedButton(IL::GLFW_KEYCODE button) const
{
	const auto& record = m_butonRecords[button];

	return record.pressedCurrentFrame && !record.pressedLastFrame;
}

bool Mouse::releasedButton(IL::GLFW_KEYCODE button) const
{
	const auto& record = m_butonRecords[button];

	return !record.pressedCurrentFrame && record.pressedLastFrame;
}

bool Mouse::heldButton(IL::GLFW_KEYCODE button) const
{
	const auto& record = m_butonRecords[button];

	return record.pressedCurrentFrame && record.pressedLastFrame;
}

void Mouse::initialize()
{
	initializeButtons();
}

void Mouse::initializeButtons()
{
	for (auto& buttonRecord : m_butonRecords)
		buttonRecord = IL::Record();
}

void Mouse::setButtonValue(IL::GLFW_KEYCODE button, bool value)
{
	m_butonRecords[button].pressed = value;
}

void Mouse::updateRecords()
{
	for (auto& record : m_butonRecords)
	{
		record.pressedLastFrame = record.pressedCurrentFrame;
		record.pressedCurrentFrame = record.pressed;
	}
}
