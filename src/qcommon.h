// qcommon.h -- common definitions and declarations

#ifndef QCOMMON_H
#define QCOMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <glm/glm.hpp>

// Platform detection
#ifdef _WIN32
    #define PLATFORM_WINDOWS 1
#elif __APPLE__
    #define PLATFORM_MACOS 1
#elif __linux__
    #define PLATFORM_LINUX 1
#endif

// Basic types
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef int qboolean;

// Unified entity structure shared by the server, game, and editor modules.
// This is the single authoritative edict_t definition for the whole codebase
// (previously duplicated with incompatible layouts in qcommon.h, game/game.h,
// and server/sv_world.h).
struct edict_t {
    struct {
        int number;
        int modelindex;
    } s;

    uint32_t id = 0;
    bool inuse = false;
    uint32_t svflags = 0;
    uint32_t flags = 0;

    std::string classname;
    char *model = nullptr;
    char *target = nullptr;
    char *targetname = nullptr;
    char *killtarget = nullptr;

    // Transform
    glm::vec3 origin{0.0f};
    glm::vec3 angles{0.0f};
    glm::vec3 old_origin{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec3 avelocity{0.0f};
    glm::vec3 mins{0.0f};
    glm::vec3 maxs{0.0f};
    glm::vec3 absmin{0.0f};
    glm::vec3 absmax{0.0f};
    glm::vec3 size{0.0f};
    float scale = 1.0f;

    uint32_t solid = 0;
    uint32_t movetype = 0;
    uint32_t contents = 0;
    uint32_t clipmask = 0;
    uint32_t linkcount = 0;

    // Health / damage
    int health = 0;
    int maxhealth = 0;
    int gib_health = 0;
    int armor = 0;
    int mass = 0;
    float gravity = 1.0f;
    uint32_t takedamage = 0;
    int damage = 0;
    int teamnum = 0;

    // Client / lifecycle
    int client = -1;  // client slot index, -1 = not a client-controlled entity
    float freetime = 0.0f;
    float nextthink = 0.0f;
    float ltime = 0.0f;
    float lastactivetime = 0.0f;
    int index = 0;

    // Weapon / inventory (game module)
    int weapons[8] = {};
    int ammo[8] = {};
    int current_weapon = 0;
    int weapon_id = 0;
    int ammo_count = 0;
    float weapon_ready_time = 0.0f;
    float last_fire_time = 0.0f;
    float reload_end_time = 0.0f;

    // Links
    edict_t *owner = nullptr;
    edict_t *enemy = nullptr;
    edict_t *goalentity = nullptr;
    edict_t *groundentity = nullptr;

    // Callbacks
    void (*think)(edict_t *) = nullptr;
    void (*touch)(edict_t *, edict_t *) = nullptr;
    void (*use)(edict_t *, edict_t *, edict_t *) = nullptr;
    void (*pain)(edict_t *, edict_t *, int) = nullptr;
    void (*die)(edict_t *, edict_t *, edict_t *, int, int) = nullptr;
    void (*blocked)(edict_t *, edict_t *) = nullptr;

    void *game_data = nullptr;
    void *priv = nullptr;
};

// Console functions
void Com_Printf(const char* fmt, ...);
void Com_DPrintf(const char* fmt, ...);
void Com_Error(int level, const char* fmt, ...);
void Com_Warning(const char* fmt, ...);
unsigned int Com_Milliseconds();
void Com_Init();
void Com_Shutdown();

#endif // QCOMMON_H
