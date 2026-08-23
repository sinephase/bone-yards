#pragma once

#include "../qcommon/math.h"
#include "../qcommon/collision.h"
#include <stdint.h>
#include <stdbool.h>

/* Entity solid types */
#define SOLID_NOT       0
#define SOLID_TRIGGER   1
#define SOLID_BBOX      2
#define SOLID_BSP       3

/* Entity structure - shared between server and game */
typedef struct edict_s {
    uint32_t id;
    uint32_t inuse;
    char *classname;
    
    /* Physics */
    vec3_t origin;
    vec3_t angles;
    vec3_t velocity;
    vec3_t avelocity;
    vec3_t mins, maxs;
    vec3_t absmin, absmax;
    uint32_t solid;
    uint32_t movetype;
    uint32_t contents;
    uint32_t clipmask;
    
    /* Damage and health */
    int health;
    int max_health;
    float mass;
    float gravity;
    uint32_t takedamage;
    
    /* Behavior */
    char *model;
    float nextthink;
    void (*think)(struct edict_s *);
    void (*touch)(struct edict_s *, struct edict_s *);
    void (*use)(struct edict_s *, struct edict_s *, struct edict_s *);
    void (*pain)(struct edict_s *, struct edict_s *, float, float);
    void (*die)(struct edict_s *, struct edict_s *, struct edict_s *, int, vec3_t);
    void (*blocked)(struct edict_s *, struct edict_s *);
    
    /* Linking */
    struct edict_s *owner;
    struct edict_s *enemy;
    struct edict_s *goalentity;
    struct edict_s *groundentity;
    
    /* Targeting */
    char *target;
    char *targetname;
    char *killtarget;
    
    /* Game-specific extensions */
    void *game_data;
    
    uint32_t linkcount;
} edict_t;

/* World state */
typedef struct {
    edict_t *edicts;
    int num_edicts;
    int max_edicts;
    
    brush_t *brushes;
    int numbrushes;
    
    edict_t *player;
    edict_t *worldspawn;
} world_t;

/* World functions */
void SV_LinkEdict(edict_t *ent);
void SV_UnlinkEdict(edict_t *ent);
edict_t **SV_AreaEdicts(const vec3_t mins, const vec3_t maxs, int *count, int areatype);

trace_t SV_Trace(const vec3_t start, const vec3_t mins, const vec3_t maxs,
                  const vec3_t end, edict_t *passent, uint32_t contentmask);
uint32_t SV_PointContents(const vec3_t p);
bool SV_Visible(const vec3_t from, const vec3_t to, edict_t *passent);
