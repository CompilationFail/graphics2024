#pragma once
#include "common.hpp"
#include <limits>
#include <vector>
#include <glm/glm.hpp>

struct Bound {
    float x1, x2;
    float y1, y2;
    float z1, z2;
    void reset();
    Bound();
    Bound(glm::vec3 vec);
    Bound(glm::vec4 vec);
    Bound(std::vector <glm::vec3> vec);
    Bound(std::vector <glm::vec4> vec);
    Bound& operator += (const Bound &rhs);
    Bound operator + (const Bound &rhs) const;
    Bound& operator += (const glm::vec3 &rhs);
    Bound& operator += (const glm::vec4 &rhs);
    glm::mat4 to_local() const;
};










