#include "Input.h"
#include "GlobalObjects.h"
#include "GlobalSynchronization.h"

#include <mutex>

Input::Input()
{
}

bool Input::pressedKey(unsigned char key) const
{
	return m_pressedKeys.find(key)->second;
}

bool Input::pressedLMB() const
{
	return glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}
bool Input::pressedRMB() const
{
	return glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}
void Input::run()
{
	initialize();
	initializeKeys();
	
	mainLoop();

	cleanup();
}
void Input::initialize()
{
	m_pWindow = App::vulkanBase.getWindow();
}

void Input::initializeKeys()
{
	m_pressedKeys.clear();
	for (uint16_t key = 0; key < std::numeric_limits<unsigned char>::max(); ++key)
		m_pressedKeys[key] = false;
}

void Input::mainLoop()
{
	GlobalSynchronizaion::input.initialized = true;
	GlobalSynchronizaion::input.cv.notify_one();

	while (!GlobalSynchronizaion::shouldStopEngine)
	{
		{
			std::mutex updateWaitMutex;
			std::unique_lock<std::mutex> updateWaitLock(updateWaitMutex);
			GlobalSynchronizaion::input.cv.wait(updateWaitLock, [] { return GlobalSynchronizaion::input.update; });
			GlobalSynchronizaion::input.update = false;
		}

		updateKeys();

		{
			GlobalSynchronizaion::input.updated = true;
			GlobalSynchronizaion::input.cv.notify_one();
		}
	}
}

void Input::cleanup()
{
	{
		std::mutex cleanupWaitMutex;
		std::unique_lock<std::mutex> cleanupWaitLock(cleanupWaitMutex);
		GlobalSynchronizaion::input.cv.wait(cleanupWaitLock, [] { return GlobalSynchronizaion::input.cleanUp; });
		GlobalSynchronizaion::input.cleanUp = false;
	}

	{
		GlobalSynchronizaion::input.cleanedUp = true;
		GlobalSynchronizaion::input.cv.notify_one();
	}
}

void Input::updateKeys()
{
	for (auto& [key, pressed] : m_pressedKeys)
		pressed = glfwGetKey(m_pWindow, key) == GLFW_PRESS;
}


/*
void Input::cleanKeyInputs()
{
	for (auto& [_, pressed] : m_pressedKeys)
		pressed = false;
}*/