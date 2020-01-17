#include "Window.h"
#include "vulkanHelper/VulkanHelper.h"
#include <GLFW/glfw3.h>

#include "GlobalObjects.h"
#include "GlobalSynchronization.h"

void Window::windowResizeCallback(GLFWwindow* pWindow, int x, int y)
{
	auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));

	window.setState(Window::State::RESIZED);
}

void Window::windowCloseCallback(GLFWwindow* pWindow)
{
	auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));

	window.setState(Window::State::CLOSED);
}

void Window::windowFocusCallback(GLFWwindow* pWindow, int focused)
{
	auto& window = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));

	window.switchState(Window::State::FOCUSED);
}

void Window::windowKeyboardInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	App::input.keyboard.setKeyValue(key, action);
}

void Window::windowMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	App::input.mouse.setButtonValue(button, action);
}

void Window::initialize(std::string windowName)
{
	// first self
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(Settings::Window::defaultWidth, Settings::Window::defaultHeight, windowName.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, windowResizeCallback);
		glfwSetWindowCloseCallback(m_window, windowCloseCallback);
		glfwSetWindowFocusCallback(m_window, windowFocusCallback);

		// for intpu
		glfwSetKeyCallback(m_window, windowKeyboardInputCallback);
		glfwSetMouseButtonCallback(m_window, windowMouseButtonCallback);

		// toggle focus
		setState(State::FOCUSED);
	}
	// then input
	{
		App::input.initialize();
	}
}
/*
void Window::mainLoop()
{
	{
		std::lock_guard notifyLock(GlobalSynchronizaion::window.notifyMutex);

		GlobalSynchronizaion::window.initialized = true;
		GlobalSynchronizaion::main_cv.notify_one();
	}

	while (!GlobalSynchronizaion::shouldStopEngine)
	{
		{
			//std::cout << "I wait\n";
			std::unique_lock updateWaitLock(GlobalSynchronizaion::window.updateWaitMutex);

			auto waitPred = [] { return GlobalSynchronizaion::window.update; };
			while (!waitPred())
				GlobalSynchronizaion::thread_cv.wait(updateWaitLock);

			GlobalSynchronizaion::window.update = false;
		}
		//std::cout << "Started input loop\n";

		updateFrame();

		{
			std::lock_guard notifyLock(GlobalSynchronizaion::window.notifyMutex);

			//std::cout << "Input about to notify\n";
			GlobalSynchronizaion::window.updated = true;
			GlobalSynchronizaion::main_cv.notify_one();
		}
	}
}
*/
void Window::cleanup()
{
	/*
	{
		std::unique_lock cleanupWaitLock(GlobalSynchronizaion::window.cleanupWaitMutex);

		GlobalSynchronizaion::thread_cv.wait(cleanupWaitLock, [] { return GlobalSynchronizaion::window.cleanUp; });
		GlobalSynchronizaion::window.cleanUp = false;
	}


	{
		std::lock_guard notifyLock(GlobalSynchronizaion::window.notifyMutex);

		GlobalSynchronizaion::window.cleanedUp = true;
		GlobalSynchronizaion::main_cv.notify_one();
	}*/


	glfwTerminate();
}

void Window::updateFrame()
{
	{
		// reset resize state
		resetState(State::RESIZED);

//#ifdef _DEBUG
		glfwSetWindowTitle(m_window, (std::to_string(int(1.0 / App::time.deltaTime())) + "FPS").c_str());
//#endif // _DEBUG

		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}

	{
		App::input.update();
	}
}

void Window::setState(State state)
{
	m_state |= static_cast<uint32_t>(state);
}

void Window::resetState(State state)
{
	m_state &= ~static_cast<uint32_t>(state);
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

void Window::showCursor()
{
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::hideCursor()
{
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void Window::disableCursor()
{
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
