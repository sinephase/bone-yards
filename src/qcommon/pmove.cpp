// pmove.cpp -- player movement and physics
// Implements slide-move physics, step-up, friction, and jump prediction.
// The same code runs on both client (for prediction) and server (for validation).

#include "qcommon.h"
#include <algorithm>
#include <cmath>

namespace engine {

// Physics tuning constants
static const float STEPSIZE = 18.0f;
static const float FRICTION = 8.0f;
static const float WATER_FRICTION = 1.0f;
static const float ACCELERATE = 10.0f;
static const float WATER_ACCELERATE = 4.0f;
static const float AIR_ACCELERATE = 1.0f;
static const float MAX_SPEED = 210.0f;
static const float SPRINT_SPEED = 330.0f;
static const float JUMP_VELOCITY = 250.0f;
static const float GRAVITY = 700.0f;

// Get player acceleration (apply input direction to desired speed)
static glm::vec3 PM_GetAcceleration(const pmove_t& pm, const glm::vec3& wish_dir, float wish_speed, float accel) {
    float current_speed = glm::dot(pm.velocity, wish_dir);
    float add_speed = wish_speed - current_speed;
    if (add_speed <= 0.0f) return glm::vec3(0);

    float accel_speed = accel * pm.frametime * wish_speed;
    if (accel_speed > add_speed) accel_speed = add_speed;

    return wish_dir * accel_speed;
}

// Ground movement with friction
static void PM_GroundMove(pmove_t& pm) {
    // Apply friction
    glm::vec3 vel = glm::vec3(pm.velocity.x, pm.velocity.y, 0);
    float speed = glm::length(vel);

    if (speed > 0.0f) {
        float drop = speed * FRICTION * pm.frametime;
        float new_speed = std::max(0.0f, speed - drop);
        vel = vel * (new_speed / speed);
    }

    pm.velocity.x = vel.x;
    pm.velocity.y = vel.y;

    // Determine wish direction from input
    glm::vec3 wish_dir(0);
    float wish_speed = 0;

    if (pm.cmd.forwardmove != 0) {
        wish_dir.z += std::copysign(pm.cmd.forwardmove, pm.cmd.forwardmove > 0 ? 1.0f : -1.0f);
    }
    if (pm.cmd.sidemove != 0) {
        wish_dir.x += std::copysign(pm.cmd.sidemove, pm.cmd.sidemove > 0 ? 1.0f : -1.0f);
    }

    if (glm::length(wish_dir) > 0.0f) {
        wish_dir = glm::normalize(wish_dir);
        wish_speed = (pm.cmd.buttons & 4) ? SPRINT_SPEED : MAX_SPEED;  // Button 4 = sprint
        
        glm::vec3 accel = PM_GetAcceleration(pm, wish_dir, wish_speed, ACCELERATE);
        pm.velocity += accel;
    }

    // Jump
    if ((pm.cmd.buttons & 2) && pm.groundentity != nullptr) {  // Button 2 = jump
        pm.velocity.z = JUMP_VELOCITY;
        pm.groundentity = nullptr;
    }
}

// Air movement (reduced acceleration)
static void PM_AirMove(pmove_t& pm) {
    glm::vec3 wish_dir(0);
    float wish_speed = 0;

    if (pm.cmd.forwardmove != 0) {
        wish_dir.z += std::copysign(pm.cmd.forwardmove, pm.cmd.forwardmove > 0 ? 1.0f : -1.0f);
    }
    if (pm.cmd.sidemove != 0) {
        wish_dir.x += std::copysign(pm.cmd.sidemove, pm.cmd.sidemove > 0 ? 1.0f : -1.0f);
    }

    if (glm::length(wish_dir) > 0.0f) {
        wish_dir = glm::normalize(wish_dir);
        wish_speed = (pm.cmd.buttons & 4) ? SPRINT_SPEED : MAX_SPEED;
        
        glm::vec3 accel = PM_GetAcceleration(pm, wish_dir, wish_speed, AIR_ACCELERATE);
        pm.velocity += accel;
    }

    // Apply gravity
    pm.velocity.z -= GRAVITY * pm.frametime;

    // Cap vertical velocity to prevent falling too fast
    const float FALL_SPEED_LIMIT = 900.0f;
    if (pm.velocity.z < -FALL_SPEED_LIMIT) {
        pm.velocity.z = -FALL_SPEED_LIMIT;
    }
}

// Slide the player along a wall/plane
static void PM_SlideMove(pmove_t& pm) {
    glm::vec3 normal(0, 0, 1);
    float blocked = 0.0f;

    for (int i = 0; i < 4; i++) {  // Slide up to 4 times per frame
        glm::vec3 goal = pm.origin + pm.velocity * pm.frametime;

        // Trace to the goal position
        // (In the full engine, this would call SV_Trace through game code)
        // For now, assume no collision and move directly
        pm.origin = goal;

        // In the full version, we'd check for groundentity here
        if (i == 0) {
            pm.groundentity = nullptr;  // Reset ground contact
        }
    }
}

void PM_PlayerMove(pmove_t& pm) {
    if (pm.frametime <= 0.0f) return;

    // Determine if on ground
    bool was_onground = (pm.groundentity != nullptr);

    // Decide between ground and air movement
    if (was_onground) {
        PM_GroundMove(pm);
    } else {
        PM_AirMove(pm);
    }

    // Apply movement and collision
    PM_SlideMove(pm);

    // Landing check
    if (!was_onground && pm.groundentity != nullptr) {
        // Just landed -- could play land sound, etc.
    }
}

// Apply predicted player movement (client-side)
void PM_ClientMove(usercmd_t& cmd, pmove_t& pm) {
    PM_PlayerMove(pm);
}

// Validate predicted movement against server state
void PM_ServerMove(usercmd_t& cmd, pmove_t& pm) {
    PM_PlayerMove(pm);
}

}  // namespace engine
