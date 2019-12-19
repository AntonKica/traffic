#include "Time.h"
#include <chrono>

Time::Time()
{
	m_lastTime = std::chrono::high_resolution_clock::now();
	m_deltaTime = {};
}

void Time::tick()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	m_deltaTime = currentTime - m_lastTime;

	m_lastTime = currentTime;
}

double Time::deltaTime() const
{
	double seconds = m_deltaTime.count();

	return seconds;
}
