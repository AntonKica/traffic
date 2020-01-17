#include "Keyboard.h"

bool Keyboard::pressedKey(IL::GLFW_KEYCODE key) const
{
	const auto& [_, record] = *m_keyRecords.find(key);

	return record.pressedCurrentFrame && !record.pressedLastFrame;
}

bool Keyboard::releasedKey(IL::GLFW_KEYCODE key) const
{
	const auto& [_, record] = *m_keyRecords.find(key);

	return !record.pressedCurrentFrame && record.pressedLastFrame;
}

bool Keyboard::heldKey(IL::GLFW_KEYCODE key) const
{
	const auto& [_, record] = *m_keyRecords.find(key);

	return record.pressedCurrentFrame && record.pressedLastFrame;
}

void Keyboard::initialize()
{
	initializeKeys();
}

void Keyboard::initializeKeys()
{
	trackedKeyCount = 500 + 1;
	for (IL::GLFW_KEYCODE curKey = 0; curKey < trackedKeyCount; ++curKey)
		m_keyRecords[curKey] = IL::Record();
}

void Keyboard::setKeyValue(IL::GLFW_KEYCODE key, IL::GLFW_KEY_STATE state)
{
	auto& [_, keyState] = *m_keyRecords.find(key);

	keyState.pressed = (state == GLFW_PRESS) || (state == GLFW_REPEAT);
}

void Keyboard::updateRecords()
{
	for (auto& [key, record] : m_keyRecords)
	{
		record.pressedLastFrame = record.pressedCurrentFrame;
		record.pressedCurrentFrame = record.pressed;
	}
}
