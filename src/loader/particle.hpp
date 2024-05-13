#pragma once
#include "common.hpp"
#include "shader.hpp"
#include <random>

struct VertexInfo {
    glm::vec3 direction;
    glm::vec3 diffuse;
    float angle_bias;
    float r;
    float speed;
    float size;
};

class ParticleShader: public Shader {
    // static const int _center = 0, _v_angle = 1, _particle_size = 2, _transform = 3, _camera = 4, _light = 5;
    int _center, _v_angle, _particle_size, _transform, _camera, _light;
public:
    ParticleShader();
    void set_static(glm::vec3 center, float particle_size);
    void set_dynamic(glm::mat4 transform, float v_angle, glm::vec3 camera, LightSource light);
};

class ParticleSystem {
    std::unique_ptr <ParticleShader> shader;
    float particle_size;
    float r_lb, r_rb;
    glm::vec3 center;
    std::vector <VertexInfo> vertices;
    std::vector <GLuint> indices;
    GLuint vao, vbo;
    std::mt19937 eng;
    double _rand(double l, double r);
public:
    ParticleSystem(float particle_size, glm::vec3 center, float radix_lb, float radix_rb);
    ~ParticleSystem();
    void generate(size_t size);
    void draw(size_t count, glm::mat4 transform, glm::vec3 camera, float v_angle, LightSource light) const;
    void set_particle_size(float size);
};
