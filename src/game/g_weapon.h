#pragma once

#include "g_local.h"

/* Weapon types */
typedef struct weapon_s {
    char *name;
    char *ammo_type;
    int ammo_per_shot;
    float firerate;
    int damage;
    int knockback;
} weapon_t;

/* Weapons */
weapon_t *W_FindWeapon(const char *name);
void W_Fire(edict_t *ent, weapon_t *weap);
void W_Select(edict_t *ent, const char *weapon_name);
