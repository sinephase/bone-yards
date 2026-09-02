// qcommon.h -- common definitions and declarations

#ifndef QCOMMON_H
#define QCOMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Platform detection
#ifdef _WIN32
    #define PLATFORM_WINDOWS 1
#elif __APPLE__
    #define PLATFORM_MACOS 1
#elif __linux__
    #define PLATFORM_LINUX 1
#endif

// Basic types
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef int qboolean;

// Entity structure
struct edict_t {
    int s_modelindex;
    float origin[3];
    float angles[3];
    float velocity[3];
    float scale;
    
    // Simplified structure - extend as needed
    struct {
        int modelindex;
    } s;
};

// Console functions
void Com_Printf(const char* fmt, ...);
void Com_DPrintf(const char* fmt, ...);
void Com_Error(int level, const char* fmt, ...);
void Com_Warning(const char* fmt, ...);
unsigned int Com_Milliseconds();
void Com_Init();
void Com_Shutdown();

#endif // QCOMMON_H
