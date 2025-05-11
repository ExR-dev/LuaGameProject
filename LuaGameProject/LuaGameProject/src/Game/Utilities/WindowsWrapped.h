#pragma once

#include <string>

struct HWND__;

// All members end with "W" (for "wrapped") to avoid naming collision with the Windows API.
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

	bool OpenFileCatalog(std::string &fileName, std::string &filePath, const std::string &startDir = "");
}