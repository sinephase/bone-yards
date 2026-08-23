/*
 * =======================================================================
 *
 * Bone Yards - Player Controller
 *
 * =======================================================================
 */

#include "by_local.h"

void
BY_InitPlayer(edict_t *player)
{
	player_archaeology_t *arch;

	if (!player)
		return;

	player->classname = "player";
	player->solid = SOLID_BBOX;
	player->flags = FL_CLIENT;
	player->svflags |= SVF_PLAYER;

	arch = (player_archaeology_t *)G_Alloc(sizeof(player_archaeology_t));
	memset(arch, 0, sizeof(player_archaeology_t));
	arch->scan_charge = 100;
	arch->decrypt_progress = 0;
	arch->current_access_level = 1;
	arch->num_inventory = 0;

	gi.dprintf("Player initialized\n");
}

void
BY_UpdatePlayerAbilities(edict_t *player)
{
	if (!player || !player->client)
		return;
}

void
BY_ScanEnvironment(edict_t *player, vec3_t scan_origin)
{
	int i;
	edict_t *ent;
	vec3_t delta;
	float distance;
	const float SCAN_RADIUS = 256.0f;

	if (!player)
		return;

	gi.dprintf("Scanning environment from (%.1f, %.1f, %.1f)\n",
		scan_origin[0], scan_origin[1], scan_origin[2]);

	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;

		VectorSubtract(ent->s.origin, scan_origin, delta);
		distance = VectorLength(delta);

		if (distance > SCAN_RADIUS)
			continue;

		gi.dprintf("  -> %s at distance %.1f\n", ent->classname, distance);
	}
}

void
BY_DecryptLog(edict_t *player, const char *log_data)
{
	if (!player || !log_data)
		return;

	gi.dprintf("Decrypting log: %s\n", log_data);
}
