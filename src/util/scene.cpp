#include "scene.hpp"

Scene::Scene()
    : shadow(0), depth_buffer(0) {}
Scene::~Scene() {
    depth_shader = nullptr;
}
std::map<std::string, std::vector<glm::mat4>> &Scene::model() {
    return _model;
}
void Scene::init_draw() {
    for(auto &[name, mesh]: meshes) mesh -> init_draw();
}
void Scene::activate_shadow() {
    shadow = 1;
    depth_shader = std::make_unique <DepthShader> ();
    
    glGenFramebuffers(1, &depth_buffer);  
}
void Scene::update_light(std::vector <LightInfo> info) {
    light_info = info;
}
void Scene::render(GLFWwindow *window, glm::mat4 vp, glm::vec3 camera) {
    glfwPollEvents();
    CheckGLError();
    if (shadow) {
        render_depth_buffer();
    }
    glfwSwapBuffers(window);
    CheckGLError();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    CheckGLError();
    glViewport(0, 0, width, height);
    CheckGLError();
    glClearColor(0., 0., 0., 1.);
    CheckGLError();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CheckGLError();
    glEnable(GL_DEPTH_TEST);
    CheckGLError();
    glDepthFunc(GL_LESS);
    CheckGLError();
    for(auto &[name, mesh]: meshes) {
        if(!_model.count(name)) {
            mesh->draw(glm::mat4(1.f), vp, camera, light_info, depth_map);
        } else {
            for(auto model: _model[name]) {
                mesh->draw(model, vp, camera, light_info, depth_map);
            }
        }
        // mesh->draw(_model.count(name) ? _model[name] : glm::mat4(1.f), vp, camera, light_info, depth_map);
    }
}
void Scene::render_depth_buffer() {
    while(light_info.size() > depth_map.size()) {
        depth_map.push_back(0);
        auto &i = depth_map.back();
        glGenTextures(1, &i);  
        glBindTexture(GL_TEXTURE_2D, i);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                    depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
    }
    glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);

    for(int i = 0; i < (int)light_info.size(); ++i) {
        auto &light = light_info[i];
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glViewport(0, 0, depth_map_width, depth_map_height);

        glClear(GL_DEPTH_BUFFER_BIT);
        CheckGLError();
        glEnable(GL_DEPTH_TEST);
        CheckGLError();
        glDepthFunc(GL_LESS);
        CheckGLError();
        depth_shader -> use();
        auto vp = light.vp();
        for(auto &[name, mesh]: meshes) {
            glm::mat mvp = vp;
            if(_model.count(name)) {
                for(auto model: _model[name]) {
                    glm::mat mvp = vp * model;
                    depth_shader ->set_transform(mvp);
                    mesh->draw_depth();
                }
            } else {
                depth_shader -> set_transform(vp);
                mesh->draw_depth();
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError();
}


