// g_spawn.cpp -- entity spawning and map loading
// Parses entity data from map files and spawns entities with proper initialization.
// Handles entity class dispatch and spawn callbacks.

#include "qcommon.h"
#include "game.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <glm/glm.hpp>
#include <cstdio>

namespace game {

extern edict_t* g_edicts;
extern int g_num_edicts;

// Entity spawn function table
struct spawn_func_t {
    const char* name;
    void (*func)(edict_t* ent);
};

// Forward declarations of entity spawn functions
static void SP_worldspawn(edict_t* ent);
static void SP_player_start(edict_t* ent);
static void SP_info_player_start(edict_t* ent);
static void SP_func_door(edict_t* ent);
static void SP_func_platform(edict_t* ent);
static void SP_monster_spawner(edict_t* ent);
static void SP_weapon_railgun(edict_t* ent);
static void SP_item_health_small(edict_t* ent);
static void SP_trigger_multiple(edict_t* ent);

// Spawn function dispatch table
static const spawn_func_t spawn_table[] = {
    {"worldspawn", SP_worldspawn},
    {"player_start", SP_player_start},
    {"info_player_start", SP_info_player_start},
    {"func_door", SP_func_door},
    {"func_platform", SP_func_platform},
    {"monster_spawner", SP_monster_spawner},
    {"weapon_railgun", SP_weapon_railgun},
    {"item_health_small", SP_item_health_small},
    {"trigger_multiple", SP_trigger_multiple},
    {nullptr, nullptr}
};

// Parse key-value pairs from entity data
static void ED_ParseEntity(const std::string& entity_str, std::unordered_map<std::string, std::string>& kvpairs) {
    std::istringstream iss(entity_str);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '/') continue;

        // Parse "key" "value"
        size_t first_quote = line.find('"');
        if (first_quote == std::string::npos) continue;

        size_t second_quote = line.find('"', first_quote + 1);
        if (second_quote == std::string::npos) continue;

        std::string key = line.substr(first_quote + 1, second_quote - first_quote - 1);

        size_t third_quote = line.find('"', second_quote + 1);
        if (third_quote == std::string::npos) continue;

        size_t fourth_quote = line.find('"', third_quote + 1);
        if (fourth_quote == std::string::npos) continue;

        std::string value = line.substr(third_quote + 1, fourth_quote - third_quote - 1);

        kvpairs[key] = value;
    }
}

// Get key-value with default
static std::string ED_GetString(const std::unordered_map<std::string, std::string>& kvpairs, 
                                const std::string& key, const std::string& defval = "") {
    auto it = kvpairs.find(key);
    return it != kvpairs.end() ? it->second : defval;
}

static float ED_GetFloat(const std::unordered_map<std::string, std::string>& kvpairs,
                         const std::string& key, float defval = 0.0f) {
    auto it = kvpairs.find(key);
    if (it != kvpairs.end()) {
        try {
            return std::stof(it->second);
        } catch (...) {
            return defval;
        }
    }
    return defval;
}

static int ED_GetInt(const std::unordered_map<std::string, std::string>& kvpairs,
                     const std::string& key, int defval = 0) {
    auto it = kvpairs.find(key);
    if (it != kvpairs.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defval;
        }
    }
    return defval;
}

