#pragma once
#include <atomic>
#include <thread>
#include <execution>

namespace Game
{
	static bool IsQuitting = false;

	// Use this to only execute console commands at the end of the game loop, making the game loop wait for it to finish
	static std::lock_guard<std::mutex> ConsoleMutex;
}
