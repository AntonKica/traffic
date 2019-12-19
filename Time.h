#pragma once
#include <chrono>

class Time
{
public:
	Time();
	void tick();
	double deltaTime() const;
private:
	std::chrono::duration<double> m_deltaTime;
	std::chrono::high_resolution_clock::time_point m_lastTime;
};