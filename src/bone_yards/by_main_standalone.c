/*
 * =======================================================================
 *
 * Bone Yards - Standalone Main
 *
 * Entry point for standalone executable
 *
 * =======================================================================
 */

#include "by_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Game state */
static struct {
	vec3_t player_pos;
	float player_pitch;
	float player_yaw;
	float player_health;
	int player_ammo;
	float fire_cooldown;
} game_state = {
	.player_pos = {0.0f, 50.0f, 0.0f},
	.player_pitch = 0.0f,
	.player_yaw = 0.0f,
	.player_health = 100.0f,
	.player_ammo = 30,
	.fire_cooldown = 0.0f,
};

/* Test entities */
#define MAX_TEST_ENTITIES 32
static struct {
	vec3_t pos;
	float size;
	float health;
	const char *type;
} test_entities[MAX_TEST_ENTITIES] = {
	{{100.0f, 50.0f, -200.0f}, 8.0f, 40.0f, "mechalichen"},
	{{-100.0f, 50.0f, -200.0f}, 6.0f, 25.0f, "slug"},
	{{0.0f, 50.0f, -400.0f}, 8.0f, 40.0f, "mechalichen"},
};
static int num_entities = 3;

/* ====================================================================== */
/* GAME LOGIC */
/* ====================================================================== */

static void
Game_Update(float dt)
{
	float mouse_dx, mouse_dy;
	float move_x = 0.0f, move_y = 0.0f, move_z = 0.0f;
	float move_speed = 100.0f;  /* Units per second */

	/* Mouse look */
	BY_Engine_GetMouseDelta(&mouse_dx, &mouse_dy);
	game_state.player_yaw += mouse_dx * 0.05f;
	game_state.player_pitch -= mouse_dy * 0.05f;

	/* Clamp pitch */
	if (game_state.player_pitch > 89.0f)
		game_state.player_pitch = 89.0f;
	if (game_state.player_pitch < -89.0f)
		game_state.player_pitch = -89.0f;

	/* WASD movement */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_W))
		move_z -= move_speed * dt;  /* Forward */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_S))
		move_z += move_speed * dt;  /* Backward */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_A))
		move_x -= move_speed * dt;  /* Strafe left */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_D))
		move_x += move_speed * dt;  /* Strafe right */

	/* Apply movement relative to player rotation */
	float yaw_rad = game_state.player_yaw * 3.14159f / 180.0f;
	float cos_yaw = cosf(yaw_rad);
	float sin_yaw = sinf(yaw_rad);

	game_state.player_pos.x += (move_x * cos_yaw - move_z * sin_yaw);
	game_state.player_pos.z += (move_x * sin_yaw + move_z * cos_yaw);

	/* Space to jump */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_SPACE))
		game_state.player_pos.y -= 50.0f * dt;

	/* Ctrl to crouch */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		game_state.player_pos.y += 50.0f * dt;

	/* Update cooldowns */
	if (game_state.fire_cooldown > 0.0f)
		game_state.fire_cooldown -= dt;

	/* Left click to fire */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_LEFT) && game_state.fire_cooldown <= 0.0f)
	{
		if (game_state.player_ammo > 0)
		{
			game_state.player_ammo--;
			game_state.fire_cooldown = 0.1f;  /* 100ms between shots */
		}
	}

	/* Right click to slash */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_RIGHT))
	{
		/* Check for nearby entities */
		for (int i = 0; i < num_entities; i++)
		{
			float dx = test_entities[i].pos.x - game_state.player_pos.x;
			float dy = test_entities[i].pos.y - game_state.player_pos.y;
			float dz = test_entities[i].pos.z - game_state.player_pos.z;
			float dist = sqrtf(dx*dx + dy*dy + dz*dz);

			if (dist < 100.0f)  /* Melee range */
			{
				test_entities[i].health -= 20.0f;
				if (test_entities[i].health <= 0.0f)
				{
					/* Remove entity */
					for (int j = i; j < num_entities - 1; j++)
						test_entities[j] = test_entities[j + 1];
					num_entities--;
				}
			}
		}
	}

	/* R to reload */
	if (BY_Engine_IsKeyPressed(GLFW_KEY_R))
		game_state.player_ammo = 30;
}

