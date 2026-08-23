#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Console variable flags */
#define CVAR_ARCHIVE     0x0001  /* Saved to config */
#define CVAR_SERVERINFO  0x0002  /* Sent to server on connect */
#define CVAR_USERINFO    0x0004  /* Sent to server on update */
#define CVAR_READONLY    0x0008  /* Cannot be changed */
#define CVAR_CHEAT       0x0010  /* Can only be used if cheats enabled */

typedef struct cvar_s {
    char *name;
    char *string;
    char *latched_string;  /* For ARCHIVE vars, changed next frame */
    float value;
    int integer;
    uint32_t flags;
    struct cvar_s *next;
} cvar_t;

cvar_t *Cvar_Get(const char *name, const char *value, uint32_t flags, const char *description);
void Cvar_Set(const char *name, const char *value);
void Cvar_SetValue(const char *name, float value);
void Cvar_SetInteger(const char *name, int value);

float Cvar_VariableValue(const char *name);
int Cvar_VariableInteger(const char *name);
const char *Cvar_VariableString(const char *name);

cvar_t *Cvar_FindVar(const char *name);

void Cvar_WriteVariables(void);
void Cvar_LoadArchive(const char *buffer);
void Cvar_Reset(cvar_t *var);
