/*
 * =======================================================================
 *
 * Bone Yards - Physics & Movement
 *
 * Player physics, gravity, collision detection
 *
 * =======================================================================
 */

#include "by_local.h"

/* ====================================================================== */
/* MOVEMENT PHYSICS */
/* ====================================================================== */

/*
 * Apply gravity and ground collision to player
 */
void
BY_ApplyGravity(edict_t *player)
{
	if (!player || !player->client)
		return;

	/* Check if player is on ground */
	if (player->groundentity)
	{
		/* On ground: minimal vertical velocity */
		if (player->velocity[2] < 0)
			player->velocity[2] = 0;
	}
	else
	{
		/* In air: apply gravity */
		player->velocity[2] -= sv_gravity->value * FRAMETIME;

		/* Terminal velocity cap */
		if (player->velocity[2] < -320.0f)
			player->velocity[2] = -320.0f;
	}
}

/*
 * Check if player is standing on ground
 */
qboolean
BY_IsOnGround(edict_t *player)
{
	if (!player)
		return false;

	/* If groundentity is set, player is on ground */
	if (player->groundentity)
		return true;

	return false;
}

/*
 * Handle player jumping
 */
void
BY_HandleJump(edict_t *player)
{
	if (!player || !player->client)
		return;

	/* Only jump if on ground */
	if (!BY_IsOnGround(player))
		return;

	/* Apply upward velocity */
	player->velocity[2] = 270.0f;  /* Quake II standard jump height */

	/* Leave ground */
	player->groundentity = NULL;

	BY_PlaySound(player, "player/jump.wav", 0.8f);
}

/*
 * Velocity limiter - prevents player from moving too fast
 */
void
BY_LimitVelocity(edict_t *player, float max_speed)
{
	float speed;
	vec3_t vel_2d;

	if (!player)
		return;

	/* Get horizontal velocity (ignore vertical for jump/fall) */
	vel_2d[0] = player->velocity[0];
	vel_2d[1] = player->velocity[1];
	vel_2d[2] = 0;

	speed = VectorLength(vel_2d);

	/* Scale down if over limit */
	if (speed > max_speed)
	{
		float scale = max_speed / speed;
		player->velocity[0] *= scale;
		player->velocity[1] *= scale;
	}
}

/*
 * Calculate player position each frame
 */
void
BY_UpdatePlayerPhysics(edict_t *player)
{
	if (!player || !player->client)
		return;

	/* Apply gravity */
	BY_ApplyGravity(player);

	/* Limit velocity */
	BY_LimitVelocity(player, 500.0f);

	/* TODO: Run player collision checks */
	/* TODO: Update groundentity based on trace */
}
