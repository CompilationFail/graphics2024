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
    static const int depth_map_width = 1920 * 2, depth_map_height = 1080 * 2;
    
    // shadow mapping are used to calculate DI visibility
    GLuint depth_buffer;
    std::vector <GLuint> depth_map;

    int width, height;
    // Geometry Buffer for first pass
    GLuint buffer, depth, normal, color;
    GLuint buffer2, ssdo, depth2;
    GLuint buffer3, out_a, buffer4, out_b; // for denoiser
    int first;

    GLuint rec_vao, rec_vbo;
    std::unique_ptr <Denoiser> denoiser;
    std::unique_ptr <Mixer> mixer;

    std::unique_ptr <DepthShader> depth_shader;
    std::vector <LightInfo> light_info;
    void render_depth_buffer();
    Scene();
    ~Scene();
    template <class ... T> void load_mesh(std::string name, T ... args) {
        meshes.emplace_back(name, std::make_unique <Mesh> (args ...));
        // meshes.back().second->apply_transform(meshes.back().second->bound().to_local());
    }
    void load(Path path);
    std::map <std::string, std::vector<glm::mat4>> &model();
    void init_draw(int width, int height);
    void activate_shadow();
    void update_light(std::vector <LightInfo> info);
    void render(GLFWwindow *window, glm::mat4 vp, glm::vec3 camera, float time, float denoise_alpha = 0.02f, float movement = 0.f);
};

