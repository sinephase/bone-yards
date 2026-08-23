// cmodel.cpp -- collision detection and spatial queries
// Traces axis-aligned boxes through the world against brushes and entities.
// The core of both movement prediction and raycasting for the editor and renderer.

#include "qcommon.h"
#include <algorithm>
#include <cmath>

namespace engine {

// Trace a ray through an axis-aligned box
trace_t CM_BoxTrace(const glm::vec3& start, const glm::vec3& end, 
                    const glm::vec3& mins, const glm::vec3& maxs,
                    const std::vector<brush_t>& brushes, int contentmask) {
    trace_t trace;
    trace.fraction = 1.0f;
    trace.endpos = end;
    trace.surface.flags = 0;
    trace.contents = 0;
    trace.allsolid = false;
    trace.startsolid = false;

    if (brushes.empty()) {
        return trace;
    }

    // Sweep box from start to end, clipping against all brushes
    for (const brush_t& brush : brushes) {
        if (!(brush.contents & contentmask)) continue;

        CM_ClipBoxToBrush(mins, maxs, start, end, &trace, brush);
        if (trace.fraction == 0.0f) break;  // Hit something solid, stop
    }

    trace.endpos = glm::mix(start, end, trace.fraction);
    return trace;
}

// Test a point against brush geometry
int CM_PointContents(const glm::vec3& p, const std::vector<brush_t>& brushes) {
    int contents = 0;

    for (const brush_t& brush : brushes) {
        // Check if point is inside this brush's bounding box
        if (p.x < brush.mins.x || p.x > brush.maxs.x ||
            p.y < brush.mins.y || p.y > brush.maxs.y ||
            p.z < brush.mins.z || p.z > brush.maxs.z) {
            continue;
        }

        // Point is inside the brush
        contents |= brush.contents;
    }

    return contents;
}

// Clip a box against a single brush plane (used by CM_ClipBoxToBrush)
static void CM_ClipBoxToBrushFace(const glm::vec3& mins, const glm::vec3& maxs,
                                  const glm::vec3& start, const glm::vec3& end,
                                  trace_t* trace, const plane_t& plane) {
    // Calculate the distance of the box from the plane at start and end
    float d1 = glm::dot(start, plane.normal) - plane.dist - 1.0f;
    float d2 = glm::dot(end, plane.normal) - plane.dist - 1.0f;

    if (d2 > 0.0f) {
        return;  // Box is on the non-solid side at end
    }

    if (d1 > 0.0f && d2 >= d1) {
        return;  // Box was and stays on the non-solid side
    }

    if (d1 <= 0.0f && d2 <= 0.0f) {
        return;  // Box stays on the solid side (inside the brush)
    }

    // Box crosses the plane -- calculate intersection fraction
    if (d1 < d2) {
        return;  // Moving away from the plane
    }

    float f = d1 / (d1 - d2);
    if (f < 0.0f) f = 0.0f;
    if (f > trace->fraction) return;  // Not closer than previous hit

    trace->fraction = f;
    trace->plane = plane;
    trace->startsolid = (d1 < 0.0f);
}

void CM_ClipBoxToBrush(const glm::vec3& mins, const glm::vec3& maxs,
                       const glm::vec3& start, const glm::vec3& end,
                       trace_t* trace, const brush_t& brush) {
    if (brush.planes.empty()) return;

    // Test each plane of the brush
    for (const plane_t& plane : brush.planes) {
        CM_ClipBoxToBrushFace(mins, maxs, start, end, trace, plane);
    }
}

// Ray-box intersection test (for editor picking)
trace_t CM_RayBox(const glm::vec3& origin, const glm::vec3& dir,
                  const glm::vec3& mins, const glm::vec3& maxs) {
    trace_t trace;
    trace.fraction = 1.0f;

    float tmin = -1e9f, tmax = 1e9f;
    int hit_axis = -1, hit_sign = 0;

    // Test intersection against each box plane
    for (int i = 0; i < 3; i++) {
        float d = dir[i];
        
        if (std::abs(d) < 1e-6f) {
            // Ray is parallel to this axis
            if (origin[i] < mins[i] || origin[i] > maxs[i]) {
                trace.fraction = -1.0f;  // Miss
                return trace;
            }
        } else {
            float inv_d = 1.0f / d;
            float t1 = (mins[i] - origin[i]) * inv_d;
            float t2 = (maxs[i] - origin[i]) * inv_d;

            if (t1 > t2) std::swap(t1, t2);

            if (t1 > tmin) {
                tmin = t1;
                hit_axis = i;
                hit_sign = (d < 0) ? 1 : -1;
            }
            if (t2 < tmax) tmax = t2;

            if (tmin > tmax) {
                trace.fraction = -1.0f;  // Miss
                return trace;
            }
        }
    }

    // Hit detected
    if (tmin > 0.0f) {
        trace.fraction = tmin;
    } else if (tmax > 0.0f) {
        trace.fraction = tmax;
        trace.startsolid = true;
    } else {
        trace.fraction = -1.0f;
        return trace;
    }

    trace.axis = hit_axis;
    trace.sign = hit_sign;
    return trace;
}

// Build planes from brush bounds (for runtime collision)
void CM_BuildBrushPlanes(brush_t& brush) {
    brush.planes.clear();

    // Define the 6 planes of an axis-aligned box
    const glm::vec3 normals[6] = {
        { 1, 0, 0}, {-1, 0, 0},  // +X, -X
        { 0, 1, 0}, { 0,-1, 0},  // +Y, -Y
        { 0, 0, 1}, { 0, 0,-1}   // +Z, -Z
    };

    const float dists[6] = {
        brush.maxs.x, -brush.mins.x,
        brush.maxs.y, -brush.mins.y,
        brush.maxs.z, -brush.mins.z
    };

    for (int i = 0; i < 6; i++) {
        plane_t p;
        p.normal = normals[i];
        p.dist = dists[i];
        brush.planes.push_back(p);
    }
}

// Create a new brush from bounds
brush_t CM_MakeBoxBrush(const glm::vec3& mins, const glm::vec3& maxs, int contents) {
    brush_t brush;
    brush.mins = mins;
    brush.maxs = maxs;
    brush.contents = contents;
    CM_BuildBrushPlanes(brush);
    return brush;
}

}  // namespace engine
