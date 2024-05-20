#pragma once
#include "common.hpp"
#include <map>
#include <string>
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"

class Scene {
    std::map <std::string, glm::mat4> _model;
    std::vector <std::pair <std::string, std::unique_ptr<Mesh>>> meshes;
    GLuint depth_map;
    int shadow, depth_map_width, depth_map_height; float depth_map_fov;
    std::unique_ptr <DepthShader> depth_shader;
    glm::mat4 light_vp; glm::vec3 light_position, light_intense;
    void render_depth_buffer(glm::mat4 transform);
public:
    Scene();
    template <class ... T> void load_mesh(std::string name, T ... args) {
        meshes.emplace_back(name, std::make_unique <Mesh> (args ...));
    }
    void init_draw();
    void update_light(Camera light, glm::vec3 intense);
    void activate_shadow(int width = 1000, int height = 1000, float fov = 45.);
    std::map <std::string, glm::mat4> &model();
    void render(glm::mat4 vp, glm::vec3 camera);
};

