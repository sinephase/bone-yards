// in_input.h -- input system interface

#ifndef IN_INPUT_H
#define IN_INPUT_H

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

namespace input {

void IN_Init();
void IN_Update();
void IN_ProcessEvent(const SDL_Event* event);
void IN_Shutdown();

glm::vec3 IN_GetMoveDirection();
void IN_GetLookAngles(float* yaw, float* pitch);

bool IN_IsForward();
bool IN_IsBackward();
bool IN_IsStrafeLeft();
bool IN_IsStrafeRight();
bool IN_IsJump();
bool IN_IsCrouch();
bool IN_IsSprint();
bool IN_IsAttack();
bool IN_IsUse();
bool IN_IsReload();

bool IN_GetMouseButton(int button);

void IN_BindKey(SDL_Scancode key, const char* command);
void IN_UnbindKey(SDL_Scancode key);
void IN_ClearBindings();

}  // namespace input

#endif // IN_INPUT_H
