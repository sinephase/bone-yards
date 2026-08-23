/*
 * =======================================================================
 *
 * Bone Yards - Main Game Loop & Initialization
 *
 * =======================================================================
 */

#include "by_local.h"

bone_yards_game_t bone_yards;
game_locals_t game;
level_locals_t level;
game_import_t gi;
game_export_t globals;
spawn_temp_t st;

edict_t *g_edicts;

static cvar_t *deathmatch;
static cvar_t *coop;
static cvar_t *skill;
static cvar_t *maxclients;
static cvar_t *sv_gravity;
static cvar_t *by_difficulty;
static cvar_t *by_editor_enabled;

static void
BY_ShutdownGame(void)
{
	gi.dprintf("==== BY_ShutdownGame ====\n");
	gi.FreeTags(TAG_LEVEL);
	gi.FreeTags(TAG_GAME);
}

Q2_DLL_EXPORTED game_export_t *
GetGameAPI(const game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = BY_InitGame;
	globals.Shutdown = BY_ShutdownGame;
	globals.SpawnEntities = BY_SpawnEntities;

	globals.WriteGame = NULL;
	globals.ReadGame = NULL;
	globals.WriteLevel = NULL;
	globals.ReadLevel = NULL;

	globals.ClientThink = NULL;
	globals.ClientConnect = NULL;
	globals.ClientUserinfoChanged = NULL;
	globals.ClientDisconnect = NULL;
	globals.ClientBegin = NULL;
	globals.ClientCommand = NULL;

	globals.RunFrame = BY_RunFrame;
	globals.ServerCommand = NULL;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

void
BY_InitGame(void)
{
	gi.dprintf("==== BY_InitGame ====\n");
	gi.dprintf("Bone Yards %s (Yamagi Quake II-based)\n", BY_VERSION);

	memset(&bone_yards, 0, sizeof(bone_yards_game_t));

	deathmatch = gi.cvar("deathmatch", "0", CVAR_SERVERINFO);
	coop = gi.cvar("coop", "1", CVAR_SERVERINFO);
	skill = gi.cvar("skill", "1", CVAR_SERVERINFO);
	maxclients = gi.cvar("maxclients", "1", CVAR_SERVERINFO);
	sv_gravity = gi.cvar("sv_gravity", "800", CVAR_SERVERINFO);
	by_difficulty = gi.cvar("by_difficulty", "1", 0);
	by_editor_enabled = gi.cvar("by_editor", "0", 0);

	BY_InitPowerSystem();
	BY_EditorInit();

	gi.dprintf("Game initialized successfully.\n");
}

static void
BY_RunFrame(void)
{
	int i;
	edict_t *ent;

	level.framenum++;
	level.time = level.framenum * FRAMETIME;

	BY_UpdatePowerZones();
	BY_UpdateHazards();

	ent = &g_edicts[0];
	for (i = 0; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		if (ent->client)
		{
			BY_UpdatePlayerAbilities(ent);
		}

		if (ent->svflags & SVF_MONSTER)
		{
			BY_EnemyThink(ent);
		}
	}

	if (by_editor_enabled->value && !deathmatch->value)
	{
		BY_EditorUpdate(NULL, NULL);
	}
}

void
Sys_Error(const char *error, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, error);
	vsnprintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	gi.error("%s", text);
}

void
Com_Printf(const char *msg, ...)
{
	va_list argptr;
	char text[1024];

	va_start(argptr, msg);
	vsnprintf(text, sizeof(text), msg, argptr);
	va_end(argptr);

	gi.dprintf("%s", text);
}
