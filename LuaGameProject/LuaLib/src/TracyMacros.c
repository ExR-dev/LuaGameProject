#include "LuaSettings.h"
#include "TracyMacros.h"
#include "../../LuaGameProject/dep/tracy-0.11.1/public/tracy/TracyC.h"

#ifdef TRACY_ENABLE
// Here goes tracy stuff


/*
void WrapTracyCZone(const char *func, const char *line)
{
	TracyCZone(ctx, 1);

}
*/

#ifdef TRACY_MEMORY
// Here goes tracy allocation stuff

void WrapTracyCFreeFunc(const void *ptr)
{
	___tracy_emit_memory_free(ptr, 0);
}

void WrapTracyCAllocFunc(const void *ptr, unsigned long long nsize)
{
	___tracy_emit_memory_alloc(ptr, nsize, 0);
}

#endif
#endif