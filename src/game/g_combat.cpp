// g_combat.cpp -- damage, health, and combat system
// Handles all damage calculations, death, gibbing, and combat interactions.
// Tracks player and entity health, applies damage, and manages giblets/corpses.

#include "qcommon.h"
#include "game.h"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

namespace game {

extern edict_t* g_edicts;
extern int g_num_edicts;
extern gamestate_t g_gamestate;

// Apply damage to an entity
void G_Damage(edict_t* victim, edict_t* attacker, edict_t* inflictor, 
              const glm::vec3& dir, const glm::vec3& point, int damage, int dflags) {
    if (!victim || !victim->inuse) return;
    if (victim->health <= 0) return;  // Already dead

    // Armor reduces damage
    float damage_mult = 1.0f;
    if (victim->armor > 0) {
        int armor_absorb = std::min(victim->armor, damage / 2);
        victim->armor -= armor_absorb;
        damage -= armor_absorb;
        damage_mult = 0.75f;  // Remaining damage reduced by armor
    }

    int final_damage = (int)(damage * damage_mult);
    final_damage = std::max(1, final_damage);

    // Apply damage
    victim->health -= final_damage;

    // Send damage event for HUD
    Com_Printf("%s took %d damage (health: %d)\n", victim->classname.c_str(), final_damage, victim->health);

    // Check if dead
    if (victim->health <= 0) {
        G_Die(victim, attacker, inflictor, final_damage, dflags);
    } else if (victim->pain) {
        victim->pain(victim, attacker, final_damage);
    }
}

// Entity death and gibbing
void G_Die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, int meansOfDeath) {
    if (!self || !self->inuse) return;
    if (self->health >= 0) self->health = -1;  // Mark as dead

    Com_Printf("%s died\n", self->classname.c_str());

    // Stop all movement
    self->velocity = {0, 0, 0};
    self->movetype = MOVETYPE_NONE;
    self->solid = SOLID_NOT;

    // Gib if enough damage
    if (damage > self->maxhealth) {
        G_Gib(self, attacker);
    }

    // Leave corpse (change model to corpse)
    // TODO: Play death animation, spawn corpse

    // Call death callback if any
    if (self->die) {
        self->die(self, inflictor, attacker, damage, meansOfDeath);
    }

    // Unlink from world
    SV_UnlinkEdict(self);
}

// Gib an entity into chunks
void G_Gib(edict_t* self, edict_t* attacker) {
    Com_Printf("%s gibbed\n", self->classname.c_str());

    // Spawn several gib pieces
    static constexpr int NUM_GIBS = 4;
    for (int i = 0; i < NUM_GIBS; i++) {
        edict_t* gib = G_Spawn();
        if (!gib) break;

        gib->classname = "gib";
        gib->solid = SOLID_NOT;
        gib->movetype = MOVETYPE_BOUNCE;
        gib->origin = self->origin;

        // Random velocity
        float angle = (360.0f / NUM_GIBS) * i;
        float speed = 100.0f + (rand() % 100);
        gib->velocity.x = cosf(angle * 3.14159f / 180.0f) * speed;
        gib->velocity.y = sinf(angle * 3.14159f / 180.0f) * speed;
        gib->velocity.z = 50.0f + (rand() % 100);

        SV_LinkEdict(gib);
    }

    // Remove original entity
    G_FreeEntity(self);
}

// Fire a projectile/hitscan weapon
void G_Fire(edict_t* ent, const glm::vec3& start, const glm::vec3& dir, 
            int damage, float speed, int lifetime) {
    // Create projectile entity
    edict_t* proj = G_Spawn();
    if (!proj) return;

    proj->classname = "projectile";
    proj->origin = start;
    proj->velocity = dir * speed;
    proj->solid = SOLID_BBOX;
    proj->movetype = MOVETYPE_FLY;
    proj->owner = ent;
    proj->damage = damage;
    proj->s.modelindex = 40;  // Projectile model

    // Set lifetime
    proj->nextthink = g_gamestate.time + lifetime;

    SV_LinkEdict(proj);

    Com_Printf("Projectile fired\n");
}

