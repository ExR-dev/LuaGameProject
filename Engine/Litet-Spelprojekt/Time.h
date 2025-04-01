#pragma once

#include <chrono>

typedef unsigned int UINT;


struct Snapshot
{
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> snapshot;
};

// Manages total time and delta time as well as capturing snapshots of time for performance testing.
class Time
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	std::chrono::time_point<std::chrono::high_resolution_clock> _frame;

	std::vector<Snapshot> _snapshots;

public:
	float time, deltaTime;

	Time();

	static inline Time &GetInstance()
	{
		static Time instance;
		return instance;
	}

	void Update();

	UINT TakeSnapshot(const std::string &name);
	[[nodiscard]] float CompareSnapshots(UINT s1, UINT s2) const;
	[[nodiscard]] float CompareSnapshots(const std::string &name) const;
	bool TryCompareSnapshots(const std::string &name, float *time) const;
};
