// in_input.cpp -- input system implementation

#include "in_input.h"
#include "../qcommon.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <cstring>

namespace input {

static bool key_state[SDL_NUM_SCANCODES] = {};
static bool mouse_buttons[3] = {};
static int mouse_x = 0, mouse_y = 0;

void IN_Init() {
    Com_Printf("Initializing input system...\n");
    std::memset(key_state, 0, sizeof(key_state));
    std::memset(mouse_buttons, 0, sizeof(mouse_buttons));
}

void IN_Update() {
    // Get current mouse state
    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    mouse_x = x;
    mouse_y = y;
    mouse_buttons[0] = buttons & SDL_BUTTON_LMASK;
    mouse_buttons[1] = buttons & SDL_BUTTON_MMASK;
    mouse_buttons[2] = buttons & SDL_BUTTON_RMASK;
}

void IN_ProcessEvent(const SDL_Event* event) {
    if (!event) return;
    
    switch (event->type) {
        case SDL_KEYDOWN:
            if (event->key.keysym.scancode < SDL_NUM_SCANCODES) {
                key_state[event->key.keysym.scancode] = true;
            }
            break;
        case SDL_KEYUP:
            if (event->key.keysym.scancode < SDL_NUM_SCANCODES) {
                key_state[event->key.keysym.scancode] = false;
            }
            break;
        default:
            break;
    }
}

void IN_Shutdown() {
    Com_Printf("Shutting down input system...\n");
}

glm::vec3 IN_GetMoveDirection() {
    glm::vec3 dir(0.0f);
    if (IN_IsForward()) dir.z += 1.0f;
    if (IN_IsBackward()) dir.z -= 1.0f;
    if (IN_IsStrafeLeft()) dir.x -= 1.0f;
    if (IN_IsStrafeRight()) dir.x += 1.0f;
    if (IN_IsJump()) dir.y += 1.0f;
    if (IN_IsCrouch()) dir.y -= 1.0f;
    return dir;
}

void IN_GetLookAngles(float* yaw, float* pitch) {
    if (yaw) *yaw = 0.0f;
    if (pitch) *pitch = 0.0f;
}

bool IN_IsForward() { return key_state[SDL_SCANCODE_W]; }
bool IN_IsBackward() { return key_state[SDL_SCANCODE_S]; }
bool IN_IsStrafeLeft() { return key_state[SDL_SCANCODE_A]; }
bool IN_IsStrafeRight() { return key_state[SDL_SCANCODE_D]; }
bool IN_IsJump() { return key_state[SDL_SCANCODE_SPACE]; }
bool IN_IsCrouch() { return key_state[SDL_SCANCODE_LCTRL]; }
bool IN_IsSprint() { return key_state[SDL_SCANCODE_LSHIFT]; }
bool IN_IsAttack() { return mouse_buttons[0]; }
bool IN_IsUse() { return key_state[SDL_SCANCODE_E]; }
bool IN_IsReload() { return key_state[SDL_SCANCODE_R]; }

bool IN_GetMouseButton(int button) {
    if (button >= 0 && button < 3) {
        return mouse_buttons[button];
    }
    return false;
}

void IN_BindKey(SDL_Scancode key, const char* command) {
    // TODO: Implement key binding system
}

void IN_UnbindKey(SDL_Scancode key) {
    // TODO: Implement key unbinding
}

void IN_ClearBindings() {
    // TODO: Clear all key bindings
}

}  // namespace input
