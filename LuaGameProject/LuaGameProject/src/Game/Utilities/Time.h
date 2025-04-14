#pragma once
#include <chrono>

using ChronoClock = std::chrono::high_resolution_clock;

class Time
{
public:
	Time() = default;
	~Time() = default;

	// Singleton getter
	static Time &Instance() noexcept
	{
		static Time instance;
		return instance;
	}

	static void Update() noexcept
	{
		Time &instance = Instance();
		instance.m_lastTick = instance.m_thisTick;
		instance.m_thisTick = ChronoClock::now();
		instance.m_deltaDuration = (instance.m_thisTick - instance.m_lastTick);
		instance.m_deltaTime = instance.m_deltaDuration.count() / 1000000000.0f;
	}

	// Returns the time since the last tick in seconds
	[[nodiscard]] static float DeltaTime() noexcept
	{
		return Instance().m_deltaTime;
	}

private:
	ChronoClock::time_point m_lastTick = ChronoClock::now();
	ChronoClock::time_point m_thisTick = m_lastTick;
	ChronoClock::duration m_deltaDuration = m_thisTick - m_lastTick;
	float m_deltaTime = 0.0f;

};