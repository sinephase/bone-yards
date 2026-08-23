#pragma once

#include "../server/sv_world.h"
#include "../qcommon/math.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_EDICTS 2048
#define FRAMETIME 0.1f

/* Entity flags */
#define FL_FLY          0x0001
#define FL_SWIM         0x0002
#define FL_NOTARGET     0x0004
#define FL_GODMODE      0x0008
#define FL_INWATER      0x0010
#define FL_TEAMSLAVE    0x0020
#define FL_NO_KNOCKBACK 0x0040
#define FL_POWERFED     0x0080

/* Server flags */
#define SVF_DEADMONSTER 0x0001
#define SVF_MONSTER     0x0002
#define SVF_NOCLIENT    0x0004

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
#define DAMAGE_RADIUS   0x0001
#define DAMAGE_NO_ARMOR 0x0002
#define DAMAGE_ENERGY   0x0004
#define DAMAGE_RADIATION 0x0008

/* Level state */
typedef struct {
    float time;
    uint32_t framenum;
    char mapname[64];
    char title[64];
    
    uint32_t killedMonsters;
    uint32_t totalMonsters;
    
    float alertLevel;
    
    edict_t *player;
    edict_t *worldspawn;
} level_t;

extern level_t level;
extern world_t world;

/* Game functions */
void G_InitGame(void);
void G_ResetWorld(void);
void G_RunFrame(float dt);
void G_Shutdown(void);

/* Entity functions */
edict_t *G_Spawn(const char *classname);
void G_FreeEdict(edict_t *ent);
void G_InitEdict(edict_t *ent, const char *classname);

/* Damage */
void T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker,
              const vec3_t dir, const vec3_t point, const vec3_t normal,
              int damage, int knockback, uint32_t dflags, const char *mod);

/* Spawning */
void SpawnEntities(const char *mapdata);