// Parse vector from string "x y z"
static glm::vec3 ED_GetVector(const std::unordered_map<std::string, std::string>& kvpairs,
                              const std::string& key, const glm::vec3& defval = {0, 0, 0}) {
    auto it = kvpairs.find(key);
    if (it != kvpairs.end()) {
        glm::vec3 v;
        int n = sscanf(it->second.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
        if (n == 3) return v;
    }
    return defval;
}

// Entity spawn functions
static void SP_worldspawn(edict_t* ent) {
    ent->solid = SOLID_NOT;
    ent->movetype = MOVETYPE_NONE;
    Com_Printf("Worldspawn entity created\n");
}

static void SP_player_start(edict_t* ent) {
    ent->solid = SOLID_NOT;
    ent->movetype = MOVETYPE_NONE;
    Com_Printf("Player start at %.0f %.0f %.0f\n", ent->origin.x, ent->origin.y, ent->origin.z);
}

static void SP_info_player_start(edict_t* ent) {
    SP_player_start(ent);
}

static void SP_func_door(edict_t* ent) {
    ent->solid = SOLID_BBOX;
    ent->movetype = MOVETYPE_PUSH;
    ent->health = 100;
    ent->maxhealth = 100;
    ent->s.modelindex = 10;  // Door model
    
    // Parse door-specific properties
    // TODO: door speed, target, etc.
    
    Com_Printf("Door spawned\n");
}

static void SP_func_platform(edict_t* ent) {
    ent->solid = SOLID_BBOX;
    ent->movetype = MOVETYPE_PUSH;
    ent->s.modelindex = 11;  // Platform model
    Com_Printf("Platform spawned\n");
}

static void SP_monster_spawner(edict_t* ent) {
    ent->solid = SOLID_NOT;
    ent->movetype = MOVETYPE_NONE;
    Com_Printf("Monster spawner created\n");
}

static void SP_weapon_railgun(edict_t* ent) {
    ent->solid = SOLID_TRIGGER;
    ent->movetype = MOVETYPE_NONE;
    ent->s.modelindex = 20;  // Weapon model
    ent->contents = MASK_WEAPON;
    Com_Printf("Railgun weapon spawned\n");
}

static void SP_item_health_small(edict_t* ent) {
    ent->solid = SOLID_TRIGGER;
    ent->movetype = MOVETYPE_NONE;
    ent->s.modelindex = 30;  // Item model
    ent->contents = MASK_ITEM;
    Com_Printf("Health item spawned\n");
}

static void SP_trigger_multiple(edict_t* ent) {
    ent->solid = SOLID_TRIGGER;
    ent->movetype = MOVETYPE_NONE;
    Com_Printf("Trigger spawned\n");
}

// Spawn an entity from key-value pairs
edict_t* ED_Spawn(const std::unordered_map<std::string, std::string>& kvpairs) {
    // Get entity class
    std::string classname = ED_GetString(kvpairs, "classname");
    if (classname.empty()) {
        Com_Printf("ED_Spawn: no classname\n");
        return nullptr;
    }

    // Find spawn function
    void (*spawn_func)(edict_t*) = nullptr;
    for (int i = 0; spawn_table[i].name; i++) {
        if (classname == spawn_table[i].name) {
            spawn_func = spawn_table[i].func;
            break;
        }
    }

    if (!spawn_func) {
        Com_Printf("ED_Spawn: unknown classname %s\n", classname.c_str());
        return nullptr;
    }

    // Allocate entity
    edict_t* ent = G_Spawn();
    if (!ent) return nullptr;

    // Set common properties
    ent->classname = classname;
    ent->origin = ED_GetVector(kvpairs, "origin", {0, 0, 0});
    ent->angles = ED_GetVector(kvpairs, "angles", {0, 0, 0});
    ent->scale = ED_GetFloat(kvpairs, "scale", 1.0f);

    // Default bounding box
    ent->mins = {-16, -16, -16};
    ent->maxs = {16, 16, 16};

    // Call spawn function to set up entity-specific properties
    spawn_func(ent);

    // Link to world
    SV_LinkEdict(ent);

    return ent;
}

// Load map entities from a map file
void G_SpawnEntities(const char* mapname) {
    Com_Printf("Loading entities from map: %s\n", mapname);

    // TODO: Load from actual map file (likely BSP or JSON format)
    // For now, spawn a test world

    // Create worldspawn
    std::unordered_map<std::string, std::string> world_kvpairs;
    world_kvpairs["classname"] = "worldspawn";
    world_kvpairs["sky"] = "sky1";
    ED_Spawn(world_kvpairs);

    // Create a player start
    std::unordered_map<std::string, std::string> player_start_kvpairs;
    player_start_kvpairs["classname"] = "player_start";
    player_start_kvpairs["origin"] = "0 0 100";
    ED_Spawn(player_start_kvpairs);

    // Create some test entities
    std::unordered_map<std::string, std::string> platform_kvpairs;
    platform_kvpairs["classname"] = "func_platform";
    platform_kvpairs["origin"] = "0 0 0";
    platform_kvpairs["scale"] = "2.0";
    ED_Spawn(platform_kvpairs);

    Com_Printf("Entity spawning complete\n");
}

}  // namespace game
