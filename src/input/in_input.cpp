// in_input.cpp -- input handling and player controls
// Manages keyboard, mouse, and gamepad input, translates to game commands,
// and handles player movement and actions.

#include "qcommon.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <string.h>
#include <vector>

namespace input {

// Key states
struct key_state_t {
    bool down;
    bool pressed;
    bool released;
};

// Input context
struct input_context_t {
    // Keyboard
    key_state_t keys[SDL_NUM_SCANCODES];

    // Mouse
    int mouse_x;
    int mouse_y;
    int mouse_dx;
    int mouse_dy;
    bool mouse_buttons[3];
    int mouse_wheel;

    // Gamepad
    bool gamepad_connected;
    float gamepad_sticks[4];  // LX, LY, RX, RY
    bool gamepad_buttons[16];
    float gamepad_triggers[2];  // LT, RT

    // Player movement
    glm::vec3 move_direction;
    float move_speed;
    float look_yaw;
    float look_pitch;
    float look_speed;

    // Commands
    bool forward;
    bool backward;
    bool strafe_left;
    bool strafe_right;
    bool jump;
    bool crouch;
    bool sprint;
    bool attack;
    bool use;
    bool reload;

} in_context;

// Key binding
struct key_binding_t {
    SDL_Scancode key;
    const char* command;
};

// Default key bindings
static const key_binding_t default_bindings[] = {
    {SDL_SCANCODE_W, "move_forward"},
    {SDL_SCANCODE_A, "move_left"},
    {SDL_SCANCODE_S, "move_backward"},
    {SDL_SCANCODE_D, "move_right"},
    {SDL_SCANCODE_SPACE, "jump"},
    {SDL_SCANCODE_LCTRL, "crouch"},
    {SDL_SCANCODE_LSHIFT, "sprint"},
    {SDL_SCANCODE_E, "use"},
    {SDL_SCANCODE_R, "reload"},
    {SDL_SCANCODE_ESCAPE, "menu"},
    {SDL_SCANCODE_TAB, "inventory"},
    {SDL_SCANCODE_F1, "help"},
};

static std::vector<key_binding_t> key_bindings;

// Initialize input system
void IN_Init() {
    Com_Printf("Initializing input system...\n");

    std::memset(&in_context, 0, sizeof(in_context));
    in_context.move_speed = 10.0f;
    in_context.look_speed = 0.1f;

    // Setup default bindings
    for (const auto& binding : default_bindings) {
        key_bindings.push_back(binding);
    }

    // Initialize SDL input
    SDL_SetRelativeMouseMode(SDL_TRUE);

    Com_Printf("Input system initialized\n");
}

// Update key states
static void IN_UpdateKeyStates() {
    const Uint8* key_state = SDL_GetKeyboardState(nullptr);

    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        bool was_down = in_context.keys[i].down;
        bool is_down = key_state[i] != 0;

        in_context.keys[i].pressed = is_down && !was_down;
        in_context.keys[i].released = !is_down && was_down;
        in_context.keys[i].down = is_down;
    }
}

// Update mouse state
static void IN_UpdateMouseState() {
    int x, y;
    Uint32 buttons = SDL_GetRelativeMouseState(&x, &y);

    in_context.mouse_dx = x;
    in_context.mouse_dy = y;

    in_context.mouse_buttons[0] = (buttons & SDL_BUTTON_LMASK) != 0;
    in_context.mouse_buttons[1] = (buttons & SDL_BUTTON_MMASK) != 0;
    in_context.mouse_buttons[2] = (buttons & SDL_BUTTON_RMASK) != 0;

    in_context.mouse_wheel = 0;
}

