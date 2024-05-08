#include "common.hpp"


int str_equal(const char *buf, const char *payload) {
    size_t l = strlen(payload);
    return !memcmp(buf, payload, l);
}

std::string unescape(const char *str) {
    std::string output;
    for(const char *c = str; *c; ++c) {
        if (*c == '\\' && *(c + 1)) {
            switch (*(c+1)) {
                case 'n': // Replace \n with newline
                    output += '\n';
                    c++;
                    break;

                case 't': // Replace \t with tab
                    output += '\t';
                    c++;
                    break;

                case '\\': // Replace double backslash
                    output += '\\';
                    c++;
                    break;

                case '"': // Replace \" with "
                    output += '"';
                    c++;
                    break;

                default:
                    output += *c;
                    break; // In case of an unknown escape sequence, keep the original
            }
        } else {
            output += *c;
        }
    }
    return output;
}

bool readvec3(const char *str, glm::vec3 *dst, const char *arg) {
    if(sscanf_s(str, "%f%f%f", &dst -> x, &dst -> y, &dst -> z) != 3) {
        warn(2, "While parsing %s : Expected 3 floats, finds: %s", arg, str);
        return 0;
    }
    return 1;
}
bool readvec2(const char *str, glm::vec2 *dst, const char *arg) {
    if(sscanf_s(str, "%f%f", &dst -> x, &dst -> y) != 2) {
        warn(2, "While parsing %s : Expected 2 floats, finds: %s", arg, str);
        return 0;
    }
    return 1;
}

glm::vec3 clamp_color(glm::vec3 &color) {
    return {
        std::clamp(color.x, 0.f, 1.f),
        std::clamp(color.y, 0.f, 1.f),
        color.z = std::clamp(color.z, 0.f, 1.f)};
}

glm::vec3 apply_transform_vec3(glm::vec3 vec, glm::mat4 trans) {
    auto v = trans * glm::vec4(vec, 1);
    return v.xyz() / v.w;
}

void _CheckGLError(const char* file, int line)
{
    GLenum err ( glGetError() );

    while ( err != GL_NO_ERROR )
    {
        std::string error;
        switch ( err )
        {
            case GL_INVALID_OPERATION:  error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:       error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:      error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:      error="OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        fprintf(stderr, "GL %s - %s: %d\n", error.c_str(), file, line);
        // std::cout << "GL_" << error.c_str() <<" - " << file << ":" << line << std::endl;
        err = glGetError();
    }

    return;
}