/*
 * =======================================================================
 *
 * Bone Yards - Environmental Hazards
 *
 * =======================================================================
 */

#include "by_local.h"

static environmental_hazard_t hazards[BONE_YARDS_MAX_HAZARDS];
static int num_hazards = 0;

void
BY_InitHazards(void)
{
	memset(hazards, 0, sizeof(hazards));
	num_hazards = 0;

	gi.dprintf("Initialized hazard system\n");
}

void
BY_UpdateHazards(void)
{
	int i;
	environmental_hazard_t *hazard;

	for (i = 0; i < num_hazards; i++)
	{
		hazard = &hazards[i];

		if (!hazard->is_active)
			continue;
	}
}

void
BY_ApplyRadiationDamage(edict_t *target, float intensity)
{
	if (!target)
		return;

	if (target->client)
	{
		target->health -= (int)(intensity * 0.5f);
		gi.dprintf("Player took %.1f radiation damage\n", intensity * 0.5f);
	}
	else if (target->svflags & SVF_MONSTER)
	{
		if (Q_stricmp(target->classname, "monster_nuclear_slug") == 0)
		{
			gi.dprintf("%s absorbed radiation, gaining power\n", target->classname);
		}
		else
		{
			target->health -= (int)(intensity * 0.25f);
		}
	}
}
