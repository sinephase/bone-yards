/*
 * =======================================================================
 *
 * Bone Yards - Player Input & Movement System
 *
 * WASD movement, mouse look, weapon switching, gun/sword combat
 *
 * =======================================================================
 */

#include "by_local.h"
#include "by_input.h"

/* ====================================================================== */
/* PLAYER INPUT INITIALIZATION */
/* ====================================================================== */

void
BY_InitPlayerInput(edict_t *player)
{
	player_state_t *state;

	if (!player)
		return;

	state = (player_state_t *)G_Alloc(sizeof(player_state_t));
	memset(state, 0, sizeof(player_state_t));

	/* Weapon setup */
	state->equipped_weapon = WEAPON_GUN;
	state->ammo_count = 30;
	state->fire_rate = 0.1f;    /* 100ms between shots */
	state->damage = 15.0f;
	state->range = 1024.0f;

	/* Melee setup */
	state->light_slash_damage = 20.0f;
	state->heavy_slash_damage = 40.0f;
	state->slash_cooldown = 0.8f;

	/* Movement */
	state->movement_speed = 320.0f;  /* Quake II units per second */
	state->max_health = 100.0f;
	state->stamina = 100.0f;
	state->max_stamina = 100.0f;

	/* Input state */
	state->input.current_weapon = WEAPON_GUN;
	state->input.attack_state = ATTACK_STATE_IDLE;

	/* Store in player entity */
	player->priv = state;

	gi.dprintf("Player input system initialized\n");
}

/* ====================================================================== */
/* INPUT PROCESSING */
/* ====================================================================== */

/*
 * Update player input from client commands
 */
void
BY_UpdatePlayerInput(edict_t *player, usercmd_t *cmd)
{
	player_state_t *state;
	player_input_t *input;
	float forward_scale, strafe_scale;

	if (!player || !player->client)
		return;

	state = (player_state_t *)player->priv;
	if (!state)
		return;

	input = &state->input;

	/* ====== MOVEMENT INPUT (WASD) ====== */
	/* Normalize movement to -1 to 1 range */
	forward_scale = (cmd->forwardmove > 0) ? 1.0f : (cmd->forwardmove < 0) ? -1.0f : 0.0f;
	strafe_scale = (cmd->rightmove > 0) ? 1.0f : (cmd->rightmove < 0) ? -1.0f : 0.0f;

	input->forward = forward_scale;
	input->strafe = strafe_scale;
	input->upward = (cmd->upmove > 0) ? 1.0f : (cmd->upmove < 0) ? -1.0f : 0.0f;

	/* ====== LOOK INPUT (MOUSE) ====== */
	/* Pitch (up/down) and Yaw (left/right) from mouse */
	input->pitch = -cmd->angles[0] * 0.0174533f;  /* Convert degrees to radians */
	input->yaw = cmd->angles[1] * 0.0174533f;

	/* ====== WEAPON SWITCHING ====== */
	/* 1-4 keys to switch weapons */
	if (cmd->buttons & BUTTON_1)
		state->equipped_weapon = WEAPON_SCANNER;
	else if (cmd->buttons & BUTTON_2)
		state->equipped_weapon = WEAPON_GUN;
	else if (cmd->buttons & BUTTON_3)
		state->equipped_weapon = WEAPON_SWORD;

	/* ====== COMBAT INPUT ====== */
	/* LEFT CLICK (attack/fire) */
	if (cmd->buttons & BUTTON_ATTACK)
	{
		input->firing = true;
	}
	else
	{
		input->firing = false;
	}

	/* RIGHT CLICK (aim for guns, slash charge for swords) */
	if (cmd->buttons & BUTTON_ATTACK2)
	{
		if (state->equipped_weapon == WEAPON_GUN)
		{
			input->aiming = true;
		}
		else if (state->equipped_weapon == WEAPON_SWORD)
		{
			input->slash_charged = true;
			input->slash_charge_time += 0.016f;  /* ~60 FPS */
		}
	}
	else
	{
		if (input->aiming || input->slash_charged)
		{
			/* Release of right click triggers action */
			input->aiming = false;

			if (input->slash_charged)
			{
				/* Charge time determines slash type */
				if (input->slash_charge_time > 0.5f)
					input->attack_state = ATTACK_STATE_HEAVY_SLASH;
				else
					input->attack_state = ATTACK_STATE_LIGHT_SLASH;

				input->slash_charge_time = 0.0f;
				input->slash_charged = false;
			}
		}
	}

	/* Process movement and combat */
	BY_ProcessMovement(player, input);
	BY_ProcessCombat(player, input);
}

/* ====================================================================== */
/* MOVEMENT PROCESSING */
/* ====================================================================== */

