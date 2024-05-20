#include "shader.hpp"

GLuint load_shader_from_text(const char *source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char *sources[] = {source};
    glShaderSource(shader, 1, sources, NULL);
    glCompileShader(shader);

    GLint is_compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE)
    {
        GLint max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
        // The maxLength includes the NULL character
        std::string error_log; error_log.resize(max_length);
        glGetShaderInfoLog(shader, max_length, &max_length, &error_log[0]);
        glDeleteShader(shader); // Don't leak the shader.
        throw error_log;
    }
    return shader;
}
GLuint load_shader_from_path(const char *path, GLenum type) {
    auto source = readFile(path);
    source.push_back('\0');
    return load_shader_from_text(source.data(), type);
}

GLuint link_program(GLuint *shaders, uint32_t shader_count) {
    GLuint program = glCreateProgram();
    for (uint32_t i = 0; i < shader_count; i++) {
      glAttachShader(program, shaders[i]);
    }
    glLinkProgram(program);

  int success;
  // check for linking errors
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLint max_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
    std::vector<GLchar> error_log(max_length);
    glGetProgramInfoLog(program, max_length, &max_length, &error_log[0]);
    throw error_log.data();
    glDeleteProgram(program);
  }
  return program;
}
GLuint prepare_shader(const char *vert, const char *frag)
{
    GLuint shaders[2];
    shaders[0] = load_shader_from_text(vert, GL_VERTEX_SHADER);
    CheckGLError();
    shaders[1] = load_shader_from_text(frag, GL_FRAGMENT_SHADER);
    CheckGLError();
    return link_program(shaders, 2);
}



Shader::Shader(const char *vert, const char *frag)
{
    _program = prepare_shader(vert, frag);
    printf("Shader loaded\n");
}
void Shader::use() {
    glUseProgram(_program);
}

GLint Shader::loc(const char *name) {
    return glGetUniformLocation(_program, name);
}

Shader::~Shader() {
    printf("shader destroyed\n");
    glDeleteShader(_program);
}
void Shader::init_uniform(std::vector <std::string> names) {
    for(auto &name: names) uniforms[name] = loc(name.c_str());
}
void Shader::check_uniform() {
    for(auto [name, loc]: uniforms) {
        printf("Uniform %s location = %d\n", name.c_str(), loc);
    }
}
GLint Shader::uniform(std::string name) {
    return uniforms[name];
}


namespace Phong {

static const char *vertex_shader_text = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 transform;

out vec2 o_uv;
out vec3 o_pos;
out vec3 o_norm;

void main() {
    gl_Position = transform * vec4(position, 1.);
    o_uv = uv;
    o_pos = position;
    o_norm = normal;
}
)";

static const char *fragment_shader_text = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

in vec2 o_uv;
in vec3 o_pos;
in vec3 o_norm;

uniform sampler2D tex;
uniform sampler2D tex_norm;
uniform vec3 m_ka;
uniform vec3 m_kd;
uniform vec3 tex_scale;
uniform vec3 tex_norm_scale;
uniform int has_tex;
uniform int has_tex_norm;
uniform vec3 camera;

uniform vec3 light_position;
uniform vec3 light_intense;
uniform mat4 light_transform;
uniform sampler2D depth_map;


layout(location = 0) out vec4 frag_color;


void main() {
    vec3 color = m_kd;
    if(has_tex == 1) {
        color = vec3(texture(tex, vec2(o_uv.x / tex_scale.x, o_uv.y / tex_scale.y)));
    }
    vec3 norm = o_norm;
    if(has_tex_norm == 1) {
        norm = vec3(texture(tex_norm, vec2(o_uv.x / tex_norm_scale.x, o_uv.y / tex_norm_scale.y)));
        norm = norm * 2 - vec3(1,1,1);
        norm = normalize(norm);
        vec3 tmp;
        tmp.y = norm.z;
        tmp.x = norm.x;
        tmp.z = norm.y;
        norm = tmp;
    }
    // frag_color = vec4(norm, 1);
    // frag_color = vec4(color, 1);
    // frag_color = vec4(vec3(0.5,0.5,0.5)+norm/2,1);
    // frag_color = vec4(o_pos / 5 + vec3(0.5,0.5,0.5), 1);
    // return;
    vec3 i = light_position - o_pos;
    float r = dot(i, i);
    i = normalize(i);
    vec3 v = normalize(camera - o_pos);
    // frag_color = vec4((v+vec3(1,1,1))/2, 0);
    // frag_color = vec4((camera + vec3(10,10,10))/20, 0);
    // frag_color = vec4(dot(i,i), 0, 0, 0);
    // return;
    vec3 h = normalize((i + v) / 2);
    float theta = dot(i, norm);

    /*if(dot(norm, v) < 0) {
        frag_color = vec4(0, 0, 0, 0);
        return;
    }*/

    vec3 intense = light_intense / r;
    vec3 diffuse = color * intense * max(0, theta);

    float alpha = dot(norm, h);
    
    vec3 specular = color * intense * pow(max(0, alpha), 200);
    if(alpha < 0) {
        specular = vec3(0,0,0);
    }

    vec3 ambient = color * light_intense * 0.02;

    if(depth_map != 0) {
        vec3 pos = vec3(light_transform * vec4(o_pos, 1));
        if(pos.x >= 0 && pos.x < 1 && pos.y >= 0 && pos.y < 1) {
            float depth = float(texture(depth_map, vec2(pos.x,pos.y)));
            if(depth < pos.z) {
                diffuse = vec3(0, 0, 0);
                specular = vec3(0, 0, 0);
            } 
        }
    }

    // frag_color = vec4(diffuse, 0);
    frag_color = vec4(diffuse + specular + ambient, 0);
    // frag_color = vec4(color, 0);
    // frag_color = vec4(specular, 0);
}
)";

}

