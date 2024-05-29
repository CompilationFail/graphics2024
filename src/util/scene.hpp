#pragma once
#include "common.hpp"
#include <map>
#include <string>
#include "mesh.hpp"
#include "shader.hpp"
#include "camera.hpp"

class Scene {
public:
    std::map <std::string, std::vector <glm::mat4>> _model;
    std::vector <std::pair <std::string, std::unique_ptr<Mesh>>> meshes;
    int shadow;
    static const int depth_map_width = 1000, depth_map_height = 1000;
    // shadow mapping are used to render directional light.
    GLuint depth_buffer;
    std::vector <GLuint> depth_map;
    std::unique_ptr <DepthShader> depth_shader;
    std::vector <LightInfo> light_info;
    void render_depth_buffer();
    Scene();
    ~Scene();
    template <class ... T> void load_mesh(std::string name, T ... args) {
        meshes.emplace_back(name, std::make_unique <Mesh> (args ...));
        // meshes.back().second->apply_transform(meshes.back().second->bound().to_local());
    }
    std::map <std::string, std::vector<glm::mat4>> &model();
    void init_draw();
    void activate_shadow();
    void update_light(std::vector <LightInfo> info);
    void render(GLFWwindow *window, glm::mat4 vp, glm::vec3 camera);
};

