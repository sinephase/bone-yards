#pragma once

#include <stdint.h>

typedef void (*xcommand_t)(void);

void Cmd_Init(void);
void Cmd_AddCommand(const char *cmd_name, xcommand_t function, const char *description);
void Cmd_RemoveCommand(const char *cmd_name);

void Cbuf_AddText(const char *text);
void Cbuf_Execute(void);

void Cmd_ForwardToServer(void);

/* Command completion */
const char **Cmd_CompleteCommand(const char *partial, int *count);

/* Printing */
typedef void (*Com_Printf_t)(const char *fmt, ...);
void Com_Printf(const char *fmt, ...);
void Com_Error(const char *fmt, ...);