// Update gamepad state
static void IN_UpdateGamepadState() {
    SDL_GameController* controller = SDL_GameControllerOpen(0);
    if (!controller) {
        in_context.gamepad_connected = false;
        return;
    }

    in_context.gamepad_connected = true;

    // Analog sticks
    in_context.gamepad_sticks[0] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
    in_context.gamepad_sticks[1] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
    in_context.gamepad_sticks[2] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f;
    in_context.gamepad_sticks[3] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f;

    // Buttons
    in_context.gamepad_buttons[0] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) != 0;
    in_context.gamepad_buttons[1] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) != 0;
    in_context.gamepad_buttons[2] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X) != 0;
    in_context.gamepad_buttons[3] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y) != 0;
    in_context.gamepad_buttons[4] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK) != 0;
    in_context.gamepad_buttons[5] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START) != 0;
    in_context.gamepad_buttons[6] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK) != 0;
    in_context.gamepad_buttons[7] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) != 0;
    in_context.gamepad_buttons[8] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0;
    in_context.gamepad_buttons[9] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0;
    in_context.gamepad_buttons[10] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0;
    in_context.gamepad_buttons[11] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0;

    // Triggers
    in_context.gamepad_triggers[0] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32768.0f;
    in_context.gamepad_triggers[1] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32768.0f;

    SDL_GameControllerClose(controller);
}

// Process key binding
static void IN_ProcessKeyBinding(SDL_Scancode key) {
    for (const auto& binding : key_bindings) {
        if (binding.key == key) {
            // Execute command
            if (strcmp(binding.command, "move_forward") == 0) {
                in_context.forward = true;
            } else if (strcmp(binding.command, "move_backward") == 0) {
                in_context.backward = true;
            } else if (strcmp(binding.command, "move_left") == 0) {
                in_context.strafe_left = true;
            } else if (strcmp(binding.command, "move_right") == 0) {
                in_context.strafe_right = true;
            } else if (strcmp(binding.command, "jump") == 0) {
                in_context.jump = true;
            } else if (strcmp(binding.command, "crouch") == 0) {
                in_context.crouch = true;
            } else if (strcmp(binding.command, "sprint") == 0) {
                in_context.sprint = true;
            } else if (strcmp(binding.command, "use") == 0) {
                in_context.use = true;
            } else if (strcmp(binding.command, "reload") == 0) {
                in_context.reload = true;
            }
            break;
        }
    }
}

// Update movement commands
static void IN_UpdateMovementCommands() {
    in_context.forward = false;
    in_context.backward = false;
    in_context.strafe_left = false;
    in_context.strafe_right = false;
    in_context.jump = false;
    in_context.crouch = false;
    in_context.sprint = false;
    in_context.use = false;
    in_context.reload = false;

    // Check key bindings
    for (const auto& binding : key_bindings) {
        if (in_context.keys[binding.key].down) {
            IN_ProcessKeyBinding(binding.key);
        }
    }

    // Gamepad input
    if (in_context.gamepad_connected) {
        // Left stick for movement
        if (in_context.gamepad_sticks[1] < -0.2f) in_context.forward = true;
        if (in_context.gamepad_sticks[1] > 0.2f) in_context.backward = true;
        if (in_context.gamepad_sticks[0] < -0.2f) in_context.strafe_left = true;
        if (in_context.gamepad_sticks[0] > 0.2f) in_context.strafe_right = true;

        // Buttons
        if (in_context.gamepad_buttons[0]) in_context.jump = true;
        if (in_context.gamepad_buttons[1]) in_context.crouch = true;
        if (in_context.gamepad_buttons[2]) in_context.use = true;
        if (in_context.gamepad_buttons[3]) in_context.reload = true;
        if (in_context.gamepad_buttons[4]) in_context.sprint = true;
    }

    // Calculate movement direction
    in_context.move_direction = {0, 0, 0};

    if (in_context.forward) in_context.move_direction.y += 1.0f;
    if (in_context.backward) in_context.move_direction.y -= 1.0f;
    if (in_context.strafe_left) in_context.move_direction.x -= 1.0f;
    if (in_context.strafe_right) in_context.move_direction.x += 1.0f;

    // Normalize movement direction
    if (glm::length(in_context.move_direction) > 0.0f) {
        in_context.move_direction = glm::normalize(in_context.move_direction);
    }

    // Apply sprint modifier
    float speed_mult = in_context.sprint ? 1.5f : 1.0f;
    in_context.move_direction *= in_context.move_speed * speed_mult;
}

