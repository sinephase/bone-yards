/*
 * =======================================================================
 *
 * Quake II Game DLL API
 *
 * Stub header for standalone build compatibility
 *
 * =======================================================================
 */

#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "../qcommon.h"
#include "../qcommon/pmove.h"
#include "../server/sv_world.h"

// Bring the shared engine types (edict_t is global already; trace_t, brush_t,
// usercmd_t, pmove_state_t, PM_Move, SV_*, CM_* helpers) into scope so the
// game module code can use them unqualified, matching how it is written.
using namespace engine;

/* ====================================================================== */
/* BASIC TYPES */
/* ====================================================================== */

/* ====================================================================== */
/* ENTITY SYSTEM */
/* ====================================================================== */

#define FL_CLIENT 1
#define FL_WEAPON 2
#define FL_NOTARGET 4
#define FL_INWATER 8

#define SVF_PLAYER 1
#define SVF_MONSTER 2
#define SVF_CORPSE 4

/* Movement types */
#define MOVETYPE_NONE   0
#define MOVETYPE_NOCLIP 1
#define MOVETYPE_PUSH   2
#define MOVETYPE_STOP   3
#define MOVETYPE_WALK   4
#define MOVETYPE_STEP   5
#define MOVETYPE_FLY    6
#define MOVETYPE_TOSS   7
#define MOVETYPE_BOUNCE 8

/* Damage flags */
#define DAMAGE_NONE         0
#define DAMAGE_NORMAL       1
#define DAMAGE_ENERGY       2
#define DAMAGE_EXPLOSION    4
#define DAMAGE_FIRE         8
#define DAMAGE_ACID         16

/* ====================================================================== */
/* GLFW KEY CODES (for input.h compatibility) */
/* ====================================================================== */

#define GLFW_KEY_W 87
#define GLFW_KEY_A 65
#define GLFW_KEY_S 83
#define GLFW_KEY_D 68
#define GLFW_KEY_SPACE 32
#define GLFW_KEY_LEFT_CONTROL 341
#define GLFW_KEY_ESCAPE 256
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#define GLFW_KEY_R 82

#define GLFW_PRESS 1
#define GLFW_RELEASE 0

/* ====================================================================== */
/* GAME IMPORT/EXPORT */
/* ====================================================================== */

#define GAME_API_VERSION 3

typedef struct {
	void (*dprintf)(const char *fmt, ...);
	void (*error)(const char *fmt, ...);
	void (*FreeTags)(int tag);
	void *(*TagMalloc)(int size, int tag);
	void (*TagFree)(void *ptr, int tag);
} game_import_t;

typedef struct {
	int apiversion;
	void (*Init)(void);
	void (*Shutdown)(void);
	void (*SpawnEntities)(const char *mapname, const char *entities, const char *spawnpoint);
	void (*WriteGame)(const char *filename, qboolean autosave);
	void (*ReadGame)(const char *filename);
	void (*WriteLevel)(const char *filename);
	void (*ReadLevel)(const char *filename);
	qboolean (*ClientConnect)(edict_t *ent, char *userinfo);
	void (*ClientBegin)(edict_t *ent);
	void (*ClientUserinfoChanged)(edict_t *ent, char *userinfo);
	void (*ClientDisconnect)(edict_t *ent);
	void (*ClientCommand)(edict_t *ent);
	void (*ClientThink)(edict_t *ent, usercmd_t *cmd);
	void (*RunFrame)(void);
	void (*ServerCommand)(void);
	int edict_size;
	int num_edicts;
} game_export_t;

/* ====================================================================== */
/* MEMORY TAGS */
/* ====================================================================== */

#define TAG_GAME 765
#define TAG_LEVEL 766

/* ====================================================================== */
/* GAME MODULE STATE */
/* ====================================================================== */

namespace game {

struct gamestate_t {
    int levelnum;
    float time;
    int framenum;
};

/* g_main.cpp */
void G_InitGame(int levelnum, const char *mapname);
void G_ShutdownGame();
edict_t *G_Spawn();
void G_FreeEntity(edict_t *ent);
void G_RunFrame();
void G_ClientConnect(int clientnum);
void G_ClientDisconnect(int clientnum);
void G_ClientThink(int clientnum, usercmd_t *cmd);
void G_GetGameState(gamestate_t *state);

/* g_combat.cpp */
void G_Damage(edict_t *victim, edict_t *attacker, edict_t *inflictor,
              const glm::vec3& dir, const glm::vec3& point, int damage, int dflags);
void G_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, int meansOfDeath);
void G_Gib(edict_t *self, edict_t *attacker);
void G_Fire(edict_t *ent, const glm::vec3& start, const glm::vec3& dir,
            int damage, float speed, int lifetime);
trace_t G_FireRay(edict_t *ent, const glm::vec3& start, const glm::vec3& dir,
                   int damage, int range);
bool G_CanSee(edict_t *ent, edict_t *target);
float G_Distance(edict_t *ent, edict_t *target);
edict_t *G_FindNearestEnemy(edict_t *ent, float range);
void G_Heal(edict_t *ent, int amount);
void G_GiveArmor(edict_t *ent, int amount);
void G_GiveAmmo(edict_t *ent, int weapon_id, int ammo_count);
void G_RadiusDamage(edict_t *inflictor, edict_t *attacker, int damage,
                     float radius, edict_t *ignore, int dflags);

/* g_spawn.cpp */
edict_t *ED_Spawn(const std::unordered_map<std::string, std::string>& kvpairs);
void G_SpawnEntities(const char *mapname);

/* g_weapon.cpp */
void W_GiveWeapon(edict_t *player, int weapon_id);
void W_GiveAmmo(edict_t *player, int ammo_type, int amount);
void W_SelectWeapon(edict_t *player, int weapon_id);
bool W_CanFire(edict_t *player);
void W_Fire(edict_t *player, const glm::vec3& dir);
void W_Reload(edict_t *player);
void SP_projectile_think(edict_t *ent);
void W_GetInventory(edict_t *player, int *weapons, int *ammo);
void W_DropWeapon(edict_t *player);

}  // namespace game

#endif /* GAME_H */