// Hitscan/raycast weapon (rail gun, laser)
trace_t G_FireRay(edict_t* ent, const glm::vec3& start, const glm::vec3& dir, 
                  int damage, int range) {
    glm::vec3 end = start + dir * (float)range;
    trace_t trace = SV_Trace(start, {0, 0, 0}, {0, 0, 0}, end, ent, MASK_SHOT);

    if (trace.ent && trace.ent->inuse) {
        // Hit an entity
        glm::vec3 point = glm::mix(start, end, trace.fraction);
        G_Damage(trace.ent, ent, ent, dir, point, damage, DAMAGE_NORMAL);
        Com_Printf("Ray hit %s\n", trace.ent->classname.c_str());
    }

    return trace;
}

// Check if entity can see another entity
bool G_CanSee(edict_t* ent, edict_t* target) {
    if (!ent || !target) return false;

    // Simple line-of-sight test
    return SV_Visible(ent->origin, target->origin, ent);
}

// Get distance to entity
float G_Distance(edict_t* ent, edict_t* target) {
    if (!ent || !target) return 0.0f;
    return glm::distance(ent->origin, target->origin);
}

// Find nearest enemy to an entity
edict_t* G_FindNearestEnemy(edict_t* ent, float range) {
    if (!ent) return nullptr;

    edict_t* best = nullptr;
    float best_dist = range;

    for (int i = 0; i < g_num_edicts; i++) {
        edict_t* other = &g_edicts[i];
        if (!other->inuse || other == ent) continue;
        if (other->health <= 0) continue;  // Skip dead entities
        if (other->teamnum == ent->teamnum && ent->teamnum > 0) continue;  // Skip team

        float dist = G_Distance(ent, other);
        if (dist < best_dist) {
            // Check line of sight
            if (G_CanSee(ent, other)) {
                best = other;
                best_dist = dist;
            }
        }
    }

    return best;
}

// Heal an entity
void G_Heal(edict_t* ent, int amount) {
    if (!ent) return;

    ent->health += amount;
    if (ent->health > ent->maxhealth) {
        ent->health = ent->maxhealth;
    }

    Com_Printf("%s healed to %d/%d\n", ent->classname.c_str(), ent->health, ent->maxhealth);
}

// Give armor to an entity
void G_GiveArmor(edict_t* ent, int amount) {
    if (!ent) return;

    ent->armor += amount;
    int max_armor = (int)(ent->maxhealth * 0.5f);
    if (ent->armor > max_armor) {
        ent->armor = max_armor;
    }

    Com_Printf("%s given %d armor (total: %d)\n", ent->classname.c_str(), amount, ent->armor);
}

// Give weapon ammo
void G_GiveAmmo(edict_t* ent, int weapon_id, int ammo_count) {
    if (!ent) return;

    // Find weapon in inventory
    // TODO: Inventory system
    Com_Printf("%s given %d ammo for weapon %d\n", ent->classname.c_str(), ammo_count, weapon_id);
}

// Area damage (explosions, etc.)
void G_RadiusDamage(edict_t* inflictor, edict_t* attacker, int damage, 
                    float radius, edict_t* ignore, int dflags) {
    if (!inflictor) return;

    glm::vec3 origin = inflictor->origin;

    // Get all entities in radius
    glm::vec3 mins = origin - glm::vec3(radius);
    glm::vec3 maxs = origin + glm::vec3(radius);
    std::vector<edict_t*> victims = SV_AreaEdicts(mins, maxs);

    for (edict_t* ent : victims) {
        if (!ent->inuse || ent == ignore) continue;
        if (ent->solid == SOLID_NOT) continue;

        // Calculate distance
        float dist = glm::distance(ent->origin, origin);
        if (dist > radius) continue;

        // Falloff damage by distance
        float falloff = 1.0f - (dist / radius);
        int final_damage = (int)(damage * falloff);

        // Ray trace to check line of sight
        trace_t trace = SV_Trace(origin, {0, 0, 0}, {0, 0, 0}, ent->origin, inflictor, MASK_SOLID);
        if (trace.fraction < 1.0f) {
            final_damage = final_damage / 2;  // Half damage through walls
        }

        if (final_damage > 0) {
            glm::vec3 dir = glm::normalize(ent->origin - origin);
            G_Damage(ent, attacker, inflictor, dir, ent->origin, final_damage, dflags);
        }
    }

    Com_Printf("Radius damage: %d at %.0f %.0f %.0f (radius %.0f)\n", 
               damage, origin.x, origin.y, origin.z, radius);
}

}  // namespace game
