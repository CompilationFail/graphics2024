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

/* Simplified*/
struct Vertex {
    glm::vec3 position; // location 0
    glm::vec2 uv;      // location 1
    glm::vec3 normal;   // location 2
    Vertex();
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
    std::vector <Vertex> vertices;
    std::vector <Object> objects;
    std::unique_ptr <MaterialLib> mtl;
    std::map <VertexIndices, uint32_t> mp;
    GLuint vertex_buffer;
    std::unique_ptr <PhongShader> shader;
public:
    Mesh() { }
    ~Mesh() {
        if(vertex_buffer) {
            glDeleteBuffers(1, &vertex_buffer);
            printf("Delete vertex buffer: %d\n", vertex_buffer);
        }
        printf("Delete program\n");
        shader = nullptr;
    }
    Mesh(const Path &path);
    void init_draw();
    void draw(const glm::mat4&, const glm::vec3 &, const LightSource &) const;
    Bound bound();
    void apply_transform(glm::mat4);
};














