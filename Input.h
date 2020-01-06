#pragma once
#include <GLFW/glfw3.h>
#include <map>
class Input
{
	friend int main();
	friend void runInput();
public:
	Input();
	bool pressedKey(unsigned char key) const;
	bool pressedLMB() const;
	bool pressedRMB() const;

private:
	void run();
	void initialize();
	void initializeKeys();

	// for acces reasons
	void mainLoop();
	void cleanup();
	void updateKeys();
	//void cleanKeyInputs();


	GLFWwindow* m_pWindow;
	std::map<char, bool> m_pressedKeys;

	// synchronization
};

