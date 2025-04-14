#include "stdafx.h"
#include "ErrMsg.h"

void ErrorMessage(const std::string &msg, const std::string &filePath, int line)
{
    std::string file = filePath.substr(filePath.find_last_of("/\\") + 1);
    std::cerr << std::format("ERROR: [{}({})]", file, line) << std::endl;
    std::cerr << msg << std::endl;
}

void WarningMessage(const std::string &msg, const std::string &filePath, int line)
{
    std::string file = filePath.substr(filePath.find_last_of("/\\") + 1);
    std::cerr << std::format("WARNING: [{}({})]", file, line) << std::endl;
    std::cerr << msg << std::endl;
}

void DebugMessage(const std::string &msg)
{
    std::cout << msg << std::endl;
    DebugMessage(msg.c_str());
}
