/*
 * =======================================================================
 *
 * Bone Yards - Entity Spawning
 *
 * =======================================================================
 */

#include "by_local.h"

typedef struct {
	const char *name;
	void (*spawn_func)(edict_t *ent);
} spawn_entry_t;

void SP_monster_mechalichen_worker(edict_t *ent);
void SP_monster_mechalichen_soldier(edict_t *ent);
void SP_monster_nuclear_slug(edict_t *ent);
void SP_func_power_node(edict_t *ent);
void SP_func_door(edict_t *ent);
void SP_func_lift(edict_t *ent);
void SP_target_terminal(edict_t *ent);
void SP_target_log(edict_t *ent);
void SP_func_radiation_zone(edict_t *ent);

static spawn_entry_t spawn_table[] = {
	{"monster_mechalichen_worker", SP_monster_mechalichen_worker},
	{"monster_mechalichen_soldier", SP_monster_mechalichen_soldier},
	{"monster_nuclear_slug", SP_monster_nuclear_slug},
	{"func_power_node", SP_func_power_node},
	{"func_door", SP_func_door},
	{"func_lift", SP_func_lift},
	{"target_terminal", SP_target_terminal},
	{"target_log", SP_target_log},
	{"func_radiation_zone", SP_func_radiation_zone},
	{NULL, NULL}
};

void
BY_SpawnEntities(const char *mapname, const char *entities, const char *spawnpoint)
{
	gi.dprintf("Spawning entities for map: %s\n", mapname);
	Q_strlcpy(bone_yards.level.current_map, mapname, sizeof(bone_yards.level.current_map));
}

void SP_monster_mechalichen_worker(edict_t *ent) { BY_SpawnMechalichen(ent->s.origin, 1); }
void SP_monster_mechalichen_soldier(edict_t *ent) { BY_SpawnMechalichen(ent->s.origin, 2); }
void SP_monster_nuclear_slug(edict_t *ent) { BY_SpawnNuclearSlug(ent->s.origin, 1); }
void SP_func_power_node(edict_t *ent) { }
void SP_func_door(edict_t *ent) { }
void SP_func_lift(edict_t *ent) { }
void SP_target_terminal(edict_t *ent) { }
void SP_target_log(edict_t *ent) { }
void SP_func_radiation_zone(edict_t *ent) { }
