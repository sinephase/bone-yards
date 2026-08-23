// cmodel.cpp -- collision detection and spatial queries
// Traces axis-aligned boxes through the world against brushes and entities.
// The core of both movement prediction and raycasting for the editor and renderer.

#include "qcommon.h"
#include <algorithm>
#include <cmath>
#include <limits>

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
    trace.ent = nullptr;

    if (brushes.empty()) {
        trace.endpos = end;
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
    // We need to account for the box extents along the plane normal
    float d1 = glm::dot(start, plane.normal) - plane.dist;
    float d2 = glm::dot(end, plane.normal) - plane.dist;

    // Add box extent in the direction of the plane normal
    float extent = 0.0f;
    if (plane.normal.x > 0) {
        extent += maxs.x * plane.normal.x;
    } else {
        extent += mins.x * plane.normal.x;
    }
    if (plane.normal.y > 0) {
        extent += maxs.y * plane.normal.y;
    } else {
        extent += mins.y * plane.normal.y;
    }
    if (plane.normal.z > 0) {
        extent += maxs.z * plane.normal.z;
    } else {
        extent += mins.z * plane.normal.z;
    }

    d1 -= extent;
    d2 -= extent;

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
    if (f > 1.0f) f = 1.0f;
    if (f > trace->fraction) return;  // Not closer than previous hit

    trace->fraction = f;
    trace->plane = plane;
    trace->startsolid = (d1 < 0.0f);
    trace->surface.flags = 0;  // TODO: surface flags from brush
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
    trace.ent = nullptr;

    float tmin = -std::numeric_limits<float>::max();
    float tmax = std::numeric_limits<float>::max();
    int hit_axis = -1;

    // Test intersection against each box plane pair
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

    return trace;
}

// Build planes from brush bounds (for runtime collision)
void CM_BuildBrushPlanes(brush_t& brush) {
    brush.planes.clear();

    // Define the 6 planes of an axis-aligned box
    plane_t p;

    // +X plane
    p.normal = {1, 0, 0};
    p.dist = brush.maxs.x;
    brush.planes.push_back(p);

    // -X plane
    p.normal = {-1, 0, 0};
    p.dist = -brush.mins.x;
    brush.planes.push_back(p);

    // +Y plane
    p.normal = {0, 1, 0};
    p.dist = brush.maxs.y;
    brush.planes.push_back(p);

    // -Y plane
    p.normal = {0, -1, 0};
    p.dist = -brush.mins.y;
    brush.planes.push_back(p);

    // +Z plane
    p.normal = {0, 0, 1};
    p.dist = brush.maxs.z;
    brush.planes.push_back(p);

    // -Z plane
    p.normal = {0, 0, -1};
    p.dist = -brush.mins.z;
    brush.planes.push_back(p);
}

// Create a new brush from bounds
brush_t CM_MakeBoxBrush(const glm::vec3& mins, const glm::vec3& maxs, int contents) {
    brush_t brush;
    brush.mins = mins;
    brush.maxs = maxs;
    brush.contents = contents;
    brush.surfaceFlags = 0;
    CM_BuildBrushPlanes(brush);
    return brush;
}

// Check if a sphere intersects a box
bool CM_SphereBoxIntersect(const glm::vec3& sphere_center, float sphere_radius,
                           const glm::vec3& box_mins, const glm::vec3& box_maxs) {
    // Find the closest point on the box to the sphere center
    glm::vec3 closest = glm::clamp(sphere_center, box_mins, box_maxs);
    
    // Calculate distance between sphere center and closest point
    glm::vec3 diff = sphere_center - closest;
    float dist_sq = glm::dot(diff, diff);
    
    return dist_sq <= (sphere_radius * sphere_radius);
}

// Check if two boxes intersect
bool CM_BoxesIntersect(const glm::vec3& mins1, const glm::vec3& maxs1,
                       const glm::vec3& mins2, const glm::vec3& maxs2) {
    return !(maxs1.x < mins2.x || mins1.x > maxs2.x ||
             maxs1.y < mins2.y || mins1.y > maxs2.y ||
             maxs1.z < mins2.z || mins1.z > maxs2.z);
}

}  // namespace engine
