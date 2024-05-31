#pragma once
#include "common.hpp"

const glm::vec3 worldUp(0,1,0);

struct Camera {
    float pitch, yaw;
    glm::vec3 position;
    Camera(float pitch = 0, float yaw = 0, glm::vec3 position = glm::vec3(0));
    glm::vec3 dir();
    glm::vec3 right();
    glm::vec3 up();
    glm::vec3 dir4(int d);
    glm::mat4 view();
};

enum LightType {
    POINT_LIGHT = 0,
    CONE_LIGHT = 1,
    DIRECTIONAL_LIGHT = 2,
};

struct LightInfo {
    Camera camera;
    glm::vec3 intense;
    LightType type;
    LightInfo(Camera camera, glm::vec3 intense, LightType type);
    glm::mat4 vp();
};