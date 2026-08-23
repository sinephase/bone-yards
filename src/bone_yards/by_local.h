/*
 * =======================================================================
 *
 * Bone Yards - Local Header
 *
 * Entity definitions, game-specific structures, and constants
 *
 * =======================================================================
 */

#ifndef BY_LOCAL_H
#define BY_LOCAL_H

#include "../game/game.h"

#define BY_VERSION "1.0.0"
#define BY_PROTOCOL_VERSION 34

#define BONE_YARDS_MAX_POWER_ZONES 32
#define BONE_YARDS_MAX_ENEMIES 256
#define BONE_YARDS_MAX_HAZARDS 64

typedef enum {
	ENTITY_PLAYER = 0,
	ENTITY_MECHALICHEN_WORKER,
	ENTITY_MECHALICHEN_SOLDIER,
	ENTITY_NUCLEAR_SLUG,
	ENTITY_HAZARD_RADIATION,
	ENTITY_POWER_NODE,
	ENTITY_DOOR,
	ENTITY_LIFT,
	ENTITY_COMPUTER_TERMINAL,
	ENTITY_ITEM,
	ENTITY_PROJECTILE,
	ENTITY_EFFECT,
	ENTITY_WORLDSPAWN,
	ENTITY_MAX_TYPES
} bone_yards_entity_type_t;

typedef struct {
	int id;
	char name[64];
	vec3_t origin;
	int power_level;
	int max_power_output;
	qboolean active;
	qboolean player_controlled;
	int connected_zones[8];
	int num_connected;
	float radiation_level;
	float activation_time;
	void (*on_activate)(int zone_id);
	void (*on_deactivate)(int zone_id);
} power_zone_t;

typedef struct {
	edict_t *owner;
	int ai_state;
	int behavior_flags;
	edict_t *current_target;
	float target_distance;
	float awareness_level;
	int power_dependency;
	float power_boost;
	int health_max;
	int aggression;
	float last_attack_time;
} enemy_ai_t;

typedef struct {
	enemy_ai_t ai;
	int tentacle_count;
	int infection_spread_rate;
	int system_hijack_attempts;
} mechalichen_t;

typedef struct {
	enemy_ai_t ai;
	float irradiation_level;
	int tail_strike_power;
	qboolean is_charged;
	float charge_time;
} nuclear_slug_t;

typedef struct {
	int scan_charge;
	int decrypt_progress;
	int current_access_level;
	vec3_t last_scan_origin;
	int inventory_slots[16];
	int num_inventory;
} player_archaeology_t;

typedef struct {
	int hazard_type;
	vec3_t origin;
	float radius;
	float intensity;
	int affected_zone_id;
	qboolean is_active;
	float activation_time;
} environmental_hazard_t;

typedef struct {
	qboolean editor_active;
	int selected_entity_id;
	int editor_mode;
	vec3_t gizmo_position;
	float property_editing_value;
	char property_editing_key[128];
} editor_state_t;

typedef struct {
	power_zone_t power_zones[BONE_YARDS_MAX_POWER_ZONES];
	int num_power_zones;
	float global_radiation;
	int command_deck_activated;
	int main_power_restored;
	int boss_defeated;
	int mission_complete;
	char current_map[256];
} bone_yards_level_t;

typedef struct {
	bone_yards_level_t level;
	editor_state_t editor;
	int total_enemies_spawned;
	int enemies_defeated;
	float mission_time;
	int difficulty_scaling;
} bone_yards_game_t;

extern bone_yards_game_t bone_yards;

void BY_InitGame(void);
void BY_ShutdownGame(void);
void BY_RunFrame(void);

void BY_InitPowerSystem(void);
void BY_UpdatePowerZones(void);
void BY_ActivatePowerZone(int zone_id);
void BY_DeactivatePowerZone(int zone_id);
void BY_SetPowerLevel(int zone_id, int level);
int BY_GetRadiationLevel(int zone_id);
void BY_PropagateRadiation(int zone_id, float intensity);

void BY_InitPlayer(edict_t *player);
void BY_UpdatePlayerAbilities(edict_t *player);
void BY_ScanEnvironment(edict_t *player, vec3_t scan_origin);
void BY_DecryptLog(edict_t *player, const char *log_data);

edict_t *BY_SpawnMechalichen(vec3_t origin, int difficulty);
edict_t *BY_SpawnNuclearSlug(vec3_t origin, int difficulty);
void BY_UpdateEnemyAI(edict_t *enemy);
void BY_EnemyThink(edict_t *enemy);

void BY_EditorInit(void);
void BY_EditorDraw(void);
void BY_EditorUpdate(edict_t *player, usercmd_t *cmd);
void BY_EditorSpawnEntity(const char *classname, vec3_t origin);
void BY_EditorDeleteEntity(int entity_id);
void BY_EditorSetProperty(int entity_id, const char *key, const char *value);
void BY_EditorSaveLevel(const char *filename);
void BY_EditorLoadLevel(const char *filename);

void BY_InitHazards(void);
void BY_UpdateHazards(void);
void BY_ApplyRadiationDamage(edict_t *target, float intensity);

void BY_SpawnEntities(const char *mapname, const char *entities, const char *spawnpoint);

#endif /* BY_LOCAL_H */
