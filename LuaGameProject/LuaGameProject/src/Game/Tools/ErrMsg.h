#pragma once
#include <iostream>
#include <string>
#include <format>
#include "Game/Utilities/WindowsWrapped.h"
#include <intrin.h>

#define CUSTOM_ASSERT(expr, file, line) \
    do { \
        if (!(expr)) { \
            if (Windows::IsDebuggerPresentW()) { \
                __debugbreak(); \
            } else { \
                char msg[1024]; \
                snprintf(msg, sizeof(msg), \
                    "Assertion failed: %s\n\nFile: %s\nLine: %d\n\n" \
                    "Press Retry to debug, Ignore to continue, Abort to exit", \
                    #expr, file, line); \
                int result = Windows::MessageBoxAW(msg, "Assertion Failed"); \
                if (result == Windows::IDRETRYW()) { \
                    __debugbreak(); \
                } else if (result == Windows::IDABORTW()) { \
                    exit(1); \
                } \
            } \
        } \
    } while (0)


#ifdef _DEBUG
    // Used for fatal problems, aborts automatically.
    #define ErrMsg(msg) { ErrorMessage(msg, __FILE__, __LINE__); std::abort(); }

    // Used for important information that does not require immediate action.
    #define DbgMsg(msg) DebugMessage(msg)

    // Used for potentially fatal problems, lets the user choose how to react.
    #define Warn(msg) { WarningMessage(msg, __FILE__, __LINE__); CUSTOM_ASSERT(false, __FILE__, __LINE__); }

    // Used to warn if a condition is not met, lets the user choose how to react.
    #define Assert(expr, msg) { CUSTOM_ASSERT(expr, __FILE__, __LINE__); }
#else
    // Used for fatal problems, aborts automatically.
    #define ErrMsg(msg)

    // Used for important information that does not require immediate action.
    #define DbgMsg(msg)

    // Used for potentially fatal problems, lets the user choose how to react.
    #define Warn(msg)

    // Used to warn if a condition is not met, lets the user choose how to react.
    #define Assert(expr, msg)
#endif // DEBUG_MESSAGES

void ErrorMessage(const std::string &msg, const std::string &filePath, int line);
void WarningMessage(const std::string &msg, const std::string &filePath, int line);
void DebugMessage(const std::string &msg);
