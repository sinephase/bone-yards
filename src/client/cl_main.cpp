// cl_main.cpp -- client-side stub implementation
// Provides a minimal, functional implementation of the cl_main.h interface so
// that the module can be linked into the build. Networking and rendering
// hookups are left as TODOs for future client milestones.

#include "cl_main.h"
#include "../qcommon.h"
#include <cstdio>
#include <cstring>

namespace {

client_t g_client;
bool g_connected = false;

}  // namespace

void CL_InitInput(void) {
    g_client.origin[0] = g_client.origin[1] = g_client.origin[2] = 0.0f;
    g_client.velocity[0] = g_client.velocity[1] = g_client.velocity[2] = 0.0f;
    g_client.v_angle[0] = g_client.v_angle[1] = g_client.v_angle[2] = 0.0f;

    g_client.health = 100;
    g_client.armor = 0;
    g_client.ammo = 0;

    g_client.weapon = 0;
    g_client.buttons = 0;
    g_client.game_data = nullptr;

    Com_Printf("Client input initialized\n");
}

void CL_ProcessInput(void) {
    // TODO: poll platform input devices and update g_client.buttons/v_angle
}

void CL_Connect(const char *server) {
    Com_Printf("CL_Connect: %s (not yet implemented)\n", server ? server : "(null)");
    g_connected = false;
}

void CL_Disconnect(void) {
    if (g_connected) {
        Com_Printf("CL_Disconnect\n");
    }
    g_connected = false;
}

void CL_Say(const char *text) {
    Com_Printf("say: %s\n", text ? text : "");
}

void CL_Fire(void) {
    g_client.buttons |= 1;  // BUTTON_ATTACK
    // TODO: forward fire event to the server/game module
}
