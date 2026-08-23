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
#include <strings.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>

/* ====================================================================== */
/* BASIC TYPES */
/* ====================================================================== */

typedef int qboolean;
#define true 1
#define false 0

typedef float vec3_t[3];

typedef int ivec3_t[3];

typedef struct {
	float m[16];
} mat4_t;

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

#define SOLID_NOT 0
#define SOLID_TRIGGER 1
#define SOLID_BBOX 2
#define SOLID_BSP 3

/* Forward declarations */
typedef struct edict_s edict_t;
typedef struct gclient_s gclient_t;

typedef struct {
	int usernum;
	unsigned int buttons;
	unsigned int oldbuttons;
	short forwardmove;
	short rightmove;
	short upmove;
	short angles[3];
} usercmd_t;

typedef struct gclient_s {
	int ping;
} gclient_t;

typedef struct {
	vec3_t origin;
	vec3_t angles;
	vec3_t old_origin;
} entity_state_t;

typedef struct edict_s {
	int s_number;
	int inuse;
	int svflags;
	int flags;
	int solid;
	int movetype;
	int classname_index;
	const char *classname;
	float freetime;
	entity_state_t s;
	vec3_t velocity;
	vec3_t avelocity;
	vec3_t mins;
	vec3_t maxs;
	vec3_t absmin;
	vec3_t absmax;
	vec3_t size;
	struct edict_s *groundentity;
	int health;
	int max_health;
	int gib_health;
	int mass;
	int contents;
	int movetype_flags;
	int nextthink;
	float ltime;
	float lastactivetime;
	int index;
	struct edict_s *owner;
	gclient_t *client;
	void *priv;
} edict_t;

/* ====================================================================== */
/* GAME STATE */
/* ====================================================================== */

typedef struct {
	int serverframe;
	int leveltime;
	char levelname[64];
	char mapname[64];
	int framenum;
	float time;
	edict_t *current_entity;
} level_locals_t;

typedef struct {
	int dummy;
} game_locals_t;

typedef struct {
	int dummy;
} spawn_temp_t;

/* ====================================================================== */
/* BUTTONS */
/* ====================================================================== */

#define BUTTON_ATTACK 1
#define BUTTON_ATTACK2 2
#define BUTTON_JUMP 4
#define BUTTON_1 8
#define BUTTON_2 16
#define BUTTON_3 32
#define BUTTON_4 64

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

#if defined(_WIN32)
#define Q2_DLL_EXPORTED __declspec(dllexport)
#else
#define Q2_DLL_EXPORTED __attribute__((visibility("default")))
#endif

#define CVAR_SERVERINFO 1

typedef struct cvar_s {
	char *name;
	char *string;
	float value;
	int flags;
} cvar_t;

typedef struct {
	void (*dprintf)(const char *fmt, ...);
	void (*error)(const char *fmt, ...);
	void (*FreeTags)(int tag);
	void *(*TagMalloc)(int size, int tag);
	void (*TagFree)(void *ptr, int tag);
	cvar_t *(*cvar)(const char *name, const char *value, int flags);
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
/* HELPER MACROS */
/* ====================================================================== */

#ifndef M_PI
#define M_PI 3.14159265359f
#endif

/* Vector operations */
#define VectorClear(a) ((a)[0]=(a)[1]=(a)[2]=0)
#define VectorSet(v,x,y,z) ((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define VectorCopy(a,b) ((b)[0]=(a)[0], (b)[1]=(a)[1], (b)[2]=(a)[2])
#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c) ((c)[0]=(a)[0]-(b)[0], (c)[1]=(a)[1]-(b)[1], (c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c) ((c)[0]=(a)[0]+(b)[0], (c)[1]=(a)[1]+(b)[1], (c)[2]=(a)[2]+(b)[2])
#define VectorScale(v,s,o) ((o)[0]=(v)[0]*(s), (o)[1]=(v)[1]*(s), (o)[2]=(v)[2]*(s))
#define VectorMA(v,s,b,o) ((o)[0]=(v)[0]+(b)[0]*(s), (o)[1]=(v)[1]+(b)[1]*(s), (o)[2]=(v)[2]+(b)[2]*(s))
#define VectorNormalize(v) { float len = sqrtf((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2]); if(len) { (v)[0]/=len; (v)[1]/=len; (v)[2]/=len; } }
#define VectorLength(v) sqrtf((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2])

#define AngleVectors(angles, forward, right, up) \
{ \
	float pitch = (angles)[0] * M_PI / 180.0f; \
	float yaw = (angles)[1] * M_PI / 180.0f; \
	float sp = sinf(pitch), cp = cosf(pitch); \
	float sy = sinf(yaw), cy = cosf(yaw); \
	if(forward) { (forward)[0] = cp*cy; (forward)[1] = cp*sy; (forward)[2] = -sp; } \
	if(right) { (right)[0] = -sy; (right)[1] = cy; (right)[2] = 0; } \
	if(up) { (up)[0] = sp*cy; (up)[1] = sp*sy; (up)[2] = cp; } \
}

/* String comparison */
#define Q_stricmp(s1, s2) strcasecmp(s1, s2)
#define Q_strlcpy(d, s, n) strncpy(d, s, n-1); d[n-1]=0

/* Random */
#define random() (rand() / (float)RAND_MAX)

/* Allocation wrappers */
#define G_Alloc(size) malloc(size)
#define G_Spawn() calloc(1, sizeof(edict_t))
#define G_FreeEdict(e) free(e)

#endif /* GAME_H */
