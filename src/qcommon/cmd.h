#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace engine {

typedef void (*cmd_callback_t)(const std::vector<std::string>& args);
typedef void (*print_sink_t)(const std::string& msg);

struct cmd_function_t {
    std::string name;
    cmd_callback_t callback;
    std::string description;
};

void Cmd_Init(void);
void Cmd_AddCommand(const std::string& name, cmd_callback_t callback, const std::string& description);
void Cmd_RemoveCommand(const std::string& name);
void Cmd_Shutdown(void);

std::vector<std::string> Cmd_TokenizeString(const std::string& str);
std::vector<std::string> Cmd_CompleteString(const std::string& partial);
std::vector<cmd_function_t> Cmd_GetList(void);

void Cbuf_AddText(const std::string& text);
void Cbuf_Execute(void);

/* Printing */
void Com_SetPrintSink(print_sink_t sink);
void Com_Printf(const std::string& msg);
void Com_DPrintf(const std::string& msg);
std::vector<std::string> Com_GetPrintBuffer(void);

}  // namespace engine
