#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Utilities.h"

#include <array>
#include "InputLiterals.h"

class Mouse :
	public Utility::PrivateClass
{
public:
	bool pressedButton(IL::GLFW_KEYCODE button) const;
	bool releasedButton(IL::GLFW_KEYCODE button) const;
	bool heldButton(IL::GLFW_KEYCODE button) const;

private:
	friend class Input;
	friend class Window;

	void initialize();
	void initializeButtons();

	void setButtonValue(IL::GLFW_KEYCODE button, bool value);
	void updateRecords();
	
	std::array<IL::Record, GLFW_MOUSE_BUTTON_LAST + 1> m_butonRecords;
};

