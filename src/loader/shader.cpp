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

Shader::~Shader() {
    printf("shader destroyed\n");
    glDeleteShader(_program);
}


namespace Phong {

static const char *vertex_shader_text = R"(
#version 330 core
#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) uniform mat4 transform;

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
#extension GL_ARB_explicit_uniform_location : enable

in vec2 o_uv;
in vec3 o_pos;
in vec3 o_norm;

uniform sampler2D tex;
layout(location = 1) uniform vec3 m_ka;
layout(location = 2) uniform vec3 m_kd;
layout(location = 3) uniform vec3 tex_scale;
layout(location = 4) uniform int m_type;
layout(location = 5) uniform vec3 camera;
struct LightSource {
    vec3 position;
    vec3 intense;
};
layout(location = 6) uniform LightSource light;

layout(location = 0) out vec4 frag_color;


void main() {
    vec3 color;
    if(m_type == 1) {
        color = vec3(texture(tex, vec2(o_uv.x / tex_scale.x, o_uv.y / tex_scale.y)));
    } else {
        color = m_kd;
    }
    vec3 i = light.position - o_pos;
    float r = dot(i, i);
    i = normalize(i);
    vec3 v = normalize(camera - o_pos);
    // frag_color = vec4((v+vec3(1,1,1))/2, 0);
    // frag_color = vec4((camera + vec3(10,10,10))/20, 0);
    // frag_color = vec4(dot(i,i), 0, 0, 0);
    // return;
    vec3 h = normalize((i + v) / 2);
    float theta = dot(i, o_norm);

    /*if(dot(o_norm, v) < 0) {
        frag_color = vec4(0, 0, 0, 0);
        return;
    }*/

    vec3 intense = light.intense / r;
    vec3 diffuse = color * intense * max(0, theta);

    float alpha = dot(o_norm, h);
    
    vec3 specular = intense * pow(max(0, alpha), 100);
    if(alpha < 0) {
        specular = vec3(0,0,0);
    }

    vec3 ambient = color * light.intense * m_ka * 1e-6;

    // frag_color = vec4(diffuse, 0);
    frag_color = vec4(diffuse + specular + ambient, 0);
    // frag_color = vec4(specular, 0);
}
)";

}


PhongShader::PhongShader(): Shader(Phong::vertex_shader_text, Phong::fragment_shader_text) {
    /*trans = glGetUniformLocation(_program, "transform");
    CheckGLError();
    Ka = glGetUniformLocation(_program, "m_ka");
    CheckGLError();
    Kd = glGetUniformLocation(_program, "m_kd");
    CheckGLError();
    type = glGetUniformLocation(_program, "m_type");
    CheckGLError();
    scale = glGetUniformLocation(_program, "tex_scale");
    printf("uniforms: %d %d %d %d\n", trans, Ka, Kd, type);
    if(trans == -1 || Ka == -1 || Kd == -1 || type == -1) {
        warn(2, "[ERROR] Fail to get uniform location");
    }*/
}
void PhongShader::set_transform(glm::mat4 transform) {
    glUniformMatrix4fv(trans, 1, false, (GLfloat *)&transform);
}
void PhongShader::set_material(Material *material) {
    if(material == nullptr) {
        glUniform3f(Ka, 0, 0, 0);
        CheckGLError();
        glUniform3f(Kd, 0, 0, 0);
        CheckGLError();
        glUniform1i(type, 0);
        CheckGLError();
    } else {
        if(material->texture != nullptr) {
            CheckGLError();
            glBindTexture(GL_TEXTURE_2D, material->texture->get());
            CheckGLError();
            glUniform1i(type, 1);
            CheckGLError();
        } else {
            glUniform1i(type, 0);
            CheckGLError();
        }
        glUniform3f(Ka, material->Ka.x, material->Ka.y, material->Ka.z);
        CheckGLError();
        glUniform3f(Kd, material->Kd.x, material->Kd.y, material->Kd.z);
        CheckGLError();
        glUniform3f(scale, material->texture_scale.x, material->texture_scale.y, material->texture_scale.z);
        CheckGLError();
    }
}
void PhongShader::set_light(LightSource l) {
    glUniform3f(light, l.position.x, l.position.y, l.position.z);
    CheckGLError();
    glUniform3f(light + 1, l.intense.x, l.intense.y, l.intense.z);
    CheckGLError();
}
void PhongShader::set_camera(glm::vec3 cam) {
    glUniform3f(camera, cam.x, cam.y, cam.z);
    CheckGLError();
}