#pragma once

#include <glm/glm.hpp>
#include <map>
#include "texture.hpp"
#include <optional>
#include "common.hpp"
#include <cstring>
#include <cstdio>
#include "bound.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "camera.hpp"

/* Simplified*/
struct Vertex {
    glm::vec3 position; // location 0
    glm::vec2 uv;      // location 1
    glm::vec3 normal;   // location 2
    Vertex();
    Vertex(glm::vec3 position, glm::vec2 uv, glm::vec3 normal);
};

struct VertexIndices {
    uint32_t positionIndex;
    uint32_t uvIndex;
    uint32_t normalIndex;
    bool operator < (const VertexIndices &rhs) const {
        if (positionIndex != rhs.positionIndex)
            return positionIndex < rhs.positionIndex;
        if (uvIndex != rhs.uvIndex)
            return uvIndex < rhs.uvIndex;
        return normalIndex < rhs.normalIndex;
    }
};

class Object {
    std::string name;
    Material *_material;
    GLuint vao;
public:
    std::vector <uint32_t> triangles;
    ~Object() {
        if(vao) {
            glDeleteVertexArrays(1, &vao);
            printf("Delete vao: %d\n", vao);
        }
    }
    Material *material() const;
    Object(const std::string &,
         const std::vector<uint32_t> &,
         Material *);
    const char* c_name() const;
    void init_draw();
    void draw() const;
};

class Mesh { 
public:
    std::vector <Vertex> vertices;
    std::vector <Object> objects;
    std::unique_ptr <MaterialLib> mtl;
    std::map <VertexIndices, uint32_t> mp;
    GLuint vertex_buffer;
    std::unique_ptr <SSDO> shader1, shader2;
    // std::unique_ptr <PhongShader> shader;
    Mesh() { }
    ~Mesh() {
        if(vertex_buffer) {
            glDeleteBuffers(1, &vertex_buffer);
            printf("Delete vertex buffer: %d\n", vertex_buffer);
        }
        printf("Delete program\n");
        shader1 = nullptr, shader2 = nullptr;
    }
    /*
     * Load from a [.obj] file
     */
    Mesh(const Path &path);
    /*
     * To generate a triangle
     */
    Mesh(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 normal, glm::vec3 color);
    void init_draw();
    void draw(glm::mat4 model, glm::mat4 vp, glm::vec3 camera,
              std::vector<LightInfo> light_info, std::vector<GLuint> depth_map,
              int render_pass, 
              GLuint depth = 0, GLuint normal = 0, GLuint color = 0);
    void draw_depth() const;
    Bound bound();
    void apply_transform(glm::mat4);
};














