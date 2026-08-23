/*
 * =======================================================================
 *
 * Bone Yards - Input System
 *
 * Player input handling: WASD movement, mouse look, combat controls
 *
 * =======================================================================
 */

#ifndef BY_INPUT_H
#define BY_INPUT_H

/* ====================================================================== */
/* INPUT STATE */
/* ====================================================================== */

typedef enum {
	WEAPON_NONE = 0,
	WEAPON_SCANNER,
	WEAPON_GUN,
	WEAPON_SWORD,
	WEAPON_MAX
} weapon_type_t;

typedef enum {
	ATTACK_STATE_IDLE = 0,
	ATTACK_STATE_AIMING,
	ATTACK_STATE_FIRING,
	ATTACK_STATE_CHARGING_SLASH,
	ATTACK_STATE_LIGHT_SLASH,
	ATTACK_STATE_HEAVY_SLASH
} attack_state_t;

typedef struct {
	/* Movement input */
	float forward;      /* -1 to 1 (W/S keys) */
	float strafe;       /* -1 to 1 (A/D keys) */
	float upward;       /* -1 to 1 (Space/Ctrl) */

	/* Look input */
	float pitch;        /* Mouse Y rotation */
	float yaw;          /* Mouse X rotation */

	/* Combat state */
	weapon_type_t current_weapon;
	attack_state_t attack_state;
	qboolean aiming;           /* Right click held (for guns) */
	qboolean firing;           /* Left click held */
	qboolean slash_charged;    /* Right click held (for swords) */

	/* Timing */
	float fire_cooldown;
	float slash_charge_time;
	float last_attack_time;
} player_input_t;

typedef struct {
	/* Weapon data */
	weapon_type_t equipped_weapon;
	int ammo_count;
	float fire_rate;
	float damage;
	float range;

	/* Melee data (swords) */
	float light_slash_damage;
	float heavy_slash_damage;
	float slash_cooldown;

	/* State */
	player_input_t input;
	float movement_speed;
	float max_health;
	float stamina;
	float max_stamina;
} player_state_t;

/* ====================================================================== */
/* FUNCTION DECLARATIONS */
/* ====================================================================== */

/* by_input.c */
void BY_InitPlayerInput(edict_t *player);
void BY_UpdatePlayerInput(edict_t *player, usercmd_t *cmd);
void BY_ProcessMovement(edict_t *player, player_input_t *input);
void BY_ProcessCombat(edict_t *player, player_input_t *input);

/* Weapon actions */
void BY_FireGun(edict_t *player, vec3_t forward);
void BY_LightSlash(edict_t *player, vec3_t forward);
void BY_HeavySlash(edict_t *player, vec3_t forward);
void BY_ScanWithScanner(edict_t *player, vec3_t forward);

/* Animation/state helpers */
void BY_SetPlayerAnimation(edict_t *player, const char *anim_name);
void BY_PlaySound(edict_t *origin, const char *sound_name, float volume);

#endif /* BY_INPUT_H */
