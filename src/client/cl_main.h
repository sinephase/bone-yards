#pragma once

#include "../qcommon/math.h"
#include <stdint.h>

/* Client state */
typedef struct {
    vec3_t origin;
    vec3_t velocity;
    vec3_t v_angle;
    
    int health;
    int armor;
    int ammo;
    
    uint32_t weapon;
    uint32_t buttons;
    
    void *game_data;
} client_t;

/* Input */
void CL_InitInput(void);
void CL_ProcessInput(void);

/* Network */
void CL_Connect(const char *server);
void CL_Disconnect(void);

/* Game commands */
void CL_Say(const char *text);
void CL_Fire(void);
