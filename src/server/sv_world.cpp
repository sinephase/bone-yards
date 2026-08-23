// sv_world.cpp -- world entity management and spatial queries
// Maintains the entity grid for fast spatial lookups, links/unlinks entities,
// and performs collision traces against the entity and brush geometry.

#include "qcommon.h"
#include <algorithm>
#include <cmath>

namespace engine {

// World entity grid for fast spatial queries
// Divides the world into a coarse grid of cells
static constexpr int WORLD_GRID_SIZE = 128;  // Units per grid cell
static constexpr int WORLD_GRID_CELLS = 512;  // 512x512 cells = 65536x65536 units
static std::vector<std::vector<edict_t*>> world_grid[WORLD_GRID_CELLS][WORLD_GRID_CELLS];

static int CM_GridFromX(float x) {
    int grid = (int)(x / WORLD_GRID_SIZE);
    return std::clamp(grid, 0, WORLD_GRID_CELLS - 1);
}

static int CM_GridFromY(float y) {
    int grid = (int)(y / WORLD_GRID_SIZE);
    return std::clamp(grid, 0, WORLD_GRID_CELLS - 1);
}

// Link an entity into the world grid
void SV_LinkEdict(edict_t* ent) {
    if (!ent || ent->solid == SOLID_NOT) return;

    // Calculate bounding box
    ent->absmin = ent->origin + ent->mins;
    ent->absmax = ent->origin + ent->maxs;

    // Find which grid cells this entity occupies
    int minx = CM_GridFromX(ent->absmin.x);
    int maxx = CM_GridFromX(ent->absmax.x);
    int miny = CM_GridFromY(ent->absmin.y);
    int maxy = CM_GridFromY(ent->absmax.y);

    // Add to all overlapping cells
    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            world_grid[x][y].push_back(ent);
        }
    }

    ent->linkcount++;
}

// Unlink an entity from the world grid
void SV_UnlinkEdict(edict_t* ent) {
    if (!ent) return;

    // Find and remove from all grid cells
    int minx = CM_GridFromX(ent->absmin.x);
    int maxx = CM_GridFromX(ent->absmax.x);
    int miny = CM_GridFromY(ent->absmin.y);
    int maxy = CM_GridFromY(ent->absmax.y);

    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            auto& cell = world_grid[x][y];
            auto it = std::find(cell.begin(), cell.end(), ent);
            if (it != cell.end()) {
                cell.erase(it);
            }
        }
    }
}

// Get all entities in an area
std::vector<edict_t*> SV_AreaEdicts(const glm::vec3& mins, const glm::vec3& maxs) {
    std::vector<edict_t*> result;
    std::unordered_set<edict_t*> seen;  // Avoid duplicates from overlapping cells

    int minx = CM_GridFromX(mins.x);
    int maxx = CM_GridFromX(maxs.x);
    int miny = CM_GridFromY(mins.y);
    int maxy = CM_GridFromY(maxs.y);

    for (int x = minx; x <= maxx; x++) {
        for (int y = miny; y <= maxy; y++) {
            for (edict_t* ent : world_grid[x][y]) {
                if (seen.find(ent) == seen.end()) {
                    result.push_back(ent);
                    seen.insert(ent);
                }
            }
        }
    }

    return result;
}

// Trace a box through the world against entities and brushes
trace_t SV_Trace(const glm::vec3& start, const glm::vec3& mins, const glm::vec3& maxs,
                 const glm::vec3& end, edict_t* passent, int contentmask) {
    trace_t trace;
    trace.fraction = 1.0f;
    trace.endpos = end;
    trace.ent = nullptr;
    trace.plane.normal = {0, 0, 1};
    trace.plane.dist = 0.0f;

    // Trace against world brushes first
    // TODO: Get brushes from worldspawn entity
    std::vector<brush_t> empty_brushes;
    trace = CM_BoxTrace(start, end, mins, maxs, empty_brushes, contentmask);

    // Now trace against entities
    glm::vec3 area_mins = glm::min(start, end) + mins;
    glm::vec3 area_maxs = glm::max(start, end) + maxs;
    std::vector<edict_t*> entities = SV_AreaEdicts(area_mins, area_maxs);

    for (edict_t* ent : entities) {
        if (ent == passent) continue;  // Skip the entity we're tracing from
        if (ent->solid == SOLID_NOT) continue;
        if (!(ent->contents & contentmask)) continue;

        // Trace against this entity's bounding box
        std::vector<brush_t> ent_brushes;
        brush_t ent_brush = CM_MakeBoxBrush(ent->mins, ent->maxs, ent->contents);
        ent_brushes.push_back(ent_brush);

        trace_t ent_trace = CM_BoxTrace(start, end, mins, maxs, ent_brushes, contentmask);
        
        if (ent_trace.fraction < trace.fraction) {
            trace = ent_trace;
            trace.ent = ent;
        }

        if (trace.fraction == 0.0f) break;  // Hit solid, stop
    }

    trace.endpos = glm::mix(start, end, trace.fraction);
    return trace;
}

// Get point contents
int SV_PointContents(const glm::vec3& p) {
    int contents = 0;

    // Check world brushes
    // TODO: Get brushes from worldspawn

    // Check entities
    std::vector<edict_t*> entities = SV_AreaEdicts(p, p);
    for (edict_t* ent : entities) {
        if (ent->solid == SOLID_NOT) continue;
        if (p.x >= ent->absmin.x && p.x <= ent->absmax.x &&
            p.y >= ent->absmin.y && p.y <= ent->absmax.y &&
            p.z >= ent->absmin.z && p.z <= ent->absmax.z) {
            contents |= ent->contents;
        }
    }

    return contents;
}

// Check visibility between two points (simple line-of-sight test)
bool SV_Visible(const glm::vec3& from, const glm::vec3& to, edict_t* passent) {
    glm::vec3 mins(0, 0, 0);
    glm::vec3 maxs(0, 0, 0);
    trace_t trace = SV_Trace(from, mins, maxs, to, passent, MASK_OPAQUE);
    return trace.fraction == 1.0f;
}

// Initialize the world
void SV_InitWorld() {
    // Clear grid
    for (int x = 0; x < WORLD_GRID_CELLS; x++) {
        for (int y = 0; y < WORLD_GRID_CELLS; y++) {
            world_grid[x][y].clear();
        }
    }
}

// Clear the world
void SV_ClearWorld() {
    for (int x = 0; x < WORLD_GRID_CELLS; x++) {
        for (int y = 0; y < WORLD_GRID_CELLS; y++) {
            world_grid[x][y].clear();
        }
    }
}

}  // namespace engine
