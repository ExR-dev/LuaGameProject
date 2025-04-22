#pragma once

// All members end with "W" (for "wrapped") to avoid name collision with the Windows API.
namespace Windows
{
	void SleepW(int milliseconds);

	bool IsDebuggerPresentW();

	int IDABORTW();
	int IDRETRYW();

	int MessageBoxAW(const char *msg, const char *caption);
}