// gl_model.cpp -- model loading and rendering
// Loads 3D models from files (OBJ, MD5, etc.), manages meshes, materials,
// and renders entity models with textures and animations.

#include "qcommon.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>

namespace render {

// Vertex structure
struct vertex_t {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
    glm::vec4 color;
};

// Material definition
struct material_t {
    char name[64];
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    GLuint texture_id;
};

// Mesh structure
struct mesh_t {
    char name[64];
    std::vector<vertex_t> vertices;
    std::vector<unsigned int> indices;
    material_t material;
    GLuint vao;  // Vertex array object
    GLuint vbo;  // Vertex buffer object
    GLuint ebo;  // Element buffer object
};

// Model structure
struct model_t {
    char name[256];
    std::vector<mesh_t> meshes;
    glm::vec3 mins;
    glm::vec3 maxs;
    float radius;
};

// Model cache
static std::unordered_map<std::string, model_t> model_cache;

// Load an OBJ model file
static bool R_LoadOBJ(const char* filename, model_t* out_model) {
    Com_Printf("Loading OBJ model: %s\n", filename);

    std::ifstream file(filename);
    if (!file.is_open()) {
        Com_Printf("Error: Could not open model file %s\n", filename);
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> normals;
    std::vector<vertex_t> vertices;
    std::vector<unsigned int> indices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            // Vertex position
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        } else if (type == "vt") {
            // Texture coordinate
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            texcoords.push_back(uv);
        } else if (type == "vn") {
            // Vertex normal
            glm::vec3 norm;
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);
        } else if (type == "f") {
            // Face (triangle)
            for (int i = 0; i < 3; i++) {
                std::string vertex_str;
                iss >> vertex_str;

                // Parse v/vt/vn indices
                vertex_t vert;
                vert.color = {1.0f, 1.0f, 1.0f, 1.0f};

                size_t pos1 = vertex_str.find('/');
                if (pos1 != std::string::npos) {
                    int v_idx = std::stoi(vertex_str.substr(0, pos1)) - 1;
                    if (v_idx >= 0 && v_idx < (int)positions.size()) {
                        vert.position = positions[v_idx];
                    }

                    size_t pos2 = vertex_str.find('/', pos1 + 1);
                    if (pos2 != std::string::npos) {
                        int vt_idx = std::stoi(vertex_str.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
                        if (vt_idx >= 0 && vt_idx < (int)texcoords.size()) {
                            vert.texcoord = texcoords[vt_idx];
                        }

                        int vn_idx = std::stoi(vertex_str.substr(pos2 + 1)) - 1;
                        if (vn_idx >= 0 && vn_idx < (int)normals.size()) {
                            vert.normal = normals[vn_idx];
                        }
                    }
                } else {
                    int v_idx = std::stoi(vertex_str) - 1;
                    if (v_idx >= 0 && v_idx < (int)positions.size()) {
                        vert.position = positions[v_idx];
                    }
                }

                vertices.push_back(vert);
                indices.push_back((unsigned int)vertices.size() - 1);
            }
        }
    }

    file.close();

    if (vertices.empty()) {
        Com_Printf("Error: No vertices loaded from %s\n", filename);
        return false;
    }

    // Create mesh
    mesh_t mesh;
    strcpy_s(mesh.name, sizeof(mesh.name), "default");
    mesh.vertices = vertices;
    mesh.indices = indices;

    // Default material
    strcpy_s(mesh.material.name, sizeof(mesh.material.name), "default");
    mesh.material.diffuse = {1.0f, 1.0f, 1.0f};
    mesh.material.texture_id = 0;

    // Setup VAO/VBO/EBO
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex_t), vertices.data(), GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, texcoord));

    glBindVertexArray(0);

    // Calculate bounding box
    out_model->meshes.push_back(mesh);
    strcpy_s(out_model->name, sizeof(out_model->name), filename);

    out_model->mins = positions[0];
    out_model->maxs = positions[0];
    out_model->radius = 0.0f;

    for (const auto& pos : positions) {
        out_model->mins = glm::min(out_model->mins, pos);
        out_model->maxs = glm::max(out_model->maxs, pos);
        out_model->radius = std::max(out_model->radius, glm::length(pos));
    }

    Com_Printf("Model loaded: %d vertices, %d indices\n", (int)vertices.size(), (int)indices.size());
    return true;
}

