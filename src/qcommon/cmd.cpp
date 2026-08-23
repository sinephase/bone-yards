// cmd.cpp -- command buffer, tokenizer, and console command system
// Commands can be bound to keys, executed from the console, or fired by the game.
// The buffer accumulates input and executes commands in order each frame.

#include "qcommon.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace engine {

struct cmd_function_t {
    std::string name;
    cmd_callback_t callback;
    std::string description;
};

static std::vector<std::string> cmd_buffer;
static std::unordered_map<std::string, cmd_function_t> cmd_functions;
static std::vector<std::string> print_buffer;
static print_sink_t g_print_sink = nullptr;

void Cmd_Init() {
    cmd_buffer.clear();
    cmd_functions.clear();
    print_buffer.clear();
}

void Cmd_AddCommand(const std::string& name, cmd_callback_t callback, const std::string& description) {
    if (name.empty() || !callback) return;

    // Don't allow duplicate registrations
    if (cmd_functions.find(name) != cmd_functions.end()) {
        return;
    }

    cmd_function_t fn;
    fn.name = name;
    fn.callback = callback;
    fn.description = description;
    cmd_functions[name] = fn;
}

void Cmd_RemoveCommand(const std::string& name) {
    auto it = cmd_functions.find(name);
    if (it != cmd_functions.end()) {
        cmd_functions.erase(it);
    }
}

// Tokenize a command line into words, respecting quotes
std::vector<std::string> Cmd_TokenizeString(const std::string& str) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;

    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];

        if (c == '"') {
            in_quotes = !in_quotes;
        } else if ((c == ' ' || c == '\t' || c == '\n') && !in_quotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

void Cbuf_AddText(const std::string& text) {
    if (text.empty()) return;
    cmd_buffer.push_back(text);
}

void Cbuf_Execute() {
    while (!cmd_buffer.empty()) {
        std::string line = cmd_buffer.front();
        cmd_buffer.erase(cmd_buffer.begin());

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || (line.length() > 1 && line[0] == '/' && line[1] == '/')) {
            continue;
        }

        // Tokenize the line
        std::vector<std::string> argv = Cmd_TokenizeString(line);
        if (argv.empty()) continue;

        std::string cmd = argv[0];
        
        // Look up and execute the command
        auto it = cmd_functions.find(cmd);
        if (it != cmd_functions.end()) {
            it->second.callback(argv);
        } else {
            Com_Printf("Unknown command: " + cmd + "\n");
        }
    }
}

std::vector<std::string> Cmd_CompleteString(const std::string& partial) {
    std::vector<std::string> matches;
    std::string lower_partial = partial;
    std::transform(lower_partial.begin(), lower_partial.end(), lower_partial.begin(), ::tolower);

    for (const auto& pair : cmd_functions) {
        std::string lower_name = pair.first;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        if (lower_name.find(lower_partial) == 0) {
            matches.push_back(pair.first);
        }
    }

    std::sort(matches.begin(), matches.end());
    return matches;
}

std::vector<cmd_function_t> Cmd_GetList() {
    std::vector<cmd_function_t> list;
    for (const auto& pair : cmd_functions) {
        list.push_back(pair.second);
    }
    return list;
}

void Com_SetPrintSink(print_sink_t sink) {
    g_print_sink = sink;
}

void Com_Printf(const std::string& msg) {
    print_buffer.push_back(msg);
    
    if (g_print_sink) {
        g_print_sink(msg);
    }

    // Also log to stderr for debugging
    std::cerr << msg;
}

void Com_DPrintf(const std::string& msg) {
    // Only print in debug builds
#ifdef _DEBUG
    Com_Printf(msg);
#endif
}

std::vector<std::string> Com_GetPrintBuffer() {
    std::vector<std::string> buffer = print_buffer;
    print_buffer.clear();
    return buffer;
}

void Cmd_Shutdown() {
    cmd_buffer.clear();
    cmd_functions.clear();
    print_buffer.clear();
}

}  // namespace engine
