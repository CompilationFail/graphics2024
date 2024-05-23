#include "scene.hpp"

Scene::Scene()
    : shadow(0), light_intense(0, 0, 0), light_position(0, 0, 0),
      light_vp(1.f), depth_map(0), light_direction(0, 0, 1) {}
std::map<std::string, glm::mat4> &Scene::model() {
    return _model;
}
void Scene::init_draw() {
    for(auto &[name, mesh]: meshes) mesh -> init_draw();
}
void Scene::activate_shadow(int width, int height, float fov) {
    shadow = 1;
    depth_map_height = height;
    depth_map_width = width;
    depth_map_fov = fov;
    depth_shader = std::make_unique <DepthShader> ();
    
    glGenFramebuffers(1, &depth_buffer);  

    glGenTextures(1, &depth_map);  
    glBindTexture(GL_TEXTURE_2D, depth_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                depth_map_width, depth_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
    
    glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Scene::update_light(Camera light_camera, glm::vec3 intense) {
    if(shadow) {
        light_vp =
            glm::perspective(depth_map_fov,
                            1.f * depth_map_width / depth_map_height, .1f, 100.f) *
            light_camera.view();
    }
    light_position = light_camera.position; 
    light_intense = intense;
    light_direction = light_camera.dir();
}
void Scene::render(GLFWwindow *window, glm::mat4 vp, glm::vec3 camera) {
    glfwPollEvents();
    CheckGLError();
    if (shadow) {
        render_depth_buffer(light_vp);
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
        mesh->draw(_model.count(name) ? _model[name] : glm::mat4(1.f), vp, camera, light_position, light_intense, light_direction, shadow ? depth_map: 0, light_vp);
    }
}
void Scene::render_depth_buffer(glm::mat4 vp) {
    glViewport(0, 0, depth_map_width, depth_map_height);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_buffer);
    CheckGLError();
    glClear(GL_DEPTH_BUFFER_BIT);
    CheckGLError();
    glEnable(GL_DEPTH_TEST);
    CheckGLError();
    glDepthFunc(GL_LESS);
    CheckGLError();
    depth_shader -> use();
    for(auto &[name, mesh]: meshes) {
        glm::mat mvp = vp;
        if(_model.count(name)) 
            mvp = mvp * _model[name];
        depth_shader ->set_transform(mvp);
        mesh->draw_depth();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLError();
}


