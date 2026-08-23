/*
 * =======================================================================
 *
 * Bone Yards - Minimal Game Engine
 *
 * Standalone engine: OpenGL window, game loop, I/O handling
 *
 * =======================================================================
 */

#ifndef BY_ENGINE_H
#define BY_ENGINE_H

#include "../game/game.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ====================================================================== */
/* WINDOW & RENDERING CONTEXT */
/* ====================================================================== */

typedef struct {
	GLFWwindow *window;
	int width;
	int height;
	float fov;
	float near_plane;
	float far_plane;
} engine_context_t;

/* ====================================================================== */
/* FUNCTION DECLARATIONS */
/* ====================================================================== */

/* Engine lifecycle */
qboolean BY_Engine_Init(int width, int height, const char *title);
void BY_Engine_Shutdown(void);
void BY_Engine_Run(void);
qboolean BY_Engine_IsRunning(void);

/* Input handling */
void BY_Engine_UpdateInput(void);
void BY_Engine_GetMouseDelta(float *dx, float *dy);
qboolean BY_Engine_IsKeyPressed(int key);

/* Rendering */
void BY_Engine_BeginFrame(void);
void BY_Engine_EndFrame(void);
void BY_Engine_SetViewport(float x, float y, float z, float pitch, float yaw);

/* Debug rendering */
void BY_Engine_DrawBox(vec3_t pos, float size, float r, float g, float b);
void BY_Engine_DrawLine(vec3_t start, vec3_t end, float r, float g, float b);
void BY_Engine_DrawText(vec3_t pos, const char *text, float scale);

/* Math helpers */
mat4_t BY_Math_Identity(void);
mat4_t BY_Math_Perspective(float fov, float aspect, float near, float far);
mat4_t BY_Math_LookAt(vec3_t eye, vec3_t center, vec3_t up);
vec3_t BY_Math_VectorAdd(vec3_t a, vec3_t b);
vec3_t BY_Math_VectorScale(vec3_t v, float s);
float BY_Math_VectorLength(vec3_t v);

extern engine_context_t engine;

#endif /* BY_ENGINE_H */
