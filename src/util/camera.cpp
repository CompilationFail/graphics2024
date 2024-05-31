#include "camera.hpp"
#include "json.hpp"

Camera::Camera(float pitch, float yaw, glm::vec3 position)
    : pitch(pitch), yaw(yaw), position(position) {}
glm::vec3 Camera::dir() {
    glm::vec3 direction;
    direction.x = cos(yaw) * cos(pitch);
    direction.y = sin(pitch);
    direction.z = sin(yaw) * cos(pitch);
    direction = glm::normalize(direction);
    return direction;
}
glm::vec3 Camera::right() {
    return glm::normalize(glm::cross(dir(), worldUp));
}
glm::vec3 Camera::up() {
    return glm::cross(right(), dir());
}

glm::vec3 Camera::dir4(int d) {
    if(d == 0) return dir();
    if(d == 2) return -dir();
    if(d == 1) return -right();
    return right();
}
glm::mat4 Camera::view() {
    return glm::lookAt(position, position + dir(), up());
}

LightInfo::LightInfo(Camera camera, glm::vec3 intense, LightType type)
    : camera(camera), intense(intense), type(type) {}
glm::mat4 LightInfo::vp() {
    if(type != DIRECTIONAL_LIGHT) {
        static const float aspect_ratio = 1.f, fov = glm::radians(45.f);
        // Parameter: fov, aspect_ratio
        return glm::perspective(fov, aspect_ratio, .1f, 100.f) *
             camera.view();
    } else {
        static const float L = 100.f;
        return glm::ortho(-L, L, -L, L, .1f, 100.f) *
             camera.view();
    }
}