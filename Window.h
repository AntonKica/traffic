#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

namespace Settings
{
	namespace Window
	{
		constexpr uint32_t defaultWidth = 1280;
		constexpr uint32_t defaultHeight = 768;
	}
}

class Window
{
private:
	// callbacks
	static void windowResizeCallback(GLFWwindow* pWindow, int x, int y);
	static void windowCloseCallback(GLFWwindow* pWindow);
	static void windowFocusCallback(GLFWwindow* pWindow, int focused);
	static void windowKeyboardInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void windowMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
public:
	struct Rectangle
	{
		int width, height;
	};

	GLFWwindow* const getWindow();

	Rectangle getWindowSize() const;
	Rectangle getFramebufferSize() const;
	bool isResized() const;
	bool isClosed() const;
	bool isFocused() const;

	glm::dvec2 getMousePosition() const;
private:
	friend int main();

	void initialize(std::string windowName);
	void cleanup();
	void updateFrame();
	

	enum class State
	{
		RESIZED = 1 << 0,
		CLOSED = 1 << 1,
		FOCUSED = 1 << 2,
	};
	void setState(State state);
	void resetState(State state);
	void switchState(State state);

	GLFWwindow* m_window;
	std::string m_windowName;

	uint32_t m_state;
};

