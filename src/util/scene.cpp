#include "scene.hpp"

Scene::Scene()
    : shadow(0), light_intense(0, 0, 0), light_position(0, 0, 0),
      light_vp(1.f) {}
std::map<std::string, glm::mat4> &Scene::model() {
    return _model;
}
void Scene::init_draw() {
    for(auto &[name, mesh]: meshes) mesh -> init_draw();
}
void Scene::activate_shadow(int width, int height, float fov) {
    depth_shader = std::make_unique <DepthShader> ();
    glGenFramebuffers(1, &depth_map);  
    depth_map_height = height;
    depth_map_width = width;
    depth_map_fov = fov;
}
void Scene::update_light(Camera light_camera, glm::vec3 intense) {
    if(shadow) {
        light_vp =
            glm::perspective(depth_map_fov,
                            1.f * depth_map_width / depth_map_height, .1f, 100.f) *
            light_camera.view();
    }
    light_intense = intense;
}
void Scene::render(glm::mat4 vp, glm::vec3 camera) {
    if (shadow) {
        render_depth_buffer(light_vp);
    }
    for(auto &[name, mesh]: meshes) {
        glm::mat mvp = vp;
        if(_model.count(name)) 
            mvp = mvp * _model[name];
        mesh->draw(mvp, camera, light_position, light_intense, shadow ? depth_map: 0, light_vp);
    }
}
void Scene::render_depth_buffer(glm::mat4 vp) {
    depth_shader -> use();
    glViewport(0, 0, depth_map_width, depth_map_height);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map);
    glClear(GL_DEPTH_BUFFER_BIT);
    for(auto &[name, mesh]: meshes) {
        glm::mat mvp = vp;
        if(_model.count(name)) 
            mvp = mvp * _model[name];
        depth_shader ->set_transform(mvp);
        mesh->draw_depth();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


