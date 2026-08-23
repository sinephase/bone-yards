/*
 * =======================================================================
 *
 * Bone Yards - Minimal Game Engine Implementation
 *
 * OpenGL 3.2 rendering, GLFW window management, basic game loop
 *
 * =======================================================================
 */

#include "by_engine.h"

engine_context_t engine = {0};
static qboolean engine_running = false;
static double last_frame_time = 0.0;

/* Input state */
static struct {
	qboolean keys[512];
	float mouse_x, mouse_y;
	float mouse_dx, mouse_dy;
} input_state = {0};

/* Shader program */
static GLuint shader_program = 0;

/* ====================================================================== */
/* WINDOW & CONTEXT MANAGEMENT */
/* ====================================================================== */

static void
GLFW_ErrorCallback(int error, const char *description)
{
	fprintf(stderr, "GLFW Error [%d]: %s\n", error, description);
}

static void
GLFW_KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key < 512)
	{
		if (action == GLFW_PRESS)
			input_state.keys[key] = true;
		else if (action == GLFW_RELEASE)
			input_state.keys[key] = false;
	}

	/* ESC to quit */
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		engine_running = false;
}

static void
GLFW_MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
	input_state.mouse_dx = (float)(xpos - input_state.mouse_x);
	input_state.mouse_dy = (float)(ypos - input_state.mouse_y);
	input_state.mouse_x = (float)xpos;
	input_state.mouse_y = (float)ypos;
}

static void
GLFW_FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	engine.width = width;
	engine.height = height;
	glViewport(0, 0, width, height);
}

qboolean
BY_Engine_Init(int width, int height, const char *title)
{
	GLFWmonitor *monitor;
	const GLFWvidmode *mode;

	fprintf(stderr, "Initializing Bone Yards Engine...\n");

	/* Initialize GLFW */
	glfwSetErrorCallback(GLFW_ErrorCallback);

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return false;
	}

	/* Request OpenGL 3.2 core profile */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	/* Create window */
	engine.window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!engine.window)
	{
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();
		return false;
	}

	engine.width = width;
	engine.height = height;
	engine.fov = 75.0f;
	engine.near_plane = 0.1f;
	engine.far_plane = 4096.0f;

	/* Make context current */
	glfwMakeContextCurrent(engine.window);
	glfwSetInputMode(engine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSwapInterval(1);  /* Vsync */

	/* Initialize GLEW */
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return false;
	}

	/* Setup callbacks */
	glfwSetKeyCallback(engine.window, GLFW_KeyCallback);
	glfwSetCursorPosCallback(engine.window, GLFW_MouseCallback);
	glfwSetFramebufferSizeCallback(engine.window, GLFW_FramebufferSizeCallback);

	/* OpenGL setup */
	glViewport(0, 0, width, height);
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	engine_running = true;

	fprintf(stderr, "Engine initialized successfully\n");
	return true;
}

void
BY_Engine_Shutdown(void)
{
	if (engine.window)
	{
		glfwDestroyWindow(engine.window);
	}
	glfwTerminate();
	fprintf(stderr, "Engine shutdown complete\n");
}

qboolean
BY_Engine_IsRunning(void)
{
	return engine_running && !glfwWindowShouldClose(engine.window);
}

void
BY_Engine_Run(void)
{
	while (BY_Engine_IsRunning())
	{
		BY_Engine_BeginFrame();
		BY_Engine_EndFrame();
	}
}

/* ====================================================================== */
/* FRAME MANAGEMENT */
/* ====================================================================== */

void
BY_Engine_BeginFrame(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
BY_Engine_EndFrame(void)
{
	glfwSwapBuffers(engine.window);
	glfwPollEvents();
}

/* ====================================================================== */
/* INPUT HANDLING */
/* ====================================================================== */

void
BY_Engine_UpdateInput(void)
{
	/* Reset frame mouse delta */
	input_state.mouse_dx = 0.0f;
	input_state.mouse_dy = 0.0f;
}

void
BY_Engine_GetMouseDelta(float *dx, float *dy)
{
	if (dx) *dx = input_state.mouse_dx;
	if (dy) *dy = input_state.mouse_dy;
}

qboolean
BY_Engine_IsKeyPressed(int key)
{
	if (key < 0 || key >= 512)
		return false;
	return input_state.keys[key];
}

/* ====================================================================== */
/* VIEWPORT & CAMERA */
/* ====================================================================== */

void
BY_Engine_SetViewport(float x, float y, float z, float pitch, float yaw)
{
	/* TODO: Set camera projection and view matrices */
	/* This will be used to set up the view for rendering */
}

/* ====================================================================== */
/* DEBUG RENDERING */
/* ====================================================================== */

void
BY_Engine_DrawBox(vec3_t pos, float size, float r, float g, float b)
{
	/* TODO: Immediate mode box rendering */
	/* For now, just a placeholder */
}

void
BY_Engine_DrawLine(vec3_t start, vec3_t end, float r, float g, float b)
{
	/* TODO: Immediate mode line rendering */
}

void
BY_Engine_DrawText(vec3_t pos, const char *text, float scale)
{
	/* TODO: 3D text rendering */
}

/* ====================================================================== */
/* MATH HELPERS */
/* ====================================================================== */

mat4_t
BY_Math_Identity(void)
{
	mat4_t m = {0};
	m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
	return m;
}

mat4_t
BY_Math_Perspective(float fov, float aspect, float near, float far)
{
	mat4_t m = {0};
	float f = 1.0f / tanf(fov * 0.5f * 3.14159f / 180.0f);
	
	m.m[0] = f / aspect;
	m.m[5] = f;
	m.m[10] = (far + near) / (near - far);
	m.m[11] = -1.0f;
	m.m[14] = (2.0f * far * near) / (near - far);
	
	return m;
}

mat4_t
BY_Math_LookAt(vec3_t eye, vec3_t center, vec3_t up)
{
	/* TODO: Implement LookAt matrix */
	return BY_Math_Identity();
}

void
BY_Math_VectorAdd(vec3_t out, vec3_t a, vec3_t b)
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

void
BY_Math_VectorScale(vec3_t out, vec3_t v, float s)
{
	out[0] = v[0] * s;
	out[1] = v[1] * s;
	out[2] = v[2] * s;
}

float
BY_Math_VectorLength(vec3_t v)
{
	return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