PhongShader::PhongShader() : Shader(Phong::vertex_shader_text, Phong::fragment_shader_text) {
    trans = loc("transform");
    Ka = loc("m_ka");
    Kd = loc("m_kd");
    has_tex = loc("has_tex");
    has_tex_norm = loc("has_tex_norm");
    scale = loc("tex_scale");
    norm_scale = loc("tex_norm_scale");
    camera = loc("camera");
    light_position = loc("light_position");
    light_intense = loc("light_intense");
    light_transform = loc("light_transform");
    depth_map = loc("depth_map");
    tex = loc("tex");
    tex_norm = loc("tex_norm");
    printf("trans: %d Ka:%d Kd:%d has_tex:%d has_tex_norm:%d scale:%d camera:%d light:%d,%d tex:%d tex_norm:%d\n", trans, Ka, Kd, has_tex, has_tex_norm, scale, camera, light_position, light_intense, tex, tex_norm);
}
void PhongShader::set_transform(glm::mat4 transform) {
    glUniformMatrix4fv(trans, 1, false, (GLfloat *)&transform);
}
void PhongShader::set_material(Material *material) {
    glUniform1i(tex, 0);
    glUniform1i(tex_norm, 1);
    if(material == nullptr) {
        glUniform3f(Ka, 0, 0, 0);
        CheckGLError();
        glUniform3f(Kd, 0, 0, 0);
        CheckGLError();
        glUniform1i(has_tex, 0);
        CheckGLError();
    } else {
        if(material->texture != nullptr) {
            CheckGLError();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material->texture->get());
            CheckGLError();
            glUniform1i(has_tex, 1);
            CheckGLError();
        } else {
            glUniform1i(has_tex, 0);
            CheckGLError();
        }
        if(material->texture_normal != nullptr) {
            CheckGLError();
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material->texture_normal->get());
            CheckGLError();
            glUniform1i(has_tex_norm, 1);
            CheckGLError();
        } else {
            glUniform1i(has_tex_norm, 0);
            CheckGLError();
        }
        uniform_vec3(Ka, material->Ka);
        CheckGLError();
        uniform_vec3(Kd, material->Kd);
        CheckGLError();
        uniform_vec3(scale, material->texture_scale);
        CheckGLError();
        uniform_vec3(norm_scale, material->texture_normal_scale);
        CheckGLError();
    }
}
void PhongShader::set_light(glm::vec3 position, glm::vec3 intense) {
    uniform_vec3(light_position, position);
    CheckGLError();
    uniform_vec3(light_intense, intense);
    CheckGLError();
}
void PhongShader::set_camera(glm::vec3 cam) {
    uniform_vec3(camera, cam);
    CheckGLError();
}
void PhongShader::set_depth(GLuint buffer, glm::mat4 transform) {
    glUniform1i(depth_map, 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, buffer);

    glUniformMatrix4fv(light_transform, 1, false, (GLfloat *)&transform);
}

namespace Depth {

static const char *vertex_shader_text = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(position, 1.);
}
)";

static const char *fragment_shader_text = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

void main() {
}
)";

}

DepthShader::DepthShader(): Shader(Depth::vertex_shader_text, Depth::fragment_shader_text) {
    trans = loc("transform");
}
void DepthShader::set_transform(glm::mat4 transform) {
    glUniformMatrix4fv(trans, 1, false, (GLfloat *)&transform);
}