void
BY_ProcessMovement(edict_t *player, player_input_t *input)
{
	vec3_t forward, right, up, move_vector;
	player_state_t *state;
	float speed;

	if (!player)
		return;

	state = (player_state_t *)player->priv;
	if (!state)
		return;

	/* Get player's view direction vectors */
	AngleVectors(player->s.angles, forward, right, up);

	/* Build movement vector from WASD input */
	move_vector[0] = 0;
	move_vector[1] = 0;
	move_vector[2] = 0;

	/* Forward/Backward (W/S) */
	VectorMA(move_vector, input->forward * state->movement_speed, forward, move_vector);

	/* Strafe Left/Right (A/D) */
	VectorMA(move_vector, input->strafe * state->movement_speed, right, move_vector);

	/* Up/Down (Space/Ctrl) - only in zero-G areas */
	VectorMA(move_vector, input->upward * state->movement_speed * 0.5f, up, move_vector);

	/* Apply movement to player velocity */
	VectorCopy(move_vector, player->velocity);

	/* Update player angles based on mouse look */
	player->s.angles[0] = input->pitch * 180.0f / M_PI;  /* Convert back to degrees */
	player->s.angles[1] = input->yaw * 180.0f / M_PI;

	/* Clamp pitch to prevent flipping */
	if (player->s.angles[0] > 89.0f)
		player->s.angles[0] = 89.0f;
	if (player->s.angles[0] < -89.0f)
		player->s.angles[0] = -89.0f;
}

/* ====================================================================== */
/* COMBAT PROCESSING */
/* ====================================================================== */

void
BY_ProcessCombat(edict_t *player, player_input_t *input)
{
	player_state_t *state;
	vec3_t forward, right, up;
	float time_since_attack;

	if (!player)
		return;

	state = (player_state_t *)player->priv;
	if (!state)
		return;

	/* Get player's forward direction */
	AngleVectors(player->s.angles, forward, right, up);

	time_since_attack = level.time - input->last_attack_time;

	/* Update weapon-specific combat states */
	switch (state->equipped_weapon)
	{
		case WEAPON_GUN:
			/* Gun: Left click to fire, Right click to aim */
			if (input->firing && time_since_attack >= state->fire_rate)
			{
				BY_FireGun(player, forward);
				input->last_attack_time = level.time;
			}

			if (input->aiming)
			{
				input->attack_state = ATTACK_STATE_AIMING;
			}
			else
			{
				input->attack_state = ATTACK_STATE_IDLE;
			}
			break;

		case WEAPON_SWORD:
			/* Sword: Left click for light attack, Right click to charge heavy */
			if (input->firing && time_since_attack >= state->slash_cooldown)
			{
				/* Quick light slash on left click */
				BY_LightSlash(player, forward);
				input->last_attack_time = level.time;
				input->attack_state = ATTACK_STATE_LIGHT_SLASH;
			}

			/* Charged heavy slash on right click release */
			if (input->attack_state == ATTACK_STATE_HEAVY_SLASH &&
				time_since_attack >= state->slash_cooldown)
			{
				BY_HeavySlash(player, forward);
				input->last_attack_time = level.time;
			}

			/* Charging animation */
			if (input->slash_charged)
			{
				input->attack_state = ATTACK_STATE_CHARGING_SLASH;
			}
			break;

		case WEAPON_SCANNER:
			/* Scanner: Left click to scan */
			if (input->firing && time_since_attack >= 0.5f)
			{
				BY_ScanWithScanner(player, forward);
				input->last_attack_time = level.time;
			}
			break;

		default:
			input->attack_state = ATTACK_STATE_IDLE;
			break;
	}
}

/* ====================================================================== */
/* WEAPON ACTIONS */
/* ====================================================================== */

/*
 * Fire a gun projectile
 */
void
BY_FireGun(edict_t *player, vec3_t forward)
{
	edict_t *projectile;
	vec3_t start, dir;
	player_state_t *state;

	if (!player)
		return;

	state = (player_state_t *)player->priv;
	if (!state || state->ammo_count <= 0)
		return;

	/* Calculate projectile start position (from player's gun) */
	VectorCopy(player->s.origin, start);
	start[2] += 28;  /* Approximate gun height */
	VectorMA(start, 16, forward, start);

	/* Fire direction (with slight inaccuracy if not aiming) */
	VectorCopy(forward, dir);

	if (!((player_input_t *)&state->input)->aiming)
	{
		/* Add small spread if not aiming */
		dir[0] += (random() - 0.5f) * 0.1f;
		dir[1] += (random() - 0.5f) * 0.1f;
		VectorNormalize(dir);
	}

	/* Spawn projectile */
	projectile = G_Spawn();
	if (projectile)
	{
		projectile->classname = "projectile_bullet";
		projectile->owner = player;
		VectorCopy(start, projectile->s.origin);
		VectorScale(dir, 1000.0f, projectile->velocity);
		projectile->health = 100;  /* Distance before despawn */
	}

	state->ammo_count--;
	BY_PlaySound(player, "weapons/gun_fire.wav", 1.0f);

	gi.dprintf("Gun fired! Ammo: %d\n", state->ammo_count);
}

