#include "scene.hpp"

std::map<std::string, glm::mat4> &Scene::model() {
    return _model;
}
void Scene::init_draw() {
    for(auto &[name, mesh]: meshes) mesh -> init_draw();
}
void Scene::render(glm::mat4 vp, glm::vec3 camera, LightSource light) {
    for(auto &[name, mesh]: meshes) {
        glm::mat mvp = vp;
        if(_model.count(name)) 
            mvp = mvp * _model[name];
        mesh->draw(mvp, camera, light);
    }
}



