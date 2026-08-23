/*
 * =======================================================================
 *
 * Bone Yards - Rendering System
 *
 * Simple 3D rendering: boxes, lines, debug text
 *
 * =======================================================================
 */

#include "by_engine.h"
#include <stdio.h>
#include <stddef.h>

/* Simple vertex for debug geometry */
typedef struct {
	float x, y, z;
	float r, g, b;
} vertex_t;

/* Debug rendering state */
static struct {
	GLuint vao, vbo;
	GLuint shader_program;
	int vertex_count;
} render_state = {0};

/* ====================================================================== */
/* SHADER COMPILATION */
/* ====================================================================== */

static const char *vertex_shader_src = 
"#version 150\n"
"in vec3 position;\n"
"in vec3 color;\n"
"out vec3 vertexColor;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"void main() {\n"
"  gl_Position = projection * view * model * vec4(position, 1.0);\n"
"  vertexColor = color;\n"
"}\n";

static const char *fragment_shader_src =
"#version 150\n"
"in vec3 vertexColor;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"  FragColor = vec4(vertexColor, 1.0);\n"
"}\n";

static GLuint
BY_Render_CompileShader(const char *src, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	/* Check for errors */
	int success;
	char info_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		fprintf(stderr, "Shader compilation failed: %s\n", info_log);
	}

	return shader;
}

static GLuint
BY_Render_CreateProgram(void)
{
	GLuint vs = BY_Render_CompileShader(vertex_shader_src, GL_VERTEX_SHADER);
	GLuint fs = BY_Render_CompileShader(fragment_shader_src, GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	/* Check for link errors */
	int success;
	char info_log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, info_log);
		fprintf(stderr, "Shader linking failed: %s\n", info_log);
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

/* ====================================================================== */
/* RENDERING INITIALIZATION */
/* ====================================================================== */

void
BY_Render_Init(void)
{
	/* Create shader program */
	render_state.shader_program = BY_Render_CreateProgram();

	/* Create VAO/VBO for debug geometry */
	glGenVertexArrays(1, &render_state.vao);
	glGenBuffers(1, &render_state.vbo);

	glBindVertexArray(render_state.vao);
	glBindBuffer(GL_ARRAY_BUFFER, render_state.vbo);

	/* Allocate buffer (large enough for debug geometry) */
	glBufferData(GL_ARRAY_BUFFER, 65536 * sizeof(vertex_t), NULL, GL_DYNAMIC_DRAW);

	/* Setup vertex attributes */
	GLint pos_attrib = glGetAttribLocation(render_state.shader_program, "position");
	GLint color_attrib = glGetAttribLocation(render_state.shader_program, "color");

	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE,
		sizeof(vertex_t), (void *)offsetof(vertex_t, x));

	glEnableVertexAttribArray(color_attrib);
	glVertexAttribPointer(color_attrib, 3, GL_FLOAT, GL_FALSE,
		sizeof(vertex_t), (void *)offsetof(vertex_t, r));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stderr, "Rendering system initialized\n");
}

void
BY_Render_Shutdown(void)
{
	if (render_state.vao)
		glDeleteVertexArrays(1, &render_state.vao);
	if (render_state.vbo)
		glDeleteBuffers(1, &render_state.vbo);
	if (render_state.shader_program)
		glDeleteProgram(render_state.shader_program);
}

/* ====================================================================== */
/* SIMPLE GEOMETRY RENDERING */
/* ====================================================================== */

void
BY_Render_DrawBox(vec3_t pos, float size, float r, float g, float b)
{
	/* Create a simple box mesh */
	float h = size / 2.0f;

	vertex_t vertices[] = {
		/* Front face */
		{pos[0] - h, pos[1] - h, pos[2] + h, r, g, b},
		{pos[0] + h, pos[1] - h, pos[2] + h, r, g, b},
		{pos[0] + h, pos[1] + h, pos[2] + h, r, g, b},
		{pos[0] - h, pos[1] + h, pos[2] + h, r, g, b},
	};

	glBindVertexArray(render_state.vao);
	glBindBuffer(GL_ARRAY_BUFFER, render_state.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glUseProgram(render_state.shader_program);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void
BY_Render_DrawLine(vec3_t start, vec3_t end, float r, float g, float b)
{
	vertex_t vertices[] = {
		{start[0], start[1], start[2], r, g, b},
		{end[0], end[1], end[2], r, g, b},
	};

	glBindVertexArray(render_state.vao);
	glBindBuffer(GL_ARRAY_BUFFER, render_state.vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glUseProgram(render_state.shader_program);
	glDrawArrays(GL_LINES, 0, 2);
}

void
BY_Render_Flush(void)
{
	glBindVertexArray(0);
	glUseProgram(0);
}
