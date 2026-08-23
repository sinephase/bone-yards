#pragma once

#include "math.h"
#include <stdint.h>

/* Content flags */
#define CONTENTS_SOLID        1
#define CONTENTS_WINDOW       2
#define CONTENTS_AUX          4
#define CONTENTS_LAVA         8
#define CONTENTS_SLIME        16
#define CONTENTS_WATER        32
#define CONTENTS_MIST         64
#define CONTENTS_MONSTER      128
#define CONTENTS_DEADMONSTER  256
#define CONTENTS_TRIGGER      512

/* Trace masks */
#define MASK_ALL              (~0)
#define MASK_SOLID            (CONTENTS_SOLID | CONTENTS_WINDOW)
#define MASK_PLAYERSOLID      (CONTENTS_SOLID | CONTENTS_MONSTER)
#define MASK_MONSTERSOLID     (CONTENTS_SOLID | CONTENTS_MONSTER)
#define MASK_SHOT             (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_WINDOW)
#define MASK_OPAQUE           (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA)

/* Plane */
typedef struct plane_s {
    vec3_t normal;
    float dist;
    uint8_t type;      /* 0-2 = axial, 3 = general */
    uint8_t signbits;  /* For fast box on plane side test */
} plane_t;

/* Brush */
typedef struct brush_s {
    plane_t *planes;
    int numplanes;
    uint32_t contents;
    uint32_t surfaceFlags;
    char *textures[6];
    vec3_t mins, maxs;
} brush_t;

/* Trace result */
typedef struct {
    bool allsolid;     /* If true, plane is not valid */
    bool startsolid;   /* If true, the initial point was in a solid area */
    float fraction;    /* Time completed, 1.0 = didn't hit anything */
    vec3_t endpos;     /* Final position */
    plane_t plane;     /* Surface normal and distance */
    uint32_t surfaceFlags;
    uint32_t contents;
    struct edict_s *ent;  /* Entity touched, if any */
} trace_t;

/* Collision model */
typedef struct cm_s {
    brush_t *brushes;
    int numbrushes;
} cmodel_t;

/* Collision functions */
void CM_BuildBrushes(brush_t *brushes, int count);
trace_t CM_BoxTrace(const vec3_t start, const vec3_t end,
                     const vec3_t mins, const vec3_t maxs,
                     const brush_t *brushes, int numbrushes,
                     uint32_t contentmask);
uint32_t CM_PointContents(const vec3_t p, const brush_t *brushes, int numbrushes);
