#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

namespace Settings
{
	namespace Window
	{
		constexpr uint32_t defaultWidth = 800;
		constexpr uint32_t defaultHeight = 600;
	}
}

class Window
{
private:
	// callbacks
	friend void windowResizeCallback(GLFWwindow* pWindow, int x, int y);
	friend void windowCloseCallback(GLFWwindow* pWindow);
	friend void windowFocusCallback(GLFWwindow* pWindow, int focused);

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
	friend class VulkanBase;
	friend int main();

	void initialize(std::string name);
	void cleanup();
	void update();
	

	enum class State
	{
		RESIZED = 1 << 0,
		CLOSED = 1 << 1,
		FOCUSED = 1 << 2,
	};
	void setState(State state);
	void switchState(State state);

	GLFWwindow* m_window;
	uint32_t m_state;
};