static void
Game_Render(void)
{
	vec3_t zero = {0.0f, 0.0f, 0.0f};
	vec3_t forward;
	float yaw_rad = game_state.player_yaw * 3.14159f / 180.0f;

	/* Calculate forward direction */
	forward.x = sinf(yaw_rad);
	forward.y = 0.0f;
	forward.z = -cosf(yaw_rad);

	/* Draw floor grid */
	for (int x = -500; x <= 500; x += 100)
	{
		vec3_t start = {(float)x, 0.0f, -500.0f};
		vec3_t end = {(float)x, 0.0f, 500.0f};
		BY_Engine_DrawLine(start, end, 0.3f, 0.3f, 0.3f);
	}
	for (int z = -500; z <= 500; z += 100)
	{
		vec3_t start = {-500.0f, 0.0f, (float)z};
		vec3_t end = {500.0f, 0.0f, (float)z};
		BY_Engine_DrawLine(start, end, 0.3f, 0.3f, 0.3f);
	}

	/* Draw test entities */
	for (int i = 0; i < num_entities; i++)
	{
		float r = 0.0f, g = 1.0f, b = 0.0f;  /* Green for alive */
		if (test_entities[i].health < 20.0f)
			r = 1.0f, g = 0.0f;  /* Red for low health */

		BY_Engine_DrawBox(test_entities[i].pos, test_entities[i].size, r, g, b);
	}

	/* Draw crosshair (center of screen) */
	vec3_t crosshair_center = {
		game_state.player_pos.x + forward.x * 100.0f,
		game_state.player_pos.y,
		game_state.player_pos.z + forward.z * 100.0f
	};

	vec3_t cross_h1 = {crosshair_center.x - 10.0f, crosshair_center.y, crosshair_center.z};
	vec3_t cross_h2 = {crosshair_center.x + 10.0f, crosshair_center.y, crosshair_center.z};
	vec3_t cross_v1 = {crosshair_center.x, crosshair_center.y - 10.0f, crosshair_center.z};
	vec3_t cross_v2 = {crosshair_center.x, crosshair_center.y + 10.0f, crosshair_center.z};

	BY_Engine_DrawLine(cross_h1, cross_h2, 1.0f, 1.0f, 0.0f);
	BY_Engine_DrawLine(cross_v1, cross_v2, 1.0f, 1.0f, 0.0f);
}

/* ====================================================================== */
/* MAIN */
/* ====================================================================== */

int
main(int argc, char *argv[])
{
	fprintf(stderr, "Bone Yards - Standalone Testable Build\n");
	fprintf(stderr, "======================================\n\n");

	/* Initialize engine */
	if (!BY_Engine_Init(1280, 720, "Bone Yards - Test Build"))
	{
		fprintf(stderr, "Failed to initialize engine\n");
		return 1;
	}

	fprintf(stderr, "\nControls:\n");
	fprintf(stderr, "  WASD  - Move\n");
	fprintf(stderr, "  Mouse - Look around\n");
	fprintf(stderr, "  Space - Jump (up)\n");
	fprintf(stderr, "  Ctrl  - Crouch (down)\n");
	fprintf(stderr, "  Left Click / <- - Fire gun\n");
	fprintf(stderr, "  Right Click / -> - Swing sword\n");
	fprintf(stderr, "  R - Reload\n");
	fprintf(stderr, "  ESC - Quit\n\n");

	/* Game loop */
	double last_time = 0.0;
	while (BY_Engine_IsRunning())
	{
		double current_time = glfwGetTime();
		float dt = (float)(current_time - last_time);
		last_time = current_time;

		if (dt > 0.016f)  /* Cap to ~60 FPS for stability */
			dt = 0.016f;

		/* Update and render */
		BY_Engine_BeginFrame();
		BY_Engine_UpdateInput();

		Game_Update(dt);
		Game_Render();

		BY_Engine_EndFrame();
	}

	BY_Engine_Shutdown();

	fprintf(stderr, "\nShutdown complete.\n");
	return 0;
}
