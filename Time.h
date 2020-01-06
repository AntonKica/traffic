#pragma once
#include <chrono>

class Time
{
	friend int main();
public:
	Time();
	double deltaTime() const;
private:
	void tick();
	std::chrono::duration<double> m_deltaTime;
	std::chrono::high_resolution_clock::time_point m_lastTime;
};