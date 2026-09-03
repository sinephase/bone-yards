// main.cpp -- application entry point
// Initializes SDL2, OpenGL, and runs the main game loop

#include "qcommon.h"

// Windows headers must come before GL headers
#ifdef _WIN32
    #include <windows.h>
#endif

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include "renderer/gl_main.h"
#include "input/in_input.h"

namespace {
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool running = true;
    int window_width = 1280;
    int window_height = 720;
}

// Initialize SDL and OpenGL
bool Main_Init() {
    Com_Printf("=== Bone Yards Engine ===\n");
    Com_Printf("Initializing systems...\n\n");

    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        Com_Error(1, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    Com_Printf("SDL2 initialized\n");

    // Create window
    window = SDL_CreateWindow(
        "Bone Yards - C++ OpenGL Port",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        Com_Error(1, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    Com_Printf("Window created: %dx%d\n", window_width, window_height);

    // Create OpenGL context
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        Com_Error(1, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }

    Com_Printf("OpenGL context created\n");

    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    // Initialize renderer
    Com_Printf("\n");
    render::R_Init(window_width, window_height);

    // Initialize input
    Com_Printf("\n");
    input::IN_Init();

    Com_Printf("\n");
    Com_Printf("=== Engine Ready ===\n\n");

    return true;
}

// Handle events
void Main_HandleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    window_width = event.window.data1;
                    window_height = event.window.data2;
                    glViewport(0, 0, window_width, window_height);
                }
                break;

            default:
                input::IN_ProcessEvent(&event);
                break;
        }
    }
}

// Main loop
void Main_Frame() {
    // Update input
    input::IN_Update();

    // Render frame
    render::R_RenderFrame();

    // Swap buffers
    SDL_GL_SwapWindow(window);
}

// Shutdown
void Main_Shutdown() {
    Com_Printf("\nShutting down...\n");

    render::R_Shutdown();
    input::IN_Shutdown();

    if (gl_context) {
        SDL_GL_DeleteContext(gl_context);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();

    Com_Printf("Goodbye!\n");
}

// Entry point - Explicitly decorated for MSVC linking
extern "C" int main(int argc, char* argv[]) {
    Com_Init();

    if (!Main_Init()) {
        Com_Error(1, "Failed to initialize engine\n");
        return 1;
    }

    // Main loop
    while (running) {
        Main_HandleEvents();
        Main_Frame();
    }

    Main_Shutdown();

    return 0;
}
