#pragma once

#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>

struct edict_t;

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
#define CONTENTS_ITEM         1024
#define CONTENTS_WEAPON       2048

/* Trace masks */
#define MASK_ALL              (~0)
#define MASK_SOLID            (CONTENTS_SOLID | CONTENTS_WINDOW)
#define MASK_PLAYERSOLID      (CONTENTS_SOLID | CONTENTS_MONSTER)
#define MASK_MONSTERSOLID     (CONTENTS_SOLID | CONTENTS_MONSTER)
#define MASK_SHOT             (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_WINDOW)
#define MASK_OPAQUE           (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA)
#define MASK_ITEM             (CONTENTS_ITEM)
#define MASK_WEAPON           (CONTENTS_WEAPON)

namespace engine {

/* Plane */
struct plane_t {
    glm::vec3 normal{0.0f};
    float dist = 0.0f;
    uint8_t type = 0;      /* 0-2 = axial, 3 = general */
    uint8_t signbits = 0;  /* For fast box on plane side test */
};

/* Brush */
struct brush_t {
    std::vector<plane_t> planes;
    uint32_t contents = 0;
    uint32_t surfaceFlags = 0;
    glm::vec3 mins{0.0f};
    glm::vec3 maxs{0.0f};
};

/* Trace result */
struct surface_t {
    uint32_t flags = 0;
};

struct trace_t {
    bool allsolid = false;     /* If true, plane is not valid */
    bool startsolid = false;   /* If true, the initial point was in a solid area */
    float fraction = 1.0f;     /* Time completed, 1.0 = didn't hit anything */
    glm::vec3 endpos{0.0f};    /* Final position */
    plane_t plane;             /* Surface normal and distance */
    surface_t surface;
    uint32_t contents = 0;
    edict_t *ent = nullptr;    /* Entity touched, if any */
};

/* Collision model */
struct cmodel_t {
    std::vector<brush_t> brushes;
};

/* Collision functions */
trace_t CM_BoxTrace(const glm::vec3& start, const glm::vec3& end,
                     const glm::vec3& mins, const glm::vec3& maxs,
                     const std::vector<brush_t>& brushes, int contentmask);
int CM_PointContents(const glm::vec3& p, const std::vector<brush_t>& brushes);

void CM_ClipBoxToBrush(const glm::vec3& mins, const glm::vec3& maxs,
                        const glm::vec3& start, const glm::vec3& end,
                        trace_t* trace, const brush_t& brush);

trace_t CM_RayBox(const glm::vec3& origin, const glm::vec3& dir,
                   const glm::vec3& mins, const glm::vec3& maxs);

void CM_BuildBrushPlanes(brush_t& brush);
brush_t CM_MakeBoxBrush(const glm::vec3& mins, const glm::vec3& maxs, int contents);

bool CM_SphereBoxIntersect(const glm::vec3& sphere_center, float sphere_radius,
                           const glm::vec3& box_mins, const glm::vec3& box_maxs);
bool CM_BoxesIntersect(const glm::vec3& mins1, const glm::vec3& maxs1,
                        const glm::vec3& mins2, const glm::vec3& maxs2);

}  // namespace engine