// Get or load a model
model_t* R_GetModel(const char* name) {
    if (!name) return nullptr;

    // Check cache
    auto it = model_cache.find(name);
    if (it != model_cache.end()) {
        return &it->second;
    }

    // Load new model
    model_t new_model;
    std::memset(&new_model, 0, sizeof(new_model));

    if (!R_LoadOBJ(name, &new_model)) {
        return nullptr;
    }

    // Add to cache
    model_cache[name] = new_model;
    return &model_cache[name];
}

// Render a model at a given position and orientation
void R_DrawModel(model_t* model, const glm::vec3& pos, const glm::vec3& angles, float scale) {
    if (!model || model->meshes.empty()) return;

    glPushMatrix();

    // Transform
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(angles.x, 1.0f, 0.0f, 0.0f);
    glRotatef(angles.y, 0.0f, 1.0f, 0.0f);
    glRotatef(angles.z, 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, scale);

    // Render meshes
    for (const auto& mesh : model->meshes) {
        // Set material
        glColor3f(mesh.material.diffuse.r, mesh.material.diffuse.g, mesh.material.diffuse.b);

        // Bind VAO and draw
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, (GLsizei)mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
    }

    glPopMatrix();
}

// Render entity model
void R_DrawEntity(const edict_t* ent) {
    if (!ent || ent->s.modelindex <= 0) return;

    // TODO: Map modelindex to actual model filename
    // For now, use a placeholder
    char model_name[256];
    sprintf_s(model_name, sizeof(model_name), "models/entity_%d.obj", ent->s.modelindex);

    model_t* model = R_GetModel(model_name);
    if (!model) {
        // Try default model
        model = R_GetModel("models/default.obj");
        if (!model) return;
    }

    R_DrawModel(model, ent->origin, ent->angles, ent->scale);
}

// Clear model cache
void R_ClearModelCache() {
    for (auto& pair : model_cache) {
        model_t& model = pair.second;
        for (auto& mesh : model.meshes) {
            glDeleteBuffers(1, &mesh.vbo);
            glDeleteBuffers(1, &mesh.ebo);
            glDeleteVertexArrays(1, &mesh.vao);
        }
    }
    model_cache.clear();
    Com_Printf("Model cache cleared\n");
}

// Get model bounding box
void R_GetModelBounds(model_t* model, glm::vec3* mins, glm::vec3* maxs) {
    if (!model) {
        *mins = {0, 0, 0};
        *maxs = {0, 0, 0};
        return;
    }

    *mins = model->mins;
    *maxs = model->maxs;
}

// Create a simple default box model (for testing)
model_t* R_CreateBoxModel(const glm::vec3& mins, const glm::vec3& maxs) {
    model_t* model = new model_t();
    std::memset(model, 0, sizeof(*model));
    strcpy_s(model->name, sizeof(model->name), "box");

    // Create a box mesh
    mesh_t mesh;
    strcpy_s(mesh.name, sizeof(mesh.name), "box");

    // Vertices for a box (8 corners)
    mesh.vertices = {
        {{mins.x, mins.y, mins.z}, {-1, -1, -1}, {0, 0}, {1, 1, 1, 1}},
        {{maxs.x, mins.y, mins.z}, {1, -1, -1}, {1, 0}, {1, 1, 1, 1}},
        {{maxs.x, maxs.y, mins.z}, {1, 1, -1}, {1, 1}, {1, 1, 1, 1}},
        {{mins.x, maxs.y, mins.z}, {-1, 1, -1}, {0, 1}, {1, 1, 1, 1}},
        {{mins.x, mins.y, maxs.z}, {-1, -1, 1}, {0, 0}, {1, 1, 1, 1}},
        {{maxs.x, mins.y, maxs.z}, {1, -1, 1}, {1, 0}, {1, 1, 1, 1}},
        {{maxs.x, maxs.y, maxs.z}, {1, 1, 1}, {1, 1}, {1, 1, 1, 1}},
        {{mins.x, maxs.y, maxs.z}, {-1, 1, 1}, {0, 1}, {1, 1, 1, 1}},
    };

    // Indices for box faces
    mesh.indices = {
        0, 1, 2, 0, 2, 3,  // Front
        4, 6, 5, 4, 7, 6,  // Back
        0, 4, 5, 0, 5, 1,  // Bottom
        2, 6, 7, 2, 7, 3,  // Top
        0, 3, 7, 0, 7, 4,  // Left
        1, 5, 6, 1, 6, 2,  // Right
    };

    // Setup VAO/VBO/EBO
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(vertex_t), mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, position));

    glBindVertexArray(0);

    model->meshes.push_back(mesh);
    model->mins = mins;
    model->maxs = maxs;
    model->radius = glm::length((maxs - mins) * 0.5f);

    return model;
}

}  // namespace render
