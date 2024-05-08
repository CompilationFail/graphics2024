#include "bound.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Bound::reset() {
    x1 = y1 = z1 = std::numeric_limits<float>::max();
    x2 = y2 = z2 = std::numeric_limits<float>::min();
}
Bound::Bound() {
    reset();
}
Bound::Bound(glm::vec3 vec) {
    x1 = x2 = vec.x;
    y1 = y2 = vec.y;
    z1 = z2 = vec.z;
}
Bound::Bound(glm::vec4 vec) {
    x1 = x2 = vec.x;
    y1 = y2 = vec.y;
    z1 = z2 = vec.z;
}
Bound& Bound::operator += (const glm::vec3 &rhs) {
    x1 = std::min(x1, rhs.x);
    y1 = std::min(y1, rhs.y);
    z1 = std::min(z1, rhs.z);
    x2 = std::max(x2, rhs.x);
    y2 = std::max(y2, rhs.y);
    z2 = std::max(z2, rhs.z);
    return *this;
}
Bound& Bound::operator += (const glm::vec4 &rhs) {
    x1 = std::min(x1, rhs.x);
    y1 = std::min(y1, rhs.y);
    z1 = std::min(z1, rhs.z);
    x2 = std::max(x2, rhs.x);
    y2 = std::max(y2, rhs.y);
    z2 = std::max(z2, rhs.z);
    return *this;
}
Bound& Bound::operator += (const Bound &rhs) {
    x1 = std::min(x1, rhs.x1);
    y1 = std::min(y1, rhs.y1);
    z1 = std::min(z1, rhs.z1);
    x2 = std::max(x2, rhs.x2);
    y2 = std::max(y2, rhs.y2);
    z2 = std::max(z2, rhs.z2);
    return *this;
}
Bound::Bound(std::vector<glm::vec3> vec) {
    reset();
    for(const auto &i: vec) *this += i;
}
Bound::Bound(std::vector <glm::vec4> vec) {
    reset();
    for(const auto &i: vec) *this += i;
}
Bound Bound::operator + (const Bound &rhs) const {
    return Bound(*this) += rhs;
}
glm::mat4 Bound::to_local() const {
    float scl = glm::min(glm::min(2 / (x2 - x1), 2 / (y2 - y1)), 2 / (z2 - z1));
    return glm::scale(glm::mat4(1.f), glm::vec3(scl))
    * glm::translate(glm::mat4(1.f), -glm::vec3((x1+x2)/2.f,(y1+y2)/2.f,(z1+z2)/2.f));
}