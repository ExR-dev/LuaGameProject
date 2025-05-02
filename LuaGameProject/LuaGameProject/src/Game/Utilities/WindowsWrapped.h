#pragma once

class HWND__;

// All members end with "W" (for "wrapped") to avoid name collision with the Windows API.
namespace Windows
{
	struct HWNDW
	{
		HWND__ *hwnd;

		operator HWND__ *() const
		{
			return hwnd;
		}
	};

	void SleepW(int milliseconds);

	bool IsDebuggerPresentW();

	int IDABORTW();
	int IDRETRYW();

	int MessageBoxAW(const char *msg, const char *caption);

	HWNDW GetConsoleWindowW();
}