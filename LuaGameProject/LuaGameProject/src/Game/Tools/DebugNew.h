#pragma once
#include "TrackedAlloc.h"

#ifdef LEAK_DETECTION
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include "crtdbg.h"

#define DEBUG_NEW   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW   new
#endif