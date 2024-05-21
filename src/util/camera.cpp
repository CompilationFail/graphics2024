#include "camera.hpp"

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
    if(d == 3) return right();
    assert(0);
}
glm::mat4 Camera::view() {
    return glm::lookAt(position, position + dir(), up());
}

