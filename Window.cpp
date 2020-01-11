#include "Window.h"
#include "vulkanHelper/VulkanHelper.h"
#include <GLFW/glfw3.h>

void windowResizeCallback(GLFWwindow* pWindow, int x, int y)
{
	auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
	window.setState(Window::State::RESIZED);
}

void windowCloseCallback(GLFWwindow* pWindow)
{
	auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
	window.setState(Window::State::CLOSED);
}

void windowFocusCallback(GLFWwindow* pWindow, int focused)
{
	auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));

	window.switchState(Window::State::FOCUSED);
}

void Window::initialize(std::string name)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(Settings::Window::defaultWidth, Settings::Window::defaultHeight, name.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, windowResizeCallback);
	glfwSetWindowCloseCallback(m_window, windowCloseCallback);
	glfwSetWindowFocusCallback(m_window, windowFocusCallback);

	// toggle focus
	switchState(State::FOCUSED);
}

void Window::cleanup()
{
	glfwTerminate();
}

void Window::update() 
{
	// reset state
	if (m_state & static_cast<int>(State::RESIZED))
		switchState(State::RESIZED);

	glfwPollEvents();
	glfwSwapBuffers(m_window);
}

void Window::setState(State state)
{
	m_state |= static_cast<uint32_t>(state);
}

void Window::switchState(State state)
{
	m_state ^= static_cast<uint32_t>(state);
}

GLFWwindow* const Window::getWindow()
{
	return m_window;
}


Window::Rectangle Window::getWindowSize() const
{
	Rectangle rect;
	glfwGetWindowSize(m_window, &rect.width, &rect.height);

	return rect;
}

Window::Rectangle Window::getFramebufferSize() const
{
	Rectangle rect;
	glfwGetFramebufferSize(m_window, &rect.width, &rect.height);

	return rect;
}

bool Window::isResized() const
{
	return m_state & static_cast<uint32_t>(State::RESIZED);
}

bool Window::isClosed() const
{
	return m_state & static_cast<uint32_t>(State::CLOSED);
}

bool Window::isFocused() const
{
	return m_state & static_cast<uint32_t>(State::FOCUSED);
}

glm::dvec2 Window::getMousePosition() const
{
	glm::dvec2 mousePosition;
	glfwGetCursorPos(m_window, &mousePosition.x, &mousePosition.y);

	return mousePosition;
}

