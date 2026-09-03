// gl.cpp -- OpenGL renderer interface stub implementation
// Provides a minimal, functional implementation of the gl.h interface so that
// the module can be linked into the build. Actual OpenGL calls are left as
// TODOs for the renderer milestone that unifies this interface with
// src/renderer/gl_main.{h,cpp}.

#include "gl.h"
#include "../qcommon.h"
#include <cstdio>

namespace {

video_t g_video;
bool g_initialized = false;

}  // namespace

bool R_Init(uint32_t width, uint32_t height, const char *window_title) {
    g_video.width = width;
    g_video.height = height;
    g_video.aspect = height ? (float)width / (float)height : 1.0f;

    Com_Printf("R_Init: %s (%ux%u)\n", window_title ? window_title : "(untitled)", width, height);
    g_initialized = true;
    return true;
}

void R_Shutdown(void) {
    if (g_initialized) {
        Com_Printf("R_Shutdown\n");
    }
    g_initialized = false;
}

void R_BeginFrame(refdef_t *rd) {
    (void)rd;
    // TODO: clear buffers, set up view/projection matrices
}

void R_DrawWorld(void) {
    // TODO: submit world geometry to the GPU
}

void R_DrawEntities(void) {
    // TODO: submit entity models to the GPU
}

void R_EndFrame(void) {
    // TODO: swap buffers
}

void R_SetLights(const dlight_t *lights, int numlights) {
    (void)lights;
    (void)numlights;
    // TODO: upload dynamic lights to the shader
}

void R_SetAmbient(const vec3_t color) {
    (void)color;
    // TODO: upload ambient color to the shader
}

model_t R_RegisterModel(const char *name) {
    (void)name;
    // TODO: load and cache model
    return 0;
}

void R_DrawModel(model_t model, const vec3_t origin, const vec3_t angles) {
    (void)model;
    (void)origin;
    (void)angles;
    // TODO: draw the model at the given origin/angles
}

texture_t R_RegisterTexture(const char *name) {
    (void)name;
    // TODO: load and cache texture
    return 0;
}

void R_SetTexture(texture_t tex) {
    (void)tex;
    // TODO: bind the texture
}

bool R_Screenshot(const char *filename) {
    Com_Printf("R_Screenshot: %s (not yet implemented)\n", filename ? filename : "(null)");
    return false;
}
