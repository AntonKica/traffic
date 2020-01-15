#pragma once
#include <GLFW/glfw3.h>
#include <map>
#include <array>
#include "Keyboard.h"
#include "Mouse.h"

class Input
{
	friend class Window;
public:
	Input();

	Keyboard keyboard;
	Mouse mouse;
private:
	// private cause current design
	void initialize();
	void update();
};

