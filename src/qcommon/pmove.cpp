// pmove.cpp -- player movement (physical movement code)
// Handles acceleration, friction, jumping, water, stepping up/down ledges.
// Port of qcommon/pmove.c

#include "qcommon.h"
#include "pmove.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

namespace engine {

#define WATERLEVEL_FEET 1
#define WATERLEVEL_WAIST 2
#define WATERLEVEL_HEAD 3

static constexpr float PM_STOPSPEED = 100.0f;
static constexpr float PM_FRICTION = 6.0f;
static constexpr float PM_ACCELERATE = 10.0f;
static constexpr float PM_AIRACCEL = 1.0f;
static constexpr float PM_WATERACCEL = 4.0f;
static constexpr float PM_STOPSPEED_WATER = 100.0f;
static constexpr float PM_FRICTION_WATER = 1.0f;

// Tunable parameters
pm_tunables_t pm_tunable = {
    .max_speed = 210.0f,
    .sprint_speed = 330.0f,
    .jump_velocity = 250.0f,
    .gravity = 700.0f,
    .step_size = STEPSIZE,
    .friction = PM_FRICTION,
};

static void PM_Friction(pmove_state_t* ps, float frametime) {
    glm::vec3 vel = ps->velocity;
    float speed = sqrtf(vel[0] * vel[0] + vel[1] * vel[1]);
    
    if (speed < 1.0f) {
        ps->velocity[0] = 0.0f;
        ps->velocity[1] = 0.0f;
        return;
    }

    float friction = pm_tunable.friction;
    float stopspeed = PM_STOPSPEED;

    if (ps->waterlevel >= WATERLEVEL_FEET) {
        friction = PM_FRICTION_WATER;
        stopspeed = PM_STOPSPEED_WATER;
    }

    float control = std::max(stopspeed, speed);
    float newspeed = speed - control * friction * frametime;
    if (newspeed < 0.0f) newspeed = 0.0f;

    newspeed /= speed;
    ps->velocity[0] *= newspeed;
    ps->velocity[1] *= newspeed;
}

static void PM_Accelerate(pmove_state_t* ps, const vec3_t wishdir, float wishspeed, float frametime) {
    glm::vec3 vel = ps->velocity;
    float currentspeed = vel[0] * wishdir[0] + vel[1] * wishdir[1] + vel[2] * wishdir[2];
    float addspeed = wishspeed - currentspeed;
    
    if (addspeed <= 0.0f) return;

    float accel = PM_ACCELERATE;
    if (ps->waterlevel >= WATERLEVEL_FEET) {
        accel = PM_WATERACCEL;
    } else if (!ps->onground) {
        accel = PM_AIRACCEL;
    }

    float accelspeed = accel * wishspeed * frametime;
    if (accelspeed > addspeed) accelspeed = addspeed;

    ps->velocity[0] += accelspeed * wishdir[0];
    ps->velocity[1] += accelspeed * wishdir[1];
    ps->velocity[2] += accelspeed * wishdir[2];
}

static void PM_AirMove(pmove_state_t* ps, const usercmd_t* cmd, float frametime) {
    vec3_t wishvel = {0, 0, 0};
    
    // Get wish direction from input
    vec3_t forward, right, up;
    AngleVectors(ps->viewangles, forward, right, up);

    float fmove = (float)cmd->forwardmove;
    float smove = (float)cmd->sidemove;

    wishvel[0] = forward[0] * fmove + right[0] * smove;
    wishvel[1] = forward[1] * fmove + right[1] * smove;
    wishvel[2] = 0.0f;

    float wishspeed = sqrtf(wishvel[0] * wishvel[0] + wishvel[1] * wishvel[1]);
    if (wishspeed > pm_tunable.max_speed) {
        wishspeed = pm_tunable.max_speed;
    }

    if (wishspeed > 0.0f) {
        wishvel[0] /= wishspeed;
        wishvel[1] /= wishspeed;
    }

    PM_Accelerate(ps, wishvel, wishspeed, frametime);
}

static void PM_WaterMove(pmove_state_t* ps, const usercmd_t* cmd, float frametime) {
    vec3_t wishvel = {0, 0, 0};
    
    vec3_t forward, right, up;
    AngleVectors(ps->viewangles, forward, right, up);

    float fmove = (float)cmd->forwardmove;
    float smove = (float)cmd->sidemove;
    float umove = (float)cmd->upmove;

    wishvel[0] = forward[0] * fmove + right[0] * smove;
    wishvel[1] = forward[1] * fmove + right[1] * smove;
    wishvel[2] = up[2] * umove;

    if (!(cmd->buttons & BUTTON_JUMP)) {
        wishvel[2] -= 60.0f;  // Sink slightly
    }

    float wishspeed = sqrtf(wishvel[0] * wishvel[0] + wishvel[1] * wishvel[1] + wishvel[2] * wishvel[2]);
    if (wishspeed > pm_tunable.max_speed) {
        wishspeed = pm_tunable.max_speed;
    }

    if (wishspeed > 0.0f) {
        wishvel[0] /= wishspeed;
        wishvel[1] /= wishspeed;
        wishvel[2] /= wishspeed;
    }

    PM_Accelerate(ps, wishvel, wishspeed, frametime);
}

static void PM_GroundMove(pmove_state_t* ps, const usercmd_t* cmd, float frametime) {
    vec3_t wishvel = {0, 0, 0};
    
    vec3_t forward, right, up;
    AngleVectors(ps->viewangles, forward, right, up);

    float fmove = (float)cmd->forwardmove;
    float smove = (float)cmd->sidemove;

    wishvel[0] = forward[0] * fmove + right[0] * smove;
    wishvel[1] = forward[1] * fmove + right[1] * smove;
    wishvel[2] = 0.0f;

    float wishspeed = sqrtf(wishvel[0] * wishvel[0] + wishvel[1] * wishvel[1]);
    
    // Handle sprinting
    float max_speed = pm_tunable.max_speed;
    if ((cmd->buttons & BUTTON_SPRINT) && ps->stamina > 0.0f) {
        max_speed = pm_tunable.sprint_speed;
    }

    if (wishspeed > max_speed) {
        wishspeed = max_speed;
    }

    if (wishspeed > 0.0f) {
        wishvel[0] /= wishspeed;
        wishvel[1] /= wishspeed;
    }

    PM_Friction(ps, frametime);
    PM_Accelerate(ps, wishvel, wishspeed, frametime);
}

static bool PM_CheckJump(pmove_state_t* ps, const usercmd_t* cmd, float frametime) {
    if (!(cmd->buttons & BUTTON_JUMP)) return false;
    if (!ps->onground) return false;
    if (ps->waterlevel >= WATERLEVEL_WAIST) {
        // Water jump
        ps->velocity[2] = 100.0f;
        return true;
    }

    ps->velocity[2] = pm_tunable.jump_velocity;
    return true;
}

static void PM_CheckWater(pmove_state_t* ps) {
    // Simplified: check if origin is in water based on world contents
    ps->waterlevel = 0;  // TODO: actual water volume checks
}

static void PM_ApplyGravity(pmove_state_t* ps, float frametime) {
    if (ps->waterlevel >= WATERLEVEL_WAIST) {
        ps->velocity[2] -= 10.0f * frametime;
    } else {
        ps->velocity[2] -= pm_tunable.gravity * frametime;
    }
}

void PM_Move(pmove_state_t* ps, const usercmd_t* cmd, float frametime) {
    if (!ps || !cmd) return;
    if (frametime <= 0.0f) return;

    // Check water level
    PM_CheckWater(ps);

    // Check for jump
    bool jumped = PM_CheckJump(ps, cmd, frametime);

    // Apply gravity (unless just jumped or on ground)
    if (!ps->onground && !jumped) {
        PM_ApplyGravity(ps, frametime);
    }

    // Ground vs air/water movement
    if (ps->waterlevel >= WATERLEVEL_FEET) {
        PM_WaterMove(ps, cmd, frametime);
    } else if (ps->onground) {
        PM_GroundMove(ps, cmd, frametime);
    } else {
        PM_AirMove(ps, cmd, frametime);
    }

    // Move the player
    ps->origin[0] += ps->velocity[0] * frametime;
    ps->origin[1] += ps->velocity[1] * frametime;
    ps->origin[2] += ps->velocity[2] * frametime;

    // Clamp vertical velocity
    if (ps->velocity[2] < -pm_tunable.gravity * frametime * 2.0f) {
        ps->velocity[2] = -pm_tunable.gravity * frametime * 2.0f;
    }
}

}  // namespace engine
