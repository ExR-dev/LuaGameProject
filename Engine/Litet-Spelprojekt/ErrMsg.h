#pragma once

//#define DEBUG_MESSAGES

#ifndef DEBUG_MESSAGES
#include <iostream>
#endif // DEBUG_MESSAGES

#include <string>
#include <format>

#define ErrMsg(msg) ErrorMessage(msg, __FILE__, __LINE__)

void ErrorMessage(const char *msg, const std::string &filePath, int line);
void ErrorMessage(const std::string &msg, const std::string &filePath, int line);
