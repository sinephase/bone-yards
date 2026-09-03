#pragma once

#include <stdint.h>
#include <string>
#include <vector>

/* Console variable flags */
#define CVAR_ARCHIVE     0x0001  /* Saved to config */
#define CVAR_SERVERINFO  0x0002  /* Sent to server on connect */
#define CVAR_USERINFO    0x0004  /* Sent to server on update */
#define CVAR_READONLY    0x0008  /* Cannot be changed */
#define CVAR_CHEAT       0x0010  /* Can only be used if cheats enabled */
#define CVAR_FORCE       0x0020  /* Force overwrite of an existing value */

namespace engine {

struct cvar_t {
    std::string name;
    std::string value;
    std::string default_value;
    std::string description;
    int flags = 0;
    float float_value = 0.0f;
    int int_value = 0;
    void (*callback)(cvar_t *) = nullptr;
};

cvar_t *Cvar_Get(const std::string& name, const std::string& value, int flags, const std::string& description);
void Cvar_Set(const std::string& name, const std::string& value);
void Cvar_SetFloat(const std::string& name, float value);
void Cvar_SetInt(const std::string& name, int value);

cvar_t *Cvar_Find(const std::string& name);
float Cvar_VariableValue(const std::string& name);
std::string Cvar_VariableString(const std::string& name);
int Cvar_VariableIntValue(const std::string& name);
std::vector<cvar_t *> Cvar_GetList(void);

std::string Cvar_WriteVariables(void);
void Cvar_LoadArchive(const std::string& script);
void Cvar_Shutdown(void);

}  // namespace engine
