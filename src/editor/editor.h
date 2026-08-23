#pragma once

#include "../server/sv_world.h"
#include <stdint.h>
#include <stdbool.h>

/* Editor mode state */
typedef struct {
    bool active;
    bool paused;
    
    vec3_t camera_origin;
    vec3_t camera_angles;
    
    edict_t **selection;
    int num_selected;
    
    uint32_t grid_size;
    bool snap_enabled;
} editor_t;

/* Editor functions */
bool ED_Init(void);
void ED_Shutdown(void);
void ED_Frame(float dt);

void ED_SelectEntity(edict_t *ent);
void ED_DeselectAll(void);
void ED_SpawnEntity(const char *classname, const vec3_t origin);
void ED_DeleteSelection(void);

bool ED_SaveLevel(const char *filename);
bool ED_LoadLevel(const char *filename);
