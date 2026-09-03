#pragma once

#include "../qcommon.h"
#include "../qcommon/collision.h"
#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include <glm/glm.hpp>

/* Entity solid types */
#define SOLID_NOT       0
#define SOLID_TRIGGER   1
#define SOLID_BBOX      2
#define SOLID_BSP       3

namespace engine {

/* World state */
struct world_t {
    std::vector<brush_t> brushes;
    edict_t *player = nullptr;
    edict_t *worldspawn = nullptr;
};

/* World functions */
void SV_LinkEdict(edict_t *ent);
void SV_UnlinkEdict(edict_t *ent);
std::vector<edict_t *> SV_AreaEdicts(const glm::vec3& mins, const glm::vec3& maxs);

trace_t SV_Trace(const glm::vec3& start, const glm::vec3& mins, const glm::vec3& maxs,
                  const glm::vec3& end, edict_t *passent, int contentmask);
int SV_PointContents(const glm::vec3& p);
bool SV_Visible(const glm::vec3& from, const glm::vec3& to, edict_t *passent);

void SV_InitWorld();
void SV_ClearWorld();

}  // namespace engine
