#include "WindowsWrapped.h"
#include <windows.h>

void Windows::SleepW(int milliseconds)
{
	Sleep(milliseconds);
}

bool Windows::IsDebuggerPresentW()
{
	return IsDebuggerPresent() != 0;
}

int Windows::IDABORTW()
{
	return IDABORT;
}
int Windows::IDRETRYW()
{
	return IDRETRY;
}

int Windows::MessageBoxAW(const char *msg, const char *caption)
{
	return MessageBoxA(0, msg, caption, MB_ABORTRETRYIGNORE | MB_ICONERROR);
}

Windows::HWNDW Windows::GetConsoleWindowW()
{
	Windows::HWNDW hwnd(GetConsoleWindow());
	return hwnd;
}

