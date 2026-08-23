/*
 * =======================================================================
 *
 * Bone Yards - Enemy AI & Creatures
 *
 * =======================================================================
 */

#include "by_local.h"

edict_t *
BY_SpawnMechalichen(vec3_t origin, int difficulty)
{
	edict_t *ent;
	mechalichen_t *mech;

	ent = G_Spawn();
	if (!ent)
		return NULL;

	ent->classname = "monster_mechalichen";
	ent->solid = SOLID_BBOX;
	ent->svflags |= SVF_MONSTER;

	VectorCopy(origin, ent->s.origin);
	VectorSet(ent->mins, -16, -16, -24);
	VectorSet(ent->maxs, 16, 16, 24);

	mech = (mechalichen_t *)G_Alloc(sizeof(mechalichen_t));
	memset(mech, 0, sizeof(mechalichen_t));

	mech->ai.owner = ent;
	mech->ai.ai_state = 0;
	mech->ai.awareness_level = 0.0f;
	mech->ai.aggression = 30 * difficulty;
	mech->ai.health_max = 40 * difficulty;

	mech->tentacle_count = 3 + difficulty;
	mech->infection_spread_rate = 10 * difficulty;

	ent->health = mech->ai.health_max;
	ent->max_health = mech->ai.health_max;

	gi.dprintf("Spawned Mechalichen (difficulty: %d) at (%.1f, %.1f, %.1f)\n",
		difficulty, origin[0], origin[1], origin[2]);

	return ent;
}

edict_t *
BY_SpawnNuclearSlug(vec3_t origin, int difficulty)
{
	edict_t *ent;
	nuclear_slug_t *slug;

	ent = G_Spawn();
	if (!ent)
		return NULL;

	ent->classname = "monster_nuclear_slug";
	ent->solid = SOLID_BBOX;
	ent->svflags |= SVF_MONSTER;

	VectorCopy(origin, ent->s.origin);
	VectorSet(ent->mins, -12, -12, -16);
	VectorSet(ent->maxs, 12, 12, 16);

	slug = (nuclear_slug_t *)G_Alloc(sizeof(nuclear_slug_t));
	memset(slug, 0, sizeof(nuclear_slug_t));

	slug->ai.owner = ent;
	slug->ai.ai_state = 0;
	slug->ai.awareness_level = 0.0f;
	slug->ai.aggression = 50 * difficulty;
	slug->ai.health_max = 25 * difficulty;

	slug->irradiation_level = 20.0f + (difficulty * 10);
	slug->tail_strike_power = 15 * difficulty;
	slug->is_charged = false;

	ent->health = slug->ai.health_max;
	ent->max_health = slug->ai.health_max;

	gi.dprintf("Spawned Nuclear Slug (difficulty: %d) at (%.1f, %.1f, %.1f)\n",
		difficulty, origin[0], origin[1], origin[2]);

	return ent;
}

void
BY_UpdateEnemyAI(edict_t *enemy)
{
	if (!enemy)
		return;
}

void
BY_EnemyThink(edict_t *enemy)
{
	BY_UpdateEnemyAI(enemy);
}
