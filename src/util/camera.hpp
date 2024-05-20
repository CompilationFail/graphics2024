#pragma once
#include "common.hpp"

const glm::vec3 worldUp(0,1,0);

struct Camera {
    float pitch, yaw;
    glm::vec3 position;
    glm::vec3 dir();
    glm::vec3 right();
    glm::vec3 up();
    glm::vec3 dir4(int d);
    glm::mat4 view();
};