// Update look commands
static void IN_UpdateLookCommands() {
    // Mouse look
    in_context.look_yaw += in_context.mouse_dx * in_context.look_speed;
    in_context.look_pitch -= in_context.mouse_dy * in_context.look_speed;

    // Clamp pitch
    if (in_context.look_pitch > 89.0f) in_context.look_pitch = 89.0f;
    if (in_context.look_pitch < -89.0f) in_context.look_pitch = -89.0f;

    // Gamepad look
    if (in_context.gamepad_connected) {
        in_context.look_yaw += in_context.gamepad_sticks[2] * in_context.look_speed * 5.0f;
        in_context.look_pitch -= in_context.gamepad_sticks[3] * in_context.look_speed * 5.0f;
    }
}

// Process SDL event
void IN_ProcessEvent(const SDL_Event* event) {
    switch (event->type) {
        case SDL_KEYDOWN:
            break;

        case SDL_KEYUP:
            break;

        case SDL_MOUSEMOTION:
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                in_context.attack = true;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event->button.button == SDL_BUTTON_LEFT) {
                in_context.attack = false;
            }
            break;

        case SDL_MOUSEWHEEL:
            in_context.mouse_wheel = event->wheel.y;
            break;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERAXISMOTION:
            // Handled in IN_UpdateGamepadState
            break;
    }
}

// Update all input states
void IN_Update() {
    IN_UpdateKeyStates();
    IN_UpdateMouseState();
    IN_UpdateGamepadState();
    IN_UpdateMovementCommands();
    IN_UpdateLookCommands();
}

// Get movement vector
glm::vec3 IN_GetMoveDirection() {
    return in_context.move_direction;
}

// Get look angles
void IN_GetLookAngles(float* yaw, float* pitch) {
    *yaw = in_context.look_yaw;
    *pitch = in_context.look_pitch;
}

// Get action states
bool IN_IsForward() { return in_context.forward; }
bool IN_IsBackward() { return in_context.backward; }
bool IN_IsStrafeLeft() { return in_context.strafe_left; }
bool IN_IsStrafeRight() { return in_context.strafe_right; }
bool IN_IsJump() { return in_context.jump; }
bool IN_IsCrouch() { return in_context.crouch; }
bool IN_IsSprint() { return in_context.sprint; }
bool IN_IsAttack() { return in_context.attack; }
bool IN_IsUse() { return in_context.use; }
bool IN_IsReload() { return in_context.reload; }

// Get mouse buttons
bool IN_GetMouseButton(int button) {
    if (button < 0 || button >= 3) return false;
    return in_context.mouse_buttons[button];
}

// Bind key to command
void IN_BindKey(SDL_Scancode key, const char* command) {
    // Remove existing binding for this key
    for (auto it = key_bindings.begin(); it != key_bindings.end(); ++it) {
        if (it->key == key) {
            key_bindings.erase(it);
            break;
        }
    }

    // Add new binding
    key_binding_t binding;
    binding.key = key;
    binding.command = command;
    key_bindings.push_back(binding);

    Com_Printf("Bound key %d to command '%s'\n", key, command);
}

// Unbind key
void IN_UnbindKey(SDL_Scancode key) {
    for (auto it = key_bindings.begin(); it != key_bindings.end(); ++it) {
        if (it->key == key) {
            key_bindings.erase(it);
            break;
        }
    }
}

// Clear all bindings
void IN_ClearBindings() {
    key_bindings.clear();
}

// Shutdown input system
void IN_Shutdown() {
    Com_Printf("Shutting down input system...\n");
    IN_ClearBindings();
}

}  // namespace input
