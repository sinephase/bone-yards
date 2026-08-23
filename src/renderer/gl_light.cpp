// gl_light.cpp -- dynamic lighting and shadow mapping
// Manages light sources, calculates lighting for surfaces and entities,
// and implements shadow mapping for realistic shadows.

#include "qcommon.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>
#include <vector>
#include <algorithm>

namespace render {

// Light types
#define LIGHT_POINT     0
#define LIGHT_SPOT      1
#define LIGHT_DIRECT    2

// Light structure
struct light_t {
    glm::vec3 origin;
    glm::vec3 color;
    float intensity;
    float radius;
    int type;
    glm::vec3 direction;  // For spot/directional lights
    float angle;          // Spot light angle
    bool cast_shadow;
    GLuint shadow_map;
    glm::mat4 shadow_matrix;
};

// Lighting state
static constexpr int MAX_LIGHTS = 32;
static light_t lights[MAX_LIGHTS];
static int num_lights = 0;

// Shadow map framebuffer
static GLuint shadow_fbo = 0;
static GLuint shadow_depth_texture = 0;
static constexpr int SHADOW_MAP_SIZE = 2048;

// Initialize shadow mapping
void L_InitShadows() {
    Com_Printf("Initializing shadow mapping...\n");

    // Create framebuffer
    glGenFramebuffers(1, &shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);

    // Create depth texture
    glGenTextures(1, &shadow_depth_texture);
    glBindTexture(GL_TEXTURE_2D, shadow_depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // Attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
                          shadow_depth_texture, 0);

    // No color output
    GLenum draw_buffers[] = {GL_NONE};
    glDrawBuffers(1, draw_buffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Com_Printf("Error: Shadow framebuffer not complete\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Add a light to the world
int L_AddLight(const glm::vec3& origin, const glm::vec3& color, float intensity, 
               float radius, int type, bool cast_shadow) {
    if (num_lights >= MAX_LIGHTS) {
        Com_Printf("Warning: Max lights exceeded\n");
        return -1;
    }

    light_t& light = lights[num_lights];
    light.origin = origin;
    light.color = color;
    light.intensity = intensity;
    light.radius = radius;
    light.type = type;
    light.cast_shadow = cast_shadow;
    light.direction = {0, 0, -1};
    light.angle = 45.0f;

    Com_Printf("Light added at %.0f %.0f %.0f (intensity: %.2f, radius: %.0f)\n",
               origin.x, origin.y, origin.z, intensity, radius);

    return num_lights++;
}

// Remove a light
void L_RemoveLight(int light_id) {
    if (light_id < 0 || light_id >= num_lights) return;

    // Swap with last light
    lights[light_id] = lights[num_lights - 1];
    num_lights--;
}

// Get light info
const light_t* L_GetLight(int light_id) {
    if (light_id < 0 || light_id >= num_lights) return nullptr;
    return &lights[light_id];
}

// Clear all lights
void L_ClearLights() {
    num_lights = 0;
}

// Calculate lighting contribution at a point
glm::vec3 L_CalculateLighting(const glm::vec3& point, const glm::vec3& normal) {
    glm::vec3 result = {0.1f, 0.1f, 0.1f};  // Ambient

    for (int i = 0; i < num_lights; i++) {
        const light_t& light = lights[i];

        // Vector from point to light
        glm::vec3 to_light = light.origin - point;
        float distance = glm::length(to_light);

        // Skip if beyond range
        if (distance > light.radius) continue;

        to_light = glm::normalize(to_light);

        // Diffuse lighting
        float diffuse = std::max(0.0f, glm::dot(normal, to_light));

        // Distance attenuation
        float attenuation = 1.0f - (distance / light.radius);
        attenuation = std::max(0.0f, attenuation);
        attenuation *= attenuation;  // Quadratic falloff

        // Add light contribution
        glm::vec3 contribution = light.color * light.intensity * diffuse * attenuation;
        result += contribution;
    }

    return result;
}

// Render shadow map for a light
void L_RenderShadowMap(int light_id) {
    if (light_id < 0 || light_id >= num_lights) return;

    light_t& light = lights[light_id];
    if (!light.cast_shadow) return;

    Com_DPrintf("Rendering shadow map for light %d\n", light_id);

    // Bind shadow framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Setup projection from light's perspective
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    float aspect = 1.0f;
    glm::mat4 projection;

    if (light.type == LIGHT_POINT) {
        // Point light uses omnidirectional shadows (6 cube maps)
        // For simplicity, render as directional from light center
        projection = glm::ortho(-light.radius, light.radius, 
                               -light.radius, light.radius, 
                               0.1f, light.radius * 2.0f);
    } else if (light.type == LIGHT_SPOT) {
        projection = glm::perspective(glm::radians(light.angle * 2.0f), aspect, 
                                     0.1f, light.radius);
    } else {
        // Directional light
        projection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 0.1f, 4000.0f);
    }

    glLoadMatrixf(glm::value_ptr(projection));

    // View matrix from light
    glm::vec3 light_look = light.origin + light.direction;
    glm::mat4 view = glm::lookAt(light.origin, light_look, {0, 0, 1});

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(view));

    // Store shadow matrix for later
    light.shadow_matrix = projection * view;

    // TODO: Render scene geometry to shadow map

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Check if point is in shadow
bool L_IsInShadow(const glm::vec3& point, int light_id) {
    if (light_id < 0 || light_id >= num_lights) return false;

    const light_t& light = lights[light_id];
    if (!light.cast_shadow) return false;

    // Transform point to light's shadow space
    glm::vec4 shadow_coord = light.shadow_matrix * glm::vec4(point, 1.0f);
    shadow_coord /= shadow_coord.w;

    // Normalize to [0, 1]
    shadow_coord.x = shadow_coord.x * 0.5f + 0.5f;
    shadow_coord.y = shadow_coord.y * 0.5f + 0.5f;
    shadow_coord.z = shadow_coord.z * 0.5f + 0.5f;

    // Check bounds
    if (shadow_coord.x < 0.0f || shadow_coord.x > 1.0f ||
        shadow_coord.y < 0.0f || shadow_coord.y > 1.0f ||
        shadow_coord.z < 0.0f || shadow_coord.z > 1.0f) {
        return false;
    }

    // Sample shadow map
    glBindTexture(GL_TEXTURE_2D, light.shadow_map);
    // TODO: Perform shadow texture lookup
    // For now, assume not in shadow
    return false;
}

// Get number of active lights
int L_GetLightCount() {
    return num_lights;
}

// Set light properties
void L_SetLightOrigin(int light_id, const glm::vec3& origin) {
    if (light_id < 0 || light_id >= num_lights) return;
    lights[light_id].origin = origin;
}

void L_SetLightColor(int light_id, const glm::vec3& color) {
    if (light_id < 0 || light_id >= num_lights) return;
    lights[light_id].color = color;
}

void L_SetLightIntensity(int light_id, float intensity) {
    if (light_id < 0 || light_id >= num_lights) return;
    lights[light_id].intensity = intensity;
}

void L_SetLightRadius(int light_id, float radius) {
    if (light_id < 0 || light_id >= num_lights) return;
    lights[light_id].radius = radius;
}

// Light flicker effect (for torches, explosions, etc.)
void L_FlickerLight(int light_id, float intensity_base, float flicker_amount, float speed) {
    if (light_id < 0 || light_id >= num_lights) return;

    light_t& light = lights[light_id];
    float time = (float)SDL_GetTicks() / 1000.0f;
    float flicker = sinf(time * speed) * flicker_amount;
    light.intensity = intensity_base + flicker;
}

// Pulsing light effect
void L_PulseLight(int light_id, float intensity_min, float intensity_max, float speed) {
    if (light_id < 0 || light_id >= num_lights) return;

    light_t& light = lights[light_id];
    float time = (float)SDL_GetTicks() / 1000.0f;
    float pulse = (sinf(time * speed) + 1.0f) * 0.5f;
    light.intensity = glm::mix(intensity_min, intensity_max, pulse);
}

// Shutdown lighting system
void L_Shutdown() {
    Com_Printf("Shutting down lighting system...\n");

    if (shadow_fbo != 0) {
        glDeleteFramebuffers(1, &shadow_fbo);
    }

    if (shadow_depth_texture != 0) {
        glDeleteTextures(1, &shadow_depth_texture);
    }
}

// Debug visualization of lights
void L_DebugDrawLights() {
    glDisable(GL_LIGHTING);
    glPointSize(8.0f);

    glBegin(GL_POINTS);
    for (int i = 0; i < num_lights; i++) {
        const light_t& light = lights[i];
        glColor3f(light.color.r, light.color.g, light.color.b);
        glVertex3f(light.origin.x, light.origin.y, light.origin.z);
    }
    glEnd();

    glPointSize(1.0f);
    glEnable(GL_LIGHTING);
}

}  // namespace render
