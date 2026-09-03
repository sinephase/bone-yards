// qcommon.cpp -- common code shared between client and server
// Console output, basic utilities, and initialization

#include "qcommon.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <chrono>

// Simple console output
void Com_Printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);
}

// Debug console output (only in debug builds)
void Com_DPrintf(const char* fmt, ...) {
#ifdef _DEBUG
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);
#endif
}

// Error output
void Com_Error(int level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);

    // TODO: Clean shutdown
    exit(level);
}

// Warning output
void Com_Warning(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

// Get current time in milliseconds using std::chrono
unsigned int Com_Milliseconds() {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    return static_cast<unsigned int>(elapsed.count());
}

// Initialize common systems
void Com_Init() {
    Com_Printf("Initializing common systems...\n");
    Com_Printf("Build: %s %s\n", __DATE__, __TIME__);
}

// Shutdown common systems
void Com_Shutdown() {
    Com_Printf("Shutting down common systems...\n");
}
