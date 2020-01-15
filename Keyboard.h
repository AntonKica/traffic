#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Utilities.h"

#include <unordered_map>
#include "InputLiterals.h"

class Keyboard :
	public Utility::PrivateClass
{
public:

	bool pressedKey(IL::GLFW_KEYCODE key) const;
	bool releasedKey(IL::GLFW_KEYCODE key) const;
	bool heldKey(IL::GLFW_KEYCODE key) const;
private:
	friend class Window;
	friend class Input;

	void initialize();
	void initializeKeys();

	void setKeyValue(IL::GLFW_KEYCODE key, bool value);
	void updateRecords();

	uint32_t trackedKeyCount;
	std::unordered_map<IL::GLFW_KEYCODE, IL::Record> m_keyRecords;
};

