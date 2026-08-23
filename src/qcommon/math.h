#pragma once

#include <math.h>
#include <stdint.h>

/* Vector and matrix math */
typedef float vec3_t[3];
typedef float vec4_t[4];
typedef float mat4_t[16];

void VectorCopy(const vec3_t in, vec3_t out);
void VectorClear(vec3_t v);
void VectorAdd(const vec3_t a, const vec3_t b, vec3_t out);
void VectorSubtract(const vec3_t a, const vec3_t b, vec3_t out);
void VectorScale(const vec3_t v, float s, vec3_t out);
float VectorLength(const vec3_t v);
float VectorDistance(const vec3_t a, const vec3_t b);
void VectorNormalize(vec3_t v);
float DotProduct(const vec3_t a, const vec3_t b);
void CrossProduct(const vec3_t a, const vec3_t b, vec3_t out);

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void VectorToAngles(const vec3_t v, vec3_t angles);

float LerpF(float a, float b, float t);
void VectorLerp(const vec3_t a, const vec3_t b, float t, vec3_t out);

float ClampF(float v, float min, float max);
int ClampI(int v, int min, int max);

float RandomF(void);
float CRandomF(void);  /* -1 to 1 */
int RandomI(int max);

#define DEG2RAD(x) ((x) * (3.14159265359f / 180.0f))
#define RAD2DEG(x) ((x) * (180.0f / 3.14159265359f))
