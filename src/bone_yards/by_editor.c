/*
 * =======================================================================
 *
 * Bone Yards - In-Game Editor
 *
 * =======================================================================
 */

#include "by_local.h"

static editor_state_t editor_state;

void
BY_EditorInit(void)
{
	memset(&editor_state, 0, sizeof(editor_state_t));
	editor_state.editor_active = false;
	editor_state.selected_entity_id = -1;
	editor_state.editor_mode = 0;
}

void
BY_EditorDraw(void)
{
	if (!editor_state.editor_active)
		return;

	gi.dprintf("[EDITOR] Active entity: %d\n", editor_state.selected_entity_id);
}

void
BY_EditorUpdate(edict_t *player, usercmd_t *cmd)
{
}

void
BY_EditorSpawnEntity(const char *classname, vec3_t origin)
{
	edict_t *ent;

	if (!classname)
		return;

	ent = G_Spawn();
	if (!ent)
	{
		gi.dprintf("Failed to spawn entity: %s\n", classname);
		return;
	}

	ent->classname = classname;
	VectorCopy(origin, ent->s.origin);
	VectorCopy(origin, ent->s.old_origin);

	gi.dprintf("Spawned entity: %s at (%.1f, %.1f, %.1f)\n",
		classname, origin[0], origin[1], origin[2]);

	editor_state.selected_entity_id = ent - g_edicts;
}

void
BY_EditorDeleteEntity(int entity_id)
{
	edict_t *ent;

	if (entity_id < 0 || entity_id >= globals.num_edicts)
		return;

	ent = &g_edicts[entity_id];
	if (!ent->inuse)
		return;

	gi.dprintf("Deleted entity: %s (ID: %d)\n", ent->classname, entity_id);
	G_FreeEdict(ent);

	if (editor_state.selected_entity_id == entity_id)
		editor_state.selected_entity_id = -1;
}

void
BY_EditorSetProperty(int entity_id, const char *key, const char *value)
{
	edict_t *ent;

	if (entity_id < 0 || entity_id >= globals.num_edicts)
		return;

	ent = &g_edicts[entity_id];
	if (!ent->inuse)
		return;

	gi.dprintf("Set property: %s.%s = %s\n", ent->classname, key, value);
}

void
BY_EditorSaveLevel(const char *filename)
{
	gi.dprintf("Saving level to: %s\n", filename);
}

void
BY_EditorLoadLevel(const char *filename)
{
	gi.dprintf("Loading level from: %s\n", filename);
}
