// editor.cpp -- level editor mode stub implementation
// Provides a minimal, functional implementation of the editor.h interface so
// that the module can be linked into the build. Level (de)serialization is
// left as a TODO for a future editor milestone.

#include "editor.h"
#include "../qcommon.h"
#include <vector>
#include <cstdio>

namespace {

editor_t g_editor;
std::vector<edict_t *> g_selection;

}  // namespace

bool ED_Init(void) {
    g_editor.active = true;
    g_editor.paused = false;

    g_editor.camera_origin[0] = g_editor.camera_origin[1] = g_editor.camera_origin[2] = 0.0f;
    g_editor.camera_angles[0] = g_editor.camera_angles[1] = g_editor.camera_angles[2] = 0.0f;

    g_selection.clear();
    g_editor.selection = nullptr;
    g_editor.num_selected = 0;

    g_editor.grid_size = 64;
    g_editor.snap_enabled = true;

    Com_Printf("Editor initialized\n");
    return true;
}

void ED_Shutdown(void) {
    Com_Printf("Editor shutdown\n");
    g_selection.clear();
    g_editor.selection = nullptr;
    g_editor.num_selected = 0;
    g_editor.active = false;
}

void ED_Frame(float dt) {
    if (!g_editor.active || g_editor.paused) return;
    (void)dt;
    // TODO: camera movement, gizmo updates, picking
}

void ED_SelectEntity(edict_t *ent) {
    if (!ent) return;

    for (edict_t *existing : g_selection) {
        if (existing == ent) return;  // Already selected
    }

    g_selection.push_back(ent);
    g_editor.selection = g_selection.data();
    g_editor.num_selected = (int)g_selection.size();
}

void ED_DeselectAll(void) {
    g_selection.clear();
    g_editor.selection = nullptr;
    g_editor.num_selected = 0;
}

void ED_SpawnEntity(const char *classname, const vec3_t origin) {
    (void)origin;
    // TODO: hook into the game module's entity spawn system
    Com_Printf("ED_SpawnEntity: %s\n", classname ? classname : "(null)");
}

void ED_DeleteSelection(void) {
    Com_Printf("ED_DeleteSelection: removing %d entities\n", g_editor.num_selected);
    ED_DeselectAll();
}

bool ED_SaveLevel(const char *filename) {
    Com_Printf("ED_SaveLevel: %s (not yet implemented)\n", filename ? filename : "(null)");
    return false;
}

bool ED_LoadLevel(const char *filename) {
    Com_Printf("ED_LoadLevel: %s (not yet implemented)\n", filename ? filename : "(null)");
    return false;
}
