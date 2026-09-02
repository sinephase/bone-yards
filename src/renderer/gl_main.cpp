// gl_main.cpp -- main renderer implementation

#include "gl_main.h"
#include "../qcommon.h"
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace render {

static int viewport_width = 1280;
static int viewport_height = 720;

void R_Init(int width, int height) {
    Com_Printf("Initializing renderer...\n");
    viewport_width = width;
    viewport_height = height;
    
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, width, height);
    
    Com_Printf("Renderer initialized (OpenGL)\n");
}

void R_RenderFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // TODO: Render scene
}

void R_Shutdown() {
    Com_Printf("Shutting down renderer...\n");
}

}  // namespace render
