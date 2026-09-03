// g_weapon.cpp -- weapon system, firing, ammo, and reload mechanics
// Handles weapon selection, firing modes, ammo tracking, and weapon-specific logic.
// Each weapon has unique characteristics: damage, fire rate, ammo type, spread, etc.

#include "qcommon.h"
#include "game.h"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

namespace game {

extern edict_t* g_edicts;
extern int g_num_edicts;
extern gamestate_t g_gamestate;

// Weapon definitions
struct weapon_def_t {
    int id;
    const char* name;
    int damage;
    int ammo_type;
    int ammo_per_shot;
    float fire_rate;  // Shots per second
    float spread;     // Accuracy spread in degrees
    float projectile_speed;
    bool hitscan;     // True for instant hit, false for projectile
};

static const weapon_def_t weapon_defs[] = {
    {1, "railgun", 100, 0, 1, 2.0f, 0.0f, 0.0f, true},
    {2, "plasma", 50, 1, 1, 5.0f, 5.0f, 500.0f, false},
    {3, "shotgun", 120, 2, 8, 1.0f, 15.0f, 400.0f, false},
    {4, "rockets", 150, 3, 1, 2.0f, 5.0f, 300.0f, false},
    {0, nullptr, 0, 0, 0, 0.0f, 0.0f, 0.0f, false}
};

// Ammo types
#define AMMO_RAIL    0
#define AMMO_PLASMA  1
#define AMMO_SHELLS  2
#define AMMO_ROCKETS 3

// Player ammo capacity
static constexpr int max_ammo[] = {100, 200, 50, 20};

// Get weapon definition by ID
static const weapon_def_t* W_GetDef(int weapon_id) {
    for (int i = 0; weapon_defs[i].name; i++) {
        if (weapon_defs[i].id == weapon_id) {
            return &weapon_defs[i];
        }
    }
    return nullptr;
}

// Give a weapon to a player
void W_GiveWeapon(edict_t* player, int weapon_id) {
    if (!player) return;

    const weapon_def_t* def = W_GetDef(weapon_id);
    if (!def) {
        Com_Printf("W_GiveWeapon: Unknown weapon %d\n", weapon_id);
        return;
    }

    // Add to weapon list
    if (player->weapons[weapon_id] == 0) {
        player->weapons[weapon_id] = 1;
        Com_Printf("%s given %s\n", player->classname.c_str(), def->name);
    }

    // Also give ammo
    W_GiveAmmo(player, def->ammo_type, 50);
}

// Give ammo to a player
void W_GiveAmmo(edict_t* player, int ammo_type, int amount) {
    if (!player || ammo_type < 0 || ammo_type >= 4) return;

    player->ammo[ammo_type] += amount;
    if (player->ammo[ammo_type] > max_ammo[ammo_type]) {
        player->ammo[ammo_type] = max_ammo[ammo_type];
    }

    Com_Printf("%s ammo %d: %d/%d\n", player->classname.c_str(), ammo_type,
               player->ammo[ammo_type], max_ammo[ammo_type]);
}

// Select a weapon
void W_SelectWeapon(edict_t* player, int weapon_id) {
    if (!player) return;

    if (!player->weapons[weapon_id]) {
        Com_Printf("Player doesn't have weapon %d\n", weapon_id);
        return;
    }

    player->current_weapon = weapon_id;
    player->weapon_ready_time = g_gamestate.time + 0.2f;  // Equip delay
    Com_Printf("Weapon selected: %d\n", weapon_id);
}

// Check if player can fire current weapon
bool W_CanFire(edict_t* player) {
    if (!player || player->current_weapon <= 0) return false;

    const weapon_def_t* def = W_GetDef(player->current_weapon);
    if (!def) return false;

    // Check fire rate
    if (g_gamestate.time < player->last_fire_time + (1.0f / def->fire_rate)) {
        return false;  // Too soon
    }

    // Check ammo
    if (player->ammo[def->ammo_type] < def->ammo_per_shot) {
        return false;  // Out of ammo
    }

    // Check if weapon is ready (not reloading, etc.)
    if (g_gamestate.time < player->weapon_ready_time) {
        return false;
    }

    return true;
}

// Fire the current weapon
void W_Fire(edict_t* player, const glm::vec3& dir) {
    if (!W_CanFire(player)) return;

    const weapon_def_t* def = W_GetDef(player->current_weapon);
    if (!def) return;

    // Consume ammo
    player->ammo[def->ammo_type] -= def->ammo_per_shot;
    player->last_fire_time = g_gamestate.time;

    Com_Printf("%s fired %s\n", player->classname.c_str(), def->name);

    // Get muzzle position
    glm::vec3 start = player->origin + glm::vec3(0, 0, 40);  // Eye height

    if (def->hitscan) {
        // Instant hit weapon
        glm::vec3 spread_dir = dir;
        
        // Apply spread
        if (def->spread > 0.0f) {
            float spread_rad = def->spread * 3.14159f / 180.0f;
            float rand_angle = (rand() % 360) * 3.14159f / 180.0f;
            float rand_spread = ((rand() % (int)(def->spread * 100)) / 100.0f) * spread_rad;
            
            // Rotate direction by random spread
            glm::vec3 right = glm::normalize(glm::cross(spread_dir, {0, 0, 1}));
            glm::vec3 up = glm::normalize(glm::cross(right, spread_dir));
            
            spread_dir = glm::normalize(
                spread_dir + right * sinf(rand_spread) * cosf(rand_angle) + 
                up * sinf(rand_spread) * sinf(rand_angle)
            );
        }

        // Cast ray
        trace_t trace = G_FireRay(player, start, spread_dir, def->damage, 4000);
        
        // TODO: Draw tracer line
    } else {
        // Projectile weapon - fire multiple projectiles for shotgun
        int num_projectiles = 1;
        if (def->id == 3) {  // Shotgun
            num_projectiles = 8;
        }

        for (int i = 0; i < num_projectiles; i++) {
            glm::vec3 proj_dir = dir;

            // Apply spread
            if (def->spread > 0.0f) {
                float spread_rad = def->spread * 3.14159f / 180.0f;
                float angle_step = 360.0f / num_projectiles;
                float rand_angle = (angle_step * i + (rand() % (int)angle_step)) * 3.14159f / 180.0f;
                float rand_spread = ((rand() % (int)(def->spread * 100)) / 100.0f) * spread_rad;
                
                glm::vec3 right = glm::normalize(glm::cross(proj_dir, {0, 0, 1}));
                glm::vec3 up = glm::normalize(glm::cross(right, proj_dir));
                
                proj_dir = glm::normalize(
                    proj_dir + right * sinf(rand_spread) * cosf(rand_angle) + 
                    up * sinf(rand_spread) * sinf(rand_angle)
                );
            }

            G_Fire(player, start, proj_dir, def->damage / num_projectiles, 
                   def->projectile_speed, 10.0f);
        }
    }
}

// Reload weapon
void W_Reload(edict_t* player) {
    if (!player || player->current_weapon <= 0) return;

    const weapon_def_t* def = W_GetDef(player->current_weapon);
    if (!def) return;

    // Already reloading?
    if (g_gamestate.time < player->reload_end_time) {
        return;
    }

    // Check if we have ammo in reserve
    if (player->ammo[def->ammo_type] <= 0) {
        Com_Printf("No ammo to reload\n");
        return;
    }

    // Start reload (0.6 second reload time for most weapons)
    player->reload_end_time = g_gamestate.time + 0.6f;
    player->weapon_ready_time = player->reload_end_time;

    Com_Printf("Reloading %s...\n", def->name);
}

// Projectile update (from g_spawn)
void SP_projectile_think(edict_t* ent) {
    if (!ent) return;

    // Check lifetime
    if (g_gamestate.time > ent->nextthink) {
        G_FreeEntity(ent);
        return;
    }

    // Trace movement
    glm::vec3 end = ent->origin + ent->velocity * (1.0f / 60.0f);
    trace_t trace = SV_Trace(ent->origin, {-2, -2, -2}, {2, 2, 2}, end, ent->owner, MASK_SHOT);

    if (trace.fraction < 1.0f) {
        // Hit something
        if (trace.ent && trace.ent->inuse && trace.ent != ent->owner) {
            G_Damage(trace.ent, ent->owner, ent, 
                    glm::normalize(ent->velocity), trace.endpos, ent->damage, DAMAGE_NORMAL);
        }

        // Explosion at impact
        G_RadiusDamage(ent, ent->owner, ent->damage / 2, 100.0f, ent, DAMAGE_EXPLOSION);

        G_FreeEntity(ent);
        return;
    }

    // Move projectile
    ent->origin = end;
    SV_UnlinkEdict(ent);
    SV_LinkEdict(ent);
}

// Get weapon list for player
void W_GetInventory(edict_t* player, int* weapons, int* ammo) {
    if (!player) return;

    for (int i = 1; i < 5; i++) {
        weapons[i] = player->weapons[i];
    }

    for (int i = 0; i < 4; i++) {
        ammo[i] = player->ammo[i];
    }
}

// Drop weapon
void W_DropWeapon(edict_t* player) {
    if (!player || player->current_weapon <= 0) return;

    const weapon_def_t* def = W_GetDef(player->current_weapon);
    if (!def) return;

    // Create weapon item entity
    edict_t* item = G_Spawn();
    if (!item) return;

    item->classname = "dropped_weapon";
    item->origin = player->origin + glm::vec3(0, 0, 10);
    item->velocity = glm::normalize(player->velocity) * 200.0f;
    item->solid = SOLID_TRIGGER;
    item->movetype = MOVETYPE_BOUNCE;
    item->weapon_id = player->current_weapon;
    item->s.modelindex = 100 + player->current_weapon;

    // Drop ammo too
    item->ammo_count = player->ammo[def->ammo_type];

    SV_LinkEdict(item);

    Com_Printf("%s dropped %s\n", player->classname.c_str(), def->name);

    // Remove from player inventory
    player->weapons[player->current_weapon] = 0;
    player->current_weapon = 0;
}

}  // namespace game
