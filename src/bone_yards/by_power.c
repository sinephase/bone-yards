/*
 * =======================================================================
 *
 * Bone Yards - Power System
 *
 * =======================================================================
 */

#include "by_local.h"

void
BY_InitPowerSystem(void)
{
	gi.dprintf("Initializing power system...\n");

	bone_yards.level.num_power_zones = 0;
	bone_yards.level.global_radiation = 0.0f;
	bone_yards.level.command_deck_activated = 0;
	bone_yards.level.main_power_restored = 0;
}

void
BY_ActivatePowerZone(int zone_id)
{
	power_zone_t *zone;

	if (zone_id < 0 || zone_id >= bone_yards.level.num_power_zones)
		return;

	zone = &bone_yards.level.power_zones[zone_id];

	if (zone->active)
		return;

	zone->active = true;
	zone->activation_time = level.time;
	zone->power_level = 50;

	gi.dprintf("POWER ZONE ACTIVATED: %s (ID:%d)\n", zone->name, zone_id);

	if (zone->on_activate)
		zone->on_activate(zone_id);

	BY_PropagateRadiation(zone_id, zone->radiation_level);
}

void
BY_DeactivatePowerZone(int zone_id)
{
	power_zone_t *zone;

	if (zone_id < 0 || zone_id >= bone_yards.level.num_power_zones)
		return;

	zone = &bone_yards.level.power_zones[zone_id];

	if (!zone->active)
		return;

	zone->active = false;
	zone->power_level = 0;

	gi.dprintf("POWER ZONE DEACTIVATED: %s (ID:%d)\n", zone->name, zone_id);

	if (zone->on_deactivate)
		zone->on_deactivate(zone_id);
}

void
BY_SetPowerLevel(int zone_id, int level_val)
{
	power_zone_t *zone;

	if (zone_id < 0 || zone_id >= bone_yards.level.num_power_zones)
		return;

	zone = &bone_yards.level.power_zones[zone_id];
	zone->power_level = level_val < 0 ? 0 : (level_val > 100 ? 100 : level_val);

	zone->radiation_level = (zone->power_level / 100.0f) * 25.0f;
}

int
BY_GetRadiationLevel(int zone_id)
{
	if (zone_id < 0 || zone_id >= bone_yards.level.num_power_zones)
		return 0;

	return (int)bone_yards.level.power_zones[zone_id].radiation_level;
}

void
BY_PropagateRadiation(int zone_id, float intensity)
{
	int i, connected_id;
	power_zone_t *zone;

	if (zone_id < 0 || zone_id >= bone_yards.level.num_power_zones)
		return;

	zone = &bone_yards.level.power_zones[zone_id];

	bone_yards.level.global_radiation += intensity * 0.1f;

	for (i = 0; i < zone->num_connected; i++)
	{
		connected_id = zone->connected_zones[i];
		if (connected_id >= 0 && connected_id < bone_yards.level.num_power_zones)
		{
			bone_yards.level.power_zones[connected_id].radiation_level += intensity * 0.5f;
		}
	}
}

void
BY_UpdatePowerZones(void)
{
	int i;
	power_zone_t *zone;

	for (i = 0; i < bone_yards.level.num_power_zones; i++)
	{
		zone = &bone_yards.level.power_zones[i];

		if (!zone->active)
			continue;

		if (zone->radiation_level > 0)
		{
			zone->radiation_level -= 0.01f;
		}
	}
}
