// cvar.cpp -- console variable system
// A cvar is a named value (string, number, or flag) that can be changed at runtime
// and archived to config files. They drive gameplay tuning, graphics settings, and UI state.

#include "qcommon.h"
#include <unordered_map>
#include <algorithm>

namespace engine {

static std::unordered_map<std::string, cvar_t*> cvar_hash;
static std::vector<cvar_t*> cvar_list;

cvar_t* Cvar_Get(const std::string& name, const std::string& value, int flags, const std::string& description) {
    if (name.empty()) return nullptr;

    auto it = cvar_hash.find(name);
    if (it != cvar_hash.end()) {
        cvar_t* var = it->second;
        // Only override if explicitly setting with CVAR_FORCE
        if ((flags & CVAR_FORCE) && var->value != value) {
            Cvar_Set(name, value);
        }
        return var;
    }

    // Create a new cvar
    cvar_t* var = new cvar_t();
    var->name = name;
    var->value = value;
    var->default_value = value;
    var->flags = flags;
    var->description = description;
    
    // Parse numeric value if applicable
    var->float_value = std::stof(value);
    var->int_value = std::stoi(value);

    cvar_hash[name] = var;
    cvar_list.push_back(var);

    if (flags & CVAR_ARCHIVE) {
        // Marked for save to config file
    }

    return var;
}

void Cvar_Set(const std::string& name, const std::string& value) {
    auto it = cvar_hash.find(name);
    if (it == cvar_hash.end()) {
        // Create if doesn't exist
        Cvar_Get(name, value, 0, "");
        return;
    }

    cvar_t* var = it->second;
    if (var->value == value) return;  // No change

    var->value = value;

    // Parse numeric values
    try {
        var->float_value = std::stof(value);
        var->int_value = static_cast<int>(var->float_value);
    } catch (...) {
        var->float_value = 0.0f;
        var->int_value = 0;
    }

    // Call callback if registered
    if (var->callback) {
        var->callback(var);
    }
}

void Cvar_SetFloat(const std::string& name, float value) {
    Cvar_Set(name, std::to_string(value));
}

void Cvar_SetInt(const std::string& name, int value) {
    Cvar_Set(name, std::to_string(value));
}

cvar_t* Cvar_Find(const std::string& name) {
    auto it = cvar_hash.find(name);
    return it != cvar_hash.end() ? it->second : nullptr;
}

float Cvar_VariableValue(const std::string& name) {
    cvar_t* var = Cvar_Find(name);
    return var ? var->float_value : 0.0f;
}

std::string Cvar_VariableString(const std::string& name) {
    cvar_t* var = Cvar_Find(name);
    return var ? var->value : "";
}

int Cvar_VariableIntValue(const std::string& name) {
    cvar_t* var = Cvar_Find(name);
    return var ? var->int_value : 0;
}

std::vector<cvar_t*> Cvar_GetList() {
    return cvar_list;
}

std::string Cvar_WriteVariables() {
    std::string out;
    for (cvar_t* var : cvar_list) {
        if (var->flags & CVAR_ARCHIVE) {
            out += "set " + var->name + " \"" + var->value + "\"\n";
        }
    }
    return out;
}

void Cvar_LoadArchive(const std::string& script) {
    std::istringstream iss(script);
    std::string line;
    while (std::getline(iss, line)) {
        // Parse "set cvarname value" lines
        std::istringstream tokens(line);
        std::string cmd, name, value;
        tokens >> cmd >> name;
        if (cmd == "set" && !name.empty()) {
            std::getline(tokens, value);
            // Trim leading/trailing whitespace and quotes
            if (value.front() == ' ') value = value.substr(1);
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            Cvar_Set(name, value);
        }
    }
}

void Cvar_Shutdown() {
    for (auto& pair : cvar_hash) {
        delete pair.second;
    }
    cvar_hash.clear();
    cvar_list.clear();
}

}  // namespace engine
