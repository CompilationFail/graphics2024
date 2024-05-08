#include "material.hpp"
#include <iostream>

Material::Material() : Ns(7),
                       Ka(0, 0, 0),
                       Kd(0, 0, 0),
                       Ks(1, 1, 1),
                       Ke(0, 0, 0),
                       illum(1),
                       texture_scale(1, 1, 1) { }
void Material::verify() {
    Ns = std::clamp(Ns, 0.f, 1000.f);
    Ka = clamp_color(Ka);
    Kd = clamp_color(Kd);
    Ks = clamp_color(Ks);
    Ke = clamp_color(Ke);
    if (illum != std::clamp(illum, 0u, 10u))
    {
        warn(1, "Unsupported illum: (%d), fallback to illum 0", illum);
        illum = std::clamp(illum, 0u, 10u);
    }
}

MaterialLib::~MaterialLib() {
    for(auto i: materials) delete i;
}

Material *MaterialLib::operator[](const std::string &str) {
    if (pool.count(str))
        return pool[str];
    return nullptr;
}

int MaterialLib::load(const Path &path) {
    printf("Mtl: Load from %s\n", path.u8string().c_str());
    clock_t begin_time = clock();
    auto filename = path.filename().u8string();
    FILE *f = fopen(path.u8string().c_str(), "r");
    if(f == nullptr) 
        throw "fail to open file";
    if(filename.size() < 4 || filename.substr(filename.size() - 4, 4) != ".mtl") 
        warn(2, "%s: not a mtl file", filename.c_str());
    static char buf[BUFFLEN];
    Material *cur = nullptr;
    int count = 0;
    while(std::fgets(buf, BUFFLEN, f) != nullptr) {
        size_t len = strlen(buf); 
        while(len && isspace(buf[len - 1])) buf[--len] = 0;
        if(len == BUFFLEN) warn(3, "Mtl: Line length exceeded bufflen");
        char *pos = strstr(buf, "#");
        if(pos != nullptr) *pos = '\0';
        pos = buf;
        while(*pos && isspace(*pos)) pos++;
        len = strlen(pos);
        if(len <= 1) continue;
        if(str_equal(pos, "newmtl ")) {
            cur = new Material();
            materials.push_back(cur);
            std::string name(pos + 7);
            warn(0, "NEW %s\n", pos + 7);
            pool[name] = cur;
            count++;
        } else if(str_equal(pos, "Ns ")) {
            if(sscanf_s(pos + 3, "%f", &cur -> Ns) != 1) {
                warn(1, "Ns: Expected 1 float, found: %s", pos + 3);
            }
        } else if(str_equal(pos, "Ka ")) {
            readvec3(pos + 3, &cur -> Ka, "Ka");
        } else if(str_equal(pos, "Kd ")) {
            readvec3(pos + 3, &cur -> Kd, "Kd");
        } else if(str_equal(pos, "Ks ")) {
            readvec3(pos + 3, &cur -> Ks, "Ks");
        } else if(str_equal(pos, "illum ")) {
            if(sscanf_s(pos + 6, "%u", &cur -> illum) != 1) {
                warn(1, "illum: Expected 1 int, found: %s", pos + 3);
            }
        } else if(str_equal(pos, "map")) {
            if(str_equal(pos, "map_Kd")) {
                pos += 7;
                while(*pos && *pos == ' ') pos++;
                if(str_equal(pos, "-s")) {
                    pos += 2;
                    while(*pos && *pos == ' ') pos++;
                    if(readvec3(pos, &cur -> texture_scale, "Map-s")) {
                        for(int i = 0; i < 3; ++i) {
                            pos = strstr(pos, " ");
                            while(*pos && *pos == ' ') pos++;
                        }
                    }
                }
                Path texture_path = path.parent_path();
                texture_path /= Path(unescape(pos));
                try {
                    cur -> texture = std::make_unique <Texture2D> (texture_path);
                } catch(std::string err) {
                    warn(2, "[ERROR] Fail to load texture2D: %s", err.c_str());
                }
            } else {
                char *tmp = strstr(pos, " ");
                if(tmp != nullptr) *tmp = '\0';
                warn(0, "Mtl: unsupported texture map: %s", pos + 3);
            }
        } else {
            char *tmp = strstr(pos, " ");
            if(tmp != nullptr) *tmp = '\0';
            warn(0, "Mtl: unsupported argument: %s", pos);
        }
    }
    /*for(const auto& [i,j]: pool) {
        printf("%s %u %p\n", i.c_str(), j->illum, j);
    }*/
    printf("MTL loaded, time: %lfs\n", 1. * (clock() - begin_time) / CLOCKS_PER_SEC);
    // exit(0);
    return count;
}

