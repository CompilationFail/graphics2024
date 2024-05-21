#pragma once
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "common.hpp"
#include "material.hpp"

GLuint prepare_program();

GLuint load_shader_from_text(const char *, GLenum);
GLuint load_shader_from_path(const char *, GLenum);
GLuint link_program(GLuint *, uint32_t);

GLuint prepare_phong_shader();
GLuint prepare_shader(const char *vert, const char* frag);

/*struct LightSource {
    glm::vec3 position;
    glm::vec3 intense;
    glm::vec3 color;
};*/

class Shader {
    GLuint _program;
    std::map <std::string, GLint> uniforms;
public:
    Shader(const char* vert, const char* frag);
    ~Shader();
    GLint loc(const char*);
    void use();
    void init_uniform(std::vector <std::string>);
    void check_uniform();
    GLint uniform(std::string);
};

class PhongShader: public Shader {
    // const GLint trans = 0, Ka = 1, Kd = 2, scale = 3, type = 4, camera = 5, light = 6;
    GLint trans, Ka, Kd, scale, norm_scale, has_tex, has_tex_norm, camera, light_position, light_intense, light_direction, light_transform, depth_map, tex, tex_norm, has_depth_map;
public:
    PhongShader();
    void set_transform(glm::mat4 transform);
    void set_material(Material *material);
    void set_light(glm::vec3 light_position, glm::vec3 light_intense, glm::vec3 light_direction);
    void set_camera(glm::vec3 camera);
    void set_depth(GLuint depth_map, glm::mat4 light_transform);
};

class DepthShader: public Shader {
    GLint trans;
public:
    DepthShader();
    void set_transform(glm::mat4 transform);
};


