#pragma once
/// NOTE: I use '///' for comments and '//' for exclusion as they are displayed with slightly different colors, making it easier to parse visually.
/// If they are both the same color for you, go to Tools -> Options -> Environment -> Fonts and Colors, find "XML Doc Comment" and change it's color.


/// PARALLEL_UPDATE enables the use of the ParallelUpdate method in entities.
#define PARALLEL_UPDATE
#define PARALLEL_THREADS 4

#define USE_RAYLIB

/// DEBUG_BUILD enables debug features like ImGui, debug drawing, gizmos and statistics reporting.
#define DEBUG_BUILD

#ifdef DEBUG_BUILD
/// -------- Debug Build Defines -------- 

/// USE_IMGUI enables the use of ImGui for development purposes. Most runtime development tools are used through ImGui.
#ifndef USE_RAYLIB
#define USE_IMGUI
#define USE_SDL3
#endif


/// PIX_TIMELINING enables the use of PIX for Windows for CPU and GPU profiling.
//#define PIX_TIMELINING

/// FAST_LOAD_MODE skips most content loading to speed up startup time. Good for running in debug mode, as loading all content can take a long time.
//#define FAST_LOAD_MODE

/// EDIT_MODE works like a preset for map editing. For example, it sets the active scene to the game scene, 
/// increases the ambient light level and skips spawning the player & monster.
//#define EDIT_MODE

/// DEBUG_DRAW enables drawing lines in the scene at any point of the frame loop.
#define DEBUG_DRAW

/// COMPILE_CONTENT enables recompilation of the content file upon startup. Required after adding new content.
#define COMPILE_CONTENT

/// DISABLE_MONSTER makes the monster handicap
//#define DISABLE_MONSTER

/// LEAK_DETECTION enables memory leak detection through the use of the _CrtSetDbgFlag function. 
/// This is only available in debug mode. All .cpp files using the new operator must redefine new to DEBUG_NEW 
/// for leak reporting to work properly. See the top of Main.cpp for an example.
#define LEAK_DETECTION

/// -------- Debug Build Defines -------- 
#endif
