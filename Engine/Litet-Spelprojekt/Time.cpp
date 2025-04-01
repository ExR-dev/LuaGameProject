#include "stdafx.h"
#include "Time.h"

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif


Time::Time() : _start(std::chrono::high_resolution_clock::now()), _frame(std::chrono::high_resolution_clock::now())
{
	time = 0.0f;
	deltaTime = 1.0f / 60.0f;
}

void Time::Update()
{
	const auto newFrame = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<float> newTime = newFrame - _start;
	time = newTime.count();

	const std::chrono::duration<float> newDeltaTime = newFrame - _frame;
	deltaTime = newDeltaTime.count();

	_snapshots.clear();
	_frame = newFrame;
}


UINT Time::TakeSnapshot(const std::string &name)
{
	_snapshots.push_back({ name, std::chrono::high_resolution_clock::now() });
	return static_cast<UINT>(_snapshots.size() - 1);
}

float Time::CompareSnapshots(const UINT s1, const UINT s2) const
{
	if (s1 >= s2)
		return -1.0f;

	if (s2 >= static_cast<UINT>(_snapshots.size()))
		return -1.0f;

	const std::chrono::duration<float> duration = _snapshots[s2].snapshot - _snapshots[s1].snapshot;
	return duration.count();
}

float Time::CompareSnapshots(const std::string &name) const
{
	UINT s1 = 0, s2 = 0;
	bool foundFirst = false;

	for (UINT i = 0; i < static_cast<UINT>(_snapshots.size()); i++)
	{
		if (_snapshots[i].name != name)
			continue;

		if (foundFirst)
		{
			s2 = i;
			break;
		}

		s1 = i;
		foundFirst = true;
	}

	return CompareSnapshots(s1, s2);
}

bool Time::TryCompareSnapshots(const std::string &name, float *time) const
{
	UINT s1 = 0, s2 = 0;
	bool foundFirst = false;
	bool foundSecond = false;

	for (UINT i = 0; i < static_cast<UINT>(_snapshots.size()); i++)
	{
		if (_snapshots[i].name != name)
			continue;

		if (foundFirst)
		{
			foundSecond = true;
			s2 = i;
			break;
		}

		s1 = i;
		foundFirst = true;
	}

	if (!foundSecond)
		return false;

	*time = CompareSnapshots(s1, s2);
	return true;
}
