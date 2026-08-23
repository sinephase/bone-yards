// gl_main.h -- main renderer interface

#ifndef GL_MAIN_H
#define GL_MAIN_H

namespace render {

void R_Init(int width, int height);
void R_RenderFrame();
void R_Shutdown();

}  // namespace render

#endif // GL_MAIN_H
