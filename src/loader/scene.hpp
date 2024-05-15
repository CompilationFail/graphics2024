#pragma once
#include "common.hpp"
#include <map>
#include <string>
#include "loader.hpp"
#include "shader.hpp"

class Scene {
    std::map <std::string, glm::mat4> _model;
    std::vector <std::pair <std::string, std::unique_ptr<Mesh>>> meshes;
public:
    template <class ... T> void load_mesh(std::string name, T ...);
    void init_draw();
    std::map <std::string, glm::mat4> &model();
    void render(glm::mat4 vp, glm::vec3 camera, LightSource light);
};