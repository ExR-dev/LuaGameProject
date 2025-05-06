#pragma once

#include "../../LuaGameProject/dep/tracy-0.11.1/public/tracy/TracyC.h"

#ifdef TRACY_ENABLE
// Here goes tracy stuff



#ifdef TRACY_MEMORY
// Here goes tracy allocation stuff
void WrapTracyCFreeFunc(const void *ptr);
void WrapTracyCAllocFunc(const void *ptr, unsigned long long nsize);

#define WrapTracyCFree(ptr) WrapTracyCFreeFunc(ptr)
#define WrapTracyCAlloc(ptr, nsize) WrapTracyCAllocFunc(ptr, nsize)

#endif
#else
// Here goes un-tracy stuff

#endif

#ifndef TRACY_MEMORY
// Here goes un-tracy allocation stuff

#define WrapTracyCFree(ptr)
#define WrapTracyCAlloc(ptr, nsize)

#endif
