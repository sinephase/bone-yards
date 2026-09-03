// g_main.cpp -- game module initialization and main game loop
// The game DLL runs at a fixed server tick rate and handles all game logic:
// entity updates, collision detection, damage, spawning, etc.

#include "qcommon.h"
#include "game.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <glm/glm.hpp>

namespace game {

// Global game state
gamestate_t g_gamestate;
static std::vector<edict_t> g_entities;
static constexpr int MAX_ENTITIES = 1024;
static constexpr float FRAMETIME = 1.0f / 60.0f;  // 60 Hz server tick

edict_t* g_edicts = nullptr;
int g_num_edicts = 0;
int g_max_edicts = MAX_ENTITIES;

// Game initialization
void G_InitGame(int levelnum, const char* mapname) {
    Com_Printf("Initializing game module...\n");

    // Clear entity list
    g_entities.clear();
    g_entities.resize(MAX_ENTITIES);
    g_edicts = g_entities.data();
    g_num_edicts = 0;

    // Initialize game state
    std::memset(&g_gamestate, 0, sizeof(g_gamestate));
    g_gamestate.levelnum = levelnum;
    g_gamestate.time = 0.0f;
    g_gamestate.framenum = 0;

    // Initialize world
    SV_InitWorld();

    // Load map/spawn entities
    G_SpawnEntities(mapname);

    Com_Printf("Game initialized. Entities: %d\n", g_num_edicts);
}

// Allocate a new entity
edict_t* G_Spawn() {
    if (g_num_edicts >= MAX_ENTITIES) {
        Com_Printf("G_Spawn: MAX_ENTITIES reached\n");
        return nullptr;
    }

    edict_t* ent = &g_edicts[g_num_edicts++];
    *ent = edict_t{};
    ent->s.number = g_num_edicts - 1;
    ent->inuse = true;

    return ent;
}

// Remove an entity from the game
void G_FreeEntity(edict_t* ent) {
    if (!ent || !ent->inuse) return;

    SV_UnlinkEdict(ent);
    ent->inuse = false;
}

// Main game frame logic
void G_RunFrame() {
    g_gamestate.framenum++;
    g_gamestate.time += FRAMETIME;

    // Update all entities
    for (int i = 0; i < g_num_edicts; i++) {
        edict_t* ent = &g_edicts[i];
        if (!ent->inuse) continue;

        // Call think function if registered
        if (ent->think) {
            ent->think(ent);
        }

        // Apply physics if needed
        if (ent->movetype == MOVETYPE_STEP || ent->movetype == MOVETYPE_WALK) {
            // Apply gravity
            ent->velocity.z -= 700.0f * FRAMETIME;  // gravity

            // Move entity
            ent->origin += ent->velocity * FRAMETIME;

            // Relink to world grid
            SV_UnlinkEdict(ent);
            SV_LinkEdict(ent);
        }
    }

    // Check entity-entity collisions and interactions
    for (int i = 0; i < g_num_edicts; i++) {
        edict_t* ent = &g_edicts[i];
        if (!ent->inuse) continue;

        // Get nearby entities
        std::vector<edict_t*> nearby = SV_AreaEdicts(ent->absmin, ent->absmax);
        for (edict_t* other : nearby) {
            if (other == ent) continue;
            if (!other->inuse) continue;

            // Check collision
            if (CM_BoxesIntersect(ent->absmin, ent->absmax, other->absmin, other->absmax)) {
                // Call collision callback
                if (ent->touch) {
                    ent->touch(ent, other);
                }
                if (other->touch) {
                    other->touch(other, ent);
                }
            }
        }
    }
}

// Shutdown the game
void G_ShutdownGame() {
    Com_Printf("Shutting down game module...\n");
    SV_ClearWorld();
    g_entities.clear();
    g_edicts = nullptr;
    g_num_edicts = 0;
}

// External game callbacks (called from engine/server)
void G_ClientConnect(int clientnum) {
    Com_Printf("Client %d connected\n", clientnum);
    
    // Spawn a player entity for this client
    edict_t* player = G_Spawn();
    if (!player) return;

    player->s.modelindex = 1;  // Player model
    player->solid = SOLID_BBOX;
    player->movetype = MOVETYPE_WALK;
    player->client = clientnum;
    player->health = 100;
    player->maxhealth = 100;

    // Set bounding box (player size)
    player->mins = {-16, -16, 0};
    player->maxs = {16, 16, 56};

    // Spawn position (TODO: get from map)
    player->origin = {0, 0, 100};

    // Initialize player state
    player->velocity = {0, 0, 0};
    player->think = nullptr;
    player->touch = nullptr;

    SV_LinkEdict(player);

    Com_Printf("Player spawned at %.0f %.0f %.0f\n", player->origin.x, player->origin.y, player->origin.z);
}

void G_ClientDisconnect(int clientnum) {
    Com_Printf("Client %d disconnected\n", clientnum);

    // Find and remove player entity
    for (int i = 0; i < g_num_edicts; i++) {
        edict_t* ent = &g_edicts[i];
        if (ent->inuse && ent->client == clientnum) {
            G_FreeEntity(ent);
            break;
        }
    }
}

void G_ClientThink(int clientnum, usercmd_t* cmd) {
    // Find player entity
    edict_t* player = nullptr;
    for (int i = 0; i < g_num_edicts; i++) {
        edict_t* ent = &g_edicts[i];
        if (ent->inuse && ent->client == clientnum) {
            player = ent;
            break;
        }
    }

    if (!player) return;

    // Update player movement from command
    pmove_state_t pm;
    std::memset(&pm, 0, sizeof(pm));
    pm.origin = player->origin;
    pm.velocity = player->velocity;
    pm.onground = (player->groundentity != nullptr);
    pm.cmd = *cmd;
    pm.frametime = FRAMETIME;

    // Run movement
    PM_Move(&pm, cmd, FRAMETIME);

    // Update entity from movement result
    player->origin = pm.origin;
    player->velocity = pm.velocity;
    player->groundentity = pm.groundentity;

    // Relink to world grid
    SV_UnlinkEdict(player);
    SV_LinkEdict(player);
}

// Get game state for client snapshot
void G_GetGameState(gamestate_t* state) {
    *state = g_gamestate;
}

}  // namespace game
