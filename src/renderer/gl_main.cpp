// gl_main.cpp -- OpenGL renderer main loop and initialization
// Initializes OpenGL, manages the render loop, handles projection/view matrices,
// and orchestrates the rendering of the scene (geometry, lighting, effects).

#include "qcommon.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>
#include <GL/glu.h>

namespace render {

// Renderer state
struct renderer_state_t {
    int screen_width;
    int screen_height;
    float fov;
    float znear;
    float zfar;
    glm::vec3 camera_pos;
    glm::vec3 camera_forward;
    glm::vec3 camera_up;
    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;
    int frame_count;
    float frame_time;
} r_state;

// Render statistics
struct render_stats_t {
    int surfaces_rendered;
    int triangles_rendered;
    int lights_active;
    int particles_rendered;
    float frame_ms;
} r_stats;

// Initialize the renderer
void R_Init(int width, int height) {
    Com_Printf("Initializing OpenGL renderer (%dx%d)...\n", width, height);

    r_state.screen_width = width;
    r_state.screen_height = height;
    r_state.fov = 75.0f;
    r_state.znear = 0.1f;
    r_state.zfar = 4000.0f;
    r_state.frame_count = 0;

    // Set viewport
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Clear color
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // Lighting setup
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Material properties
    float ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

    Com_Printf("OpenGL renderer initialized\n");
}

// Update projection matrix based on screen dimensions
static void R_UpdateProjection() {
    float aspect = (float)r_state.screen_width / (float)r_state.screen_height;
    r_state.projection_matrix = glm::perspective(
        glm::radians(r_state.fov),
        aspect,
        r_state.znear,
        r_state.zfar
    );

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(r_state.projection_matrix));
    glMatrixMode(GL_MODELVIEW);
}

// Update view matrix from camera
static void R_UpdateView() {
    glm::vec3 look_at = r_state.camera_pos + r_state.camera_forward;
    r_state.view_matrix = glm::lookAt(
        r_state.camera_pos,
        look_at,
        r_state.camera_up
    );

    glLoadMatrixf(glm::value_ptr(r_state.view_matrix));
}

// Set camera position and orientation
void R_SetCamera(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up) {
    r_state.camera_pos = pos;
    r_state.camera_forward = glm::normalize(forward);
    r_state.camera_up = glm::normalize(up);
    R_UpdateView();
}

// Clear the screen
static void R_Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    std::memset(&r_stats, 0, sizeof(r_stats));
}

// Render world geometry
static void R_DrawWorld() {
    // TODO: Get brushes/faces from world
    // For now, render a simple grid for debugging

    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(0.5f, 0.5f, 0.5f);

    // Draw grid (100x100 units)
    for (int i = -50; i <= 50; i += 10) {
        // Lines along X axis
        glVertex3f(-500.0f, i * 10.0f, 0.0f);
        glVertex3f(500.0f, i * 10.0f, 0.0f);

        // Lines along Y axis
        glVertex3f(i * 10.0f, -500.0f, 0.0f);
        glVertex3f(i * 10.0f, 500.0f, 0.0f);
    }

    glEnd();
    glEnable(GL_LIGHTING);
}

// Render all entities in the world
static void R_DrawEntities() {
    // TODO: Get entities from game world and render their models
    // This would iterate through entity list and call R_DrawModel() for each

    Com_DPrintf("Drawing entities...\n");
}

// Draw a simple bounding box (for debug visualization)
static void R_DrawBBox(const glm::vec3& mins, const glm::vec3& maxs, float r, float g, float b) {
    glDisable(GL_LIGHTING);
    glColor3f(r, g, b);
    glBegin(GL_LINE_BOX);

    // Front face
    glVertex3f(mins.x, mins.y, mins.z);
    glVertex3f(maxs.x, mins.y, mins.z);
    glVertex3f(maxs.x, maxs.y, mins.z);
    glVertex3f(mins.x, maxs.y, mins.z);

    // Back face
    glVertex3f(mins.x, mins.y, maxs.z);
    glVertex3f(maxs.x, mins.y, maxs.z);
    glVertex3f(maxs.x, maxs.y, maxs.z);
    glVertex3f(mins.x, maxs.y, maxs.z);

    // Edges
    glVertex3f(mins.x, mins.y, mins.z);
    glVertex3f(mins.x, mins.y, maxs.z);
    glVertex3f(maxs.x, mins.y, mins.z);
    glVertex3f(maxs.x, mins.y, maxs.z);
    glVertex3f(maxs.x, maxs.y, mins.z);
    glVertex3f(maxs.x, maxs.y, maxs.z);
    glVertex3f(mins.x, maxs.y, mins.z);
    glVertex3f(mins.x, maxs.y, maxs.z);

    glEnd();
    glEnable(GL_LIGHTING);
}

// Render lighting pass
static void R_RenderLighting() {
    // Setup default light
    float light_pos[] = {100.0f, 100.0f, 200.0f, 1.0f};
    float light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float light_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    float light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // TODO: Render dynamic lights
}

// Render HUD/UI overlays
static void R_RenderUI() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, (float)r_state.screen_width, (float)r_state.screen_height, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Draw simple text/crosshair (TODO: actual HUD implementation)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Crosshair
    float cx = r_state.screen_width / 2.0f;
    float cy = r_state.screen_height / 2.0f;
    glBegin(GL_LINES);
    glVertex2f(cx - 10.0f, cy);
    glVertex2f(cx + 10.0f, cy);
    glVertex2f(cx, cy - 10.0f);
    glVertex2f(cx, cy + 10.0f);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Main render frame
void R_RenderFrame(const glm::vec3& camera_pos, const glm::vec3& camera_dir) {
    r_state.frame_count++;

    // Set camera
    R_SetCamera(camera_pos, camera_dir, {0, 0, 1});
    R_UpdateProjection();

    // Clear buffers
    R_Clear();

    // Render world
    R_DrawWorld();
    R_DrawEntities();

    // Lighting
    R_RenderLighting();

    // UI overlays
    R_RenderUI();

    // Stats
    if (r_stats.frame_ms > 0) {
        Com_DPrintf("Render: %.2f ms, Surfaces: %d, Triangles: %d\n", 
                    r_stats.frame_ms, r_stats.surfaces_rendered, r_stats.triangles_rendered);
    }
}

// Resize viewport
void R_Resize(int width, int height) {
    if (width <= 0 || height <= 0) return;

    r_state.screen_width = width;
    r_state.screen_height = height;
    glViewport(0, 0, width, height);
    R_UpdateProjection();

    Com_Printf("Renderer resized to %dx%d\n", width, height);
}

// Shutdown renderer
void R_Shutdown() {
    Com_Printf("Shutting down OpenGL renderer...\n");
    // Clean up resources
}

// Get render statistics
render_stats_t R_GetStats() {
    return r_stats;
}

// Set field of view
void R_SetFOV(float fov) {
    r_state.fov = fov;
    R_UpdateProjection();
}

// Enable/disable wireframe mode
void R_SetWireframe(bool enable) {
    if (enable) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

}  // namespace render
