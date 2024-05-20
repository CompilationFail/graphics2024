#pragma once
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <GL/glew.h>
#include <cstring>
#include <vector>

namespace fs = std::filesystem;
using Path = fs::path;

static const size_t BUFFLEN = 1000;
static const int WARNING_LEVEL = 0;
const float PI = acos(-1);

template<typename ... Args>
void warn(int level, const char *format, Args ... args) {
    if(level < WARNING_LEVEL) return;
    static char buf[1024];
    snprintf(buf, 1000, format, args ...);
    fprintf(stderr, "Warning: %s\n", buf);
}
std::string unescape(const char *);
int str_equal(const char *buf, const char *payload);
bool readvec3(const char *str, glm::vec3 *dst, const char *arg);
bool readvec2(const char *str, glm::vec2 *dst, const char *arg);

glm::vec3 clamp_color(glm::vec3 &color);
glm::vec3 apply_transform_vec3(glm::vec3, glm::mat4);

static std::vector<char> readFile(const char *filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    // ate: start at the end

    if (!file.is_open()) {
        throw std::runtime_error(std::string("failed to open file: ") + filename);
    }
    // pointer is at the end
    // therefore we can use file point to get file size
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize, 0);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

void uniform_vec3(GLint loc, glm::vec3 vec);

void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

// #define CheckGLError() 42