/*
 * Light sword slash (quick, less damage)
 */
void
BY_LightSlash(edict_t *player, vec3_t forward)
{
	vec3_t slash_origin, slash_end;
	int i;
	edict_t *ent;
	vec3_t delta;
	float distance;
	player_state_t *state;
	const float SLASH_RANGE = 64.0f;

	if (!player)
		return;

	state = (player_state_t *)player->priv;
	if (!state)
		return;

	/* Slash originates from player's position */
	VectorCopy(player->s.origin, slash_origin);
	slash_origin[2] += 28;
	VectorMA(slash_origin, SLASH_RANGE, forward, slash_end);

	/* Check for enemies in slash arc */
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse || ent == player)
			continue;

		if (!(ent->svflags & SVF_MONSTER))
			continue;

		VectorSubtract(ent->s.origin, slash_origin, delta);
		distance = VectorLength(delta);

		if (distance <= SLASH_RANGE)
		{
			ent->health -= (int)state->light_slash_damage;
			gi.dprintf("Light slash hit! Damage: %.0f, Enemy health: %d\n",
				state->light_slash_damage, ent->health);
		}
	}

	BY_PlaySound(player, "weapons/sword_light.wav", 0.8f);
}

/*
 * Heavy sword slash (slower, more damage, requires charging)
 */
void
BY_HeavySlash(edict_t *player, vec3_t forward)
{
	vec3_t slash_origin, slash_end;
	int i;
	edict_t *ent;
	vec3_t delta;
	float distance;
	player_state_t *state;
	const float HEAVY_SLASH_RANGE = 96.0f;

	if (!player)
		return;

	state = (player_state_t *)player->priv;
	if (!state)
		return;

	/* Heavy slash has longer reach */
	VectorCopy(player->s.origin, slash_origin);
	slash_origin[2] += 28;
	VectorMA(slash_origin, HEAVY_SLASH_RANGE, forward, slash_end);

	/* Check for enemies in extended slash arc */
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse || ent == player)
			continue;

		if (!(ent->svflags & SVF_MONSTER))
			continue;

		VectorSubtract(ent->s.origin, slash_origin, delta);
		distance = VectorLength(delta);

		if (distance <= HEAVY_SLASH_RANGE)
		{
			ent->health -= (int)state->heavy_slash_damage;
			gi.dprintf("Heavy slash hit! Damage: %.0f, Enemy health: %d\n",
				state->heavy_slash_damage, ent->health);
		}
	}

	BY_PlaySound(player, "weapons/sword_heavy.wav", 1.0f);
}

/*
 * Scanner ability - reveals enemy information
 */
void
BY_ScanWithScanner(edict_t *player, vec3_t forward)
{
	int i;
	edict_t *ent;
	vec3_t delta;
	float distance;
	const float SCAN_RANGE = 512.0f;

	if (!player)
		return;

	gi.dprintf("Scanning environment...\n");

	/* Find and log all entities in range */
	for (i = 0; i < globals.num_edicts; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;

		VectorSubtract(ent->s.origin, player->s.origin, delta);
		distance = VectorLength(delta);

		if (distance > SCAN_RANGE)
			continue;

		/* Log entity info */
		if (ent->svflags & SVF_MONSTER)
		{
			gi.dprintf("  SCAN: %s at %.1f units, Health: %d\n",
				ent->classname, distance, ent->health);
		}
		else if (ent->classname && strstr(ent->classname, "power_"))
		{
			gi.dprintf("  SCAN: Power system at %.1f units\n", distance);
		}
	}

	BY_PlaySound(player, "equipment/scanner.wav", 0.7f);
}

/* ====================================================================== */
/* ANIMATION & AUDIO HELPERS */
/* ====================================================================== */

void
BY_SetPlayerAnimation(edict_t *player, const char *anim_name)
{
	if (!player || !anim_name)
		return;

	/* TODO: Set skeletal animation frame/state */
	gi.dprintf("Player animation: %s\n", anim_name);
}

void
BY_PlaySound(edict_t *origin, const char *sound_name, float volume)
{
	if (!origin || !sound_name)
		return;

	/* TODO: Queue sound for playback */
	gi.dprintf("Playing sound: %s (volume: %.1f)\n", sound_name, volume);
}
