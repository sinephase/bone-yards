#pragma once

#include "../qcommon/math.h"
#include <stdint.h>

/* OpenGL renderer interface */
typedef struct {
    uint32_t width;
    uint32_t height;
    float aspect;
} video_t;

typedef struct {
    vec3_t origin;
    vec3_t angles;
    float fov;
} refdef_t;

typedef struct {
    vec3_t origin;
    vec3_t color;
    float radius;
    float intensity;
} dlight_t;

/* Renderer initialization and shutdown */
bool R_Init(uint32_t width, uint32_t height, const char *window_title);
void R_Shutdown(void);

/* Frame rendering */
void R_BeginFrame(refdef_t *rd);
void R_DrawWorld(void);
void R_DrawEntities(void);
void R_EndFrame(void);

/* Lighting */
void R_SetLights(const dlight_t *lights, int numlights);
void R_SetAmbient(const vec3_t color);

/* Models and drawing */
typedef uint32_t model_t;
model_t R_RegisterModel(const char *name);
void R_DrawModel(model_t model, const vec3_t origin, const vec3_t angles);

/* Textures */
typedef uint32_t texture_t;
texture_t R_RegisterTexture(const char *name);
void R_SetTexture(texture_t tex);

/* Screenshots */
bool R_Screenshot(const char *filename);
