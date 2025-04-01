#include "stdafx.h"
#include "ErrMsg.h"

#include <Windows.h>

#ifdef LEAK_DETECTION
#define new			DEBUG_NEW
#endif


inline static std::wstring NarrowToWide(const std::string &narrow)
{
    if (narrow.empty())
        return { };

    const size_t reqLength = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.length(), nullptr, 0);

    std::wstring ret(reqLength, L'\0');

    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.length(), &ret[0], (int)ret.length());
    return ret;
}

void ErrorMessage(const char *msg, const std::string &filePath, int line)
{
    std::string file = filePath.substr(filePath.find_last_of("/\\") + 1);
    OutputDebugString(NarrowToWide(std::format("[{}({})]\n", file, line)).c_str());

	OutputDebugString(NarrowToWide(msg).c_str());
	OutputDebugString(L"\n");
    std::cerr << msg << std::endl;
}

void ErrorMessage(const std::string &msg, const std::string &filePath, int line)
{
    std::string file = filePath.substr(filePath.find_last_of("/\\") + 1);
    OutputDebugString(NarrowToWide(std::format("[{}({})]\n", file, line)).c_str());

    OutputDebugString(NarrowToWide(msg).c_str());
    OutputDebugString(L"\n");
    std::cerr << msg << std::endl;
}
