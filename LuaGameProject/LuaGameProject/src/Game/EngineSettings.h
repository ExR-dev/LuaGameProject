#pragma once

#ifdef _DEBUG
//#define LEAK_DETECTION
#endif

#define LUA_DEBUG

#undef TRACY_ENABLE
#ifdef TRACY_ENABLE
	#define TRACY_MEMORY
#endif
