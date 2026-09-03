#pragma once

#include "math.h"
#include <stdint.h>
#include <stdbool.h>
#include <glm/glm.hpp>

struct edict_t;

namespace engine {

/* Player movement constants */
#define PM_NORMAL     0
#define PM_DEAD       1
#define PM_SPECTATOR  2
#define PM_FREEZE     3

#define STEPSIZE      18.0f
#define MAXMOVE       16.0f

/* Movement input buttons */
#define BUTTON_JUMP   0x0001
#define BUTTON_SPRINT 0x0002

/* Per-frame tunable movement parameters */
struct pm_tunables_t {
    float max_speed;
    float sprint_speed;
    float jump_velocity;
    float gravity;
    float step_size;
    float friction;
};

extern pm_tunables_t pm_tunable;

/* Player input command for a single simulation frame */
struct usercmd_t {
    int forwardmove;
    int sidemove;
    int upmove;
    uint32_t buttons;
};

/* Player movement state, updated in place by PM_Move() */
struct pmove_state_t {
    glm::vec3 origin;
    glm::vec3 velocity;
    vec3_t viewangles;
    int waterlevel;
    bool onground;
    float stamina;
    edict_t *groundentity;
    usercmd_t cmd;
    float frametime;
};

void PM_Move(pmove_state_t *ps, const usercmd_t *cmd, float frametime);

}  // namespace engine
