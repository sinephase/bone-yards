#include "math.h"
#include <cmath>
#include <algorithm>

namespace qcommon {

constexpr float DEG_TO_RAD = 3.14159265359f / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / 3.14159265359f;

struct vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct mat3 {
    float m[3][3] = {};

    static mat3 identity();
    static mat3 translation(const vec3& t);
    static mat3 rotationZ(float radians);
    static mat3 scale(const vec3& s);
};

struct angleVectors_t {
    vec3 forward;
    vec3 right;
    vec3 up;
};

// ============================================================================
// Vector operations
// ============================================================================

vec3 operator+(const vec3& a, const vec3& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 operator-(const vec3& a, const vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3 operator*(const vec3& v, float scale) {
    return {v.x * scale, v.y * scale, v.z * scale};
}

vec3 operator*(float scale, const vec3& v) {
    return {v.x * scale, v.y * scale, v.z * scale};
}

vec3 operator/(const vec3& v, float scale) {
    float inv = 1.0f / scale;
    return {v.x * inv, v.y * inv, v.z * inv};
}

float dot(const vec3& a, const vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 cross(const vec3& a, const vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float length(const vec3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float distanceSquared(const vec3& a, const vec3& b) {
    vec3 d = a - b;
    return d.x * d.x + d.y * d.y + d.z * d.z;
}

float distance(const vec3& a, const vec3& b) {
    return std::sqrt(distanceSquared(a, b));
}

vec3 normalize(const vec3& v) {
    float len = length(v);
    if (len < 1e-6f) return {0, 0, 0};
    return v / len;
}

vec3 clamp(const vec3& v, float min_val, float max_val) {
    return {
        std::clamp(v.x, min_val, max_val),
        std::clamp(v.y, min_val, max_val),
        std::clamp(v.z, min_val, max_val)
    };
}

// ============================================================================
// Matrix operations
// ============================================================================

mat3 mat3::identity() {
    return {{
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    }};
}

mat3 mat3::translation(const vec3& t) {
    return {{
        {1, 0, t.x},
        {0, 1, t.y},
        {0, 0, 1}
    }};
}

mat3 mat3::rotationZ(float radians) {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return {{
        {c, -s, 0},
        {s, c, 0},
        {0, 0, 1}
    }};
}

mat3 mat3::scale(const vec3& s) {
    return {{
        {s.x, 0, 0},
        {0, s.y, 0},
        {0, 0, s.z}
    }};
}

mat3 operator*(const mat3& a, const mat3& b) {
    mat3 result = {};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return result;
}

vec3 operator*(const mat3& m, const vec3& v) {
    return {
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z
    };
}

// ============================================================================
// Angle conversions
// ============================================================================

vec3 angleVectors(const vec3& angles) {
    float pitch = angles.x * DEG_TO_RAD;
    float yaw = angles.y * DEG_TO_RAD;
    
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);
    
    return {
        cp * cy,
        cp * sy,
        -sp
    };
}

angleVectors_t angleVectorsFull(const vec3& angles) {
    float pitch = angles.x * DEG_TO_RAD;
    float yaw = angles.y * DEG_TO_RAD;
    float roll = angles.z * DEG_TO_RAD;
    
    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);
    float cr = std::cos(roll);
    float sr = std::sin(roll);
    
    return {
        .forward = {cp * cy, cp * sy, -sp},
        .right = {-sy * cr + sp * cy * sr, cy * cr + sp * sy * sr, cp * sr},
        .up = {-sy * sr - sp * cy * cr, cy * sr - sp * sy * cr, cp * cr}
    };
}

vec3 vectorToAngles(const vec3& v) {
    vec3 normalized = normalize(v);
    float pitch = std::asin(-normalized.z) * RAD_TO_DEG;
    float yaw = std::atan2(normalized.y, normalized.x) * RAD_TO_DEG;
    return {pitch, yaw, 0};
}

// ============================================================================
// Scalar utilities
// ============================================================================

float clamp(float value, float min_val, float max_val) {
    return std::clamp(value, min_val, max_val);
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float crandom() {
    return (std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

float random() {
    return std::rand() / (float)RAND_MAX;
}

int randomInt(int max) {
    return std::rand() % max;
}

// ============================================================================
// Bounding box utilities
// ============================================================================

bool boxesIntersect(const vec3& min1, const vec3& max1, const vec3& min2, const vec3& max2) {
    return !(max1.x < min2.x || min1.x > max2.x ||
             max1.y < min2.y || min1.y > max2.y ||
             max1.z < min2.z || min1.z > max2.z);
}

vec3 boxCenter(const vec3& mins, const vec3& maxs) {
    return {
        (mins.x + maxs.x) * 0.5f,
        (mins.y + maxs.y) * 0.5f,
        (mins.z + maxs.z) * 0.5f
    };
}

vec3 boxSize(const vec3& mins, const vec3& maxs) {
    return maxs - mins;
}

} // namespace qcommon

// ============================================================================
// Global vec3_t-based API declared in math.h (used by qcommon/pmove.cpp)
// ============================================================================

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up) {
    float pitch = DEG2RAD(angles[0]);
    float yaw = DEG2RAD(angles[1]);
    float roll = DEG2RAD(angles[2]);

    float sp = sinf(pitch), cp = cosf(pitch);
    float sy = sinf(yaw), cy = cosf(yaw);
    float sr = sinf(roll), cr = cosf(roll);

    if (forward) {
        forward[0] = cp * cy;
        forward[1] = cp * sy;
        forward[2] = -sp;
    }
    if (right) {
        right[0] = -sr * sp * cy + -cr * -sy;
        right[1] = -sr * sp * sy + -cr * cy;
        right[2] = -sr * cp;
    }
    if (up) {
        up[0] = cr * sp * cy + -sr * -sy;
        up[1] = cr * sp * sy + -sr * cy;
        up[2] = cr * cp;
    }
}

