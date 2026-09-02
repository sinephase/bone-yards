// gl_main.h -- main renderer interface

#ifndef GL_MAIN_H
#define GL_MAIN_H

// Windows headers must come first
#ifdef _WIN32
    #include <windows.h>
#endif

#include <GL/gl.h>

namespace render {

void R_Init(int width, int height);
void R_RenderFrame();
void R_Shutdown();

}  // namespace render

#endif // GL_MAIN_H
