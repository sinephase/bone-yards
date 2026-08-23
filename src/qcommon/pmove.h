#pragma once

#include "math.h"
#include "collision.h"
#include <stdint.h>
#include <stdbool.h>

/* Player movement constants */
#define PM_NORMAL     0
#define PM_DEAD       1
#define PM_SPECTATOR  2
#define PM_FREEZE     3

#define PMF_ON_GROUND 0x0001
#define PMF_JUMP      0x0002
#define PMF_CROUCH    0x0004
#define PMF_SPRINT    0x0008

#define STEPSIZE      18.0f
#define MAXMOVE       16.0f

typedef struct {
    vec3_t origin;
    vec3_t velocity;
    uint32_t pm_type;
    uint32_t pm_flags;
    float pm_time;
    float stamina;
    bool exhausted;
} pstate_t;

typedef struct {
    pstate_t *state;
    vec3_t origin;
    vec3_t velocity;
    vec3_t mins, maxs;
    vec3_t forward, right, up;
    
    uint32_t buttons;
    float msec;
    
    trace_t (*trace_fn)(const vec3_t, const vec3_t, const vec3_t, const vec3_t, uint32_t);
    uint32_t (*contents_fn)(const vec3_t);
} pmove_t;

void Pmove(pmove_t *pm);
