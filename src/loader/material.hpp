#pragma once
#include "common.hpp"
#include "texture.hpp"
#include <memory>
#include <map>

struct Material {
    float Ns; // specular exponent
    glm::vec3 Ka; // ambient color
    glm::vec3 Kd; // diffuse color
    glm::vec3 Ks; // specular color
    glm::vec3 Ke; // emit color
    uint32_t illum; // illumination type
    /*
    0. Color on and Ambient off
    1. Color on and Ambient on
    2. Highlight on
    3. Reflection on and Ray trace on
    4. Transparency: Glass on, Reflection: Ray trace on
    5. Reflection: Fresnel on and Ray trace on
    6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
    7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
    8. Reflection on and Ray trace off
    9. Transparency: Glass on, Reflection: Ray trace off
    10. Casts shadows onto invisible surfaces
    */
    std::unique_ptr <Texture2D> texture;
    glm::vec3 texture_scale;
    // only support kd texture now
    Material();
    void verify();
};

class MaterialLib {
    std::vector <Material*> materials;
    std::map <std::string, Material*> pool;
public:
    ~MaterialLib();
    int load(const Path &path);
    Material* operator [] (const std::string &str);
};

