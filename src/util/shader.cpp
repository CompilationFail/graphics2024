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

uniform mat4 model;
uniform mat4 vp;

out vec2 o_uv;
out vec3 o_pos;
out vec3 o_norm;

void main() {
    vec4 p = model * vec4(position, 1);
    gl_Position = vp * p;
    o_pos = p.xyz / p.w;
    o_uv = uv;
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
uniform vec3 light_direction;
uniform mat4 light_vp;
uniform sampler2D depth_map;
uniform int has_depth_map;

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

    if(dot(-light_direction, i) < 0.7) {
        diffuse = specular = vec3(0);
    }

    vec3 ambient = color * light_intense * 0.01;

    // ambient = vec3(0,0,0);

    if(has_depth_map != 0) {
        vec4 pos2 = light_vp * vec4(o_pos, 1);
        vec3 pos = pos2.xyz / pos2.w * 0.5 + 0.5;
        if(pos.x >= 0 && pos.x < 1 && pos.y >= 0 && pos.y < 1) {
            float depth = texture(depth_map, vec2(pos.x,pos.y)).r;
            // frag_color = vec4(depth / 3, 0,0, 0);
            // frag_color = vec4((depth - 0.9) * 10);
            // frag_color = vec4((pos.z - 0.9) * 10);
            // return;
            if(pos.z > depth + 1e-3) {
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
    model = loc("model");
    vp = loc("vp");
    Ka = loc("m_ka");
    Kd = loc("m_kd");
    has_tex = loc("has_tex");
    has_tex_norm = loc("has_tex_norm");
    scale = loc("tex_scale");
    norm_scale = loc("tex_norm_scale");
    camera = loc("camera");
    light_position = loc("light_position");
    light_intense = loc("light_intense");
    light_vp = loc("light_vp");
    light_direction = loc("light_direction");
    depth_map = loc("depth_map");
    tex = loc("tex");
    tex_norm = loc("tex_norm");
    has_depth_map = loc("has_depth_map");
    // printf("trans: %d Ka:%d Kd:%d has_tex:%d has_tex_norm:%d scale:%d camera:%d light:%d,%d tex:%d tex_norm:%d\n", vp, Ka, Kd, has_tex, has_tex_norm, scale, camera, light_position, light_intense, tex, tex_norm);
}
void PhongShader::set_mvp(glm::mat4 _model, glm::mat4 _vp) {
    glUniformMatrix4fv(model, 1, false, (GLfloat *)&_model);
    glUniformMatrix4fv(vp, 1, false, (GLfloat *)&_vp);
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
void PhongShader::set_light(glm::vec3 position, glm::vec3 intense, glm::vec3 direction) {
    uniform_vec3(light_position, position);
    CheckGLError();
    uniform_vec3(light_intense, intense);
    CheckGLError();
    uniform_vec3(light_direction, direction);
    CheckGLError();
}
void PhongShader::set_camera(glm::vec3 cam) {
    uniform_vec3(camera, cam);
    CheckGLError();
}
void PhongShader::set_depth(GLuint map, glm::mat4 vp) {
    if(map == 0) {
        glUniform1i(has_depth_map, 0);
    } else {
        glUniform1i(has_depth_map, 1);
        glUniform1i(depth_map, 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, map);

        glUniformMatrix4fv(light_vp, 1, false, (GLfloat *)&vp);
    }
}

namespace Depth {

static const char *vertex_shader_text = R"(
#version 330 core

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

void main() {
    // gl_FragDepth = gl_FragCoord.z;

}
)";

}

DepthShader::DepthShader(): Shader(Depth::vertex_shader_text, Depth::fragment_shader_text) {
    trans = loc("transform");
}
void DepthShader::set_transform(glm::mat4 transform) {
    glUniformMatrix4fv(trans, 1, false, (GLfloat *)&transform);
}

namespace PBR { 
static const char *vertex_shader_text = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 vp;

out vec2 o_uv;
out vec3 o_pos;
out vec3 o_norm;

void main() {
    vec4 p = model * vec4(position, 1);
    gl_Position = vp * p;
    o_pos = p.xyz / p.w;
    o_uv = uv;
    o_norm = normal;
}
)";

static const char *fragment_shader_text = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

in vec2 o_uv;
in vec3 o_pos;
in vec3 o_norm;

uniform mat4 vp;

uniform sampler2D tex;
uniform sampler2D tex_norm;
uniform vec3 tex_scale;
uniform vec3 tex_norm_scale;
uniform int has_tex;
uniform int has_tex_norm;
uniform vec3 camera;

uniform int light_cnt;
uniform vec3 light_position[10];
uniform vec3 light_intense[10];
uniform vec3 light_direction[10];
uniform mat4 light_vp[10];
uniform int light_type[10];

uniform sampler2D depth_map[10];
uniform int has_depth_map;

float F0; // constant for fresnel term

// material parameters
uniform vec3  m_albedo;
uniform float m_metallic;
uniform float m_roughness;
uniform float m_ao;

layout(location = 0) out vec4 frag_color;

vec2 scale_uv(vec2 uv, vec3 scale) {
    return vec2(uv.x / scale.x, uv.y / scale.y);
}

float PI = 3.14159265;

vec3 fresnel(vec3 v, vec3 h, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * vec3(pow(clamp(1.0 - dot(v, h), 0.0, 1.0), 5.0));
}
float D_GGX(vec3 n, vec3 h, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float G_SchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}
float G_Smith(vec3 n, vec3 v, vec3 i, float roughness) {
    return G_SchlickGGX(max(0., dot(n, v)), roughness) * G_SchlickGGX(max(0., dot(n,i)), roughness);
}

vec3 L(vec3 light_position, vec3 light_direction, vec3 light_intense, mat4 light_vp, int light_type, sampler2D depth_map, 
    vec3 n, vec3 pos, vec3 albedo, float metallic, float roughness) {
    
    vec3 i = light_position - pos;
    float r = dot(i, i);
    if(light_type == 2) i = -light_direction;
    i = normalize(i);
    
    vec3 v = normalize(camera - pos);
    vec3 h = normalize(i + v);

    float theta = dot(i, n);
    if(theta <= 0) return vec3(0);
    
    if(has_depth_map != 0) {
        vec4 lpos_w = light_vp * vec4(pos, 1);
        vec3 lpos = (lpos_w.xyz / lpos_w.w + vec3(1)) / 2;
        if(lpos.x >= 0 && lpos.x < 1 && lpos.y >= 0 && lpos.y < 1 && lpos.z >= 0 && lpos.z < 1) {
            float depth = texture(depth_map, lpos.xy).r;
            // return vec3(lpos.x, lpos.y, lpos.z);
            // depth / 10.0);
            float bias = max((1.0 - dot(n, i)) * r, 1) / 3000; 
            if(lpos.z > depth + bias) return vec3(0);
        }
    }
    return albedo;
    // return vec3(1,1,1);
 
    // cone light
    if(light_type == 1 && dot(-light_direction, i) < 0.7) {
        return vec3(0);
    }
    // point light
    vec3 radiance = light_intense / r;

    if(light_type == 2) {
        // directional light
        radiance = light_intense;
    }

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, metallic);

    vec3 F =  fresnel(v, h, F0);
    float D = D_GGX(n, h, roughness);
    float G = G_Smith(n, v, i, roughness);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI * radiance * theta;
    
    vec3 specular = (F * D * G) / (4.0 * max(dot(n, v), 0.0) * max(dot(n, i), 0.0) + 0.0001);
    specular = specular * radiance * theta;

    return specular + diffuse;
}

void main() {
    vec3 albedo = m_albedo;
    float metallic = m_metallic;
    float roughness = m_roughness;
    vec3 normal = o_norm;
    vec3 pos = o_pos;

    if(has_tex == 1) {
        vec3 color = texture(tex, scale_uv(o_uv, tex_scale)).rgb;
        albedo = pow(color, vec3(2.2));
    }
    if(has_tex_norm == 1) {
        normal = texture(tex_norm, scale_uv(o_uv, tex_norm_scale)).rgb;
        normal = normalize(normal * 2 - vec3(1,1,1));
        vec3 tmp;
        tmp.y = normal.z;
        tmp.x = normal.x;
        tmp.z = normal.y;
        normal = tmp;
    }
    // frag_color = vec4(norm, 1);
    // frag_color = vec4(color, 1);
    // frag_color = vec4(vec3(0.5,0.5,0.5)+norm/2,1);
    /*vec4 lpos = vp * vec4(o_pos, 1);
    vec3 tmp = lpos.xyz / lpos.w;
    tmp.z = (tmp.z - 0.98) * 50;
    frag_color = vec4(tmp, 1);
    // o_pos / 5 + vec3(0.5,0.5,0.5), 1);
    return;*/
    
    vec3 ambient = light_intense[0] * m_ao * albedo * 0.002;
    // ambient = vec3(0);
    
    vec3 color = vec3(0);
    int i = 0;
    for(; i < light_cnt; ++i) {
        color += L(light_position[i], light_direction[i], light_intense[i], light_vp[i], light_type[i], depth_map[i],
            normal, pos, albedo, metallic, roughness);
    }

    color += ambient;

    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

    frag_color = vec4(color, 1);
    
    // frag_color = vec4(diffuse, 0);
    // frag_color = vec4(color, 0);
    // frag_color = vec4(specular, 0);
}
)";
}

PBRShader::PBRShader(): Shader(PBR::vertex_shader_text, PBR::fragment_shader_text) {
    model = loc("model");
    vp = loc("vp");
    has_tex = loc("has_tex");
    has_tex_norm = loc("has_tex_norm");
    scale = loc("tex_scale");
    norm_scale = loc("tex_norm_scale");
    camera = loc("camera");
    light_position = loc("light_position");
    light_intense = loc("light_intense");
    light_vp = loc("light_vp");
    light_direction = loc("light_direction");
    light_type = loc("light_type");
    light_cnt = loc("light_cnt");
    depth_map = loc("depth_map");
    tex = loc("tex");
    tex_norm = loc("tex_norm");
    has_depth_map = loc("has_depth_map");
    m_albedo = loc("m_albedo");
    m_metallic = loc("m_metallic");
    m_roughness = loc("m_roughness");
    m_ao = loc("m_ao");
}
void PBRShader::set_mvp(glm::mat4 _model, glm::mat4 _vp) {
    glUniformMatrix4fv(model, 1, false, (GLfloat *)&_model);
    glUniformMatrix4fv(vp, 1, false, (GLfloat *)&_vp);
}
void PBRShader::set_material(Material *material) {
    glUniform1i(tex, 0);
    glUniform1i(tex_norm, 1);
    if(material == nullptr) {
        glUniform1i(has_tex, 0);
        CheckGLError();
        uniform_vec3(m_albedo, glm::vec3(0.f,0,0));
        glUniform1f(m_metallic, 0.5);
        glUniform1f(m_roughness, 0.5);
        glUniform1f(m_ao, 0.1);
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
        uniform_vec3(scale, material->texture_scale);
        CheckGLError();
        uniform_vec3(norm_scale, material->texture_normal_scale);
        CheckGLError();
        uniform_vec3(m_albedo, material -> Kd);
        glUniform1f(m_ao, material->ao);
        glUniform1f(m_metallic, material->metallic);
        glUniform1f(m_roughness, material->roughness);
    }
}
void PBRShader::set_light(std::vector <LightInfo> info) {
    // glm::vec3 position, glm::vec3 intense, glm::vec3 direction) {
    int n = info.size();
    std::vector <glm::vec3> tmp(n);
    glUniform1i(light_cnt, n);
    CheckGLError();
    for(int i = 0; i < n; ++i) tmp[i] = info[i].camera.position;
    glUniform3fv(light_position, n, (GLfloat*)tmp.data());
    CheckGLError();
    for(int i = 0; i < n; ++i) tmp[i] = info[i].intense;
    glUniform3fv(light_intense, n, (GLfloat*)tmp.data());
    CheckGLError();
    for(int i = 0; i < n; ++i) tmp[i] = info[i].camera.dir();
    glUniform3fv(light_direction, n, (GLfloat*)tmp.data());
    CheckGLError();
    std::vector <glm::mat4> tmp2(n);
    for(int i = 0; i < n; ++i) tmp2[i] = info[i].vp();
    glUniformMatrix4fv(light_vp, n, false, (GLfloat*)tmp2.data());
    CheckGLError();
    std::vector <GLint> tmp3(n);
    for(int i = 0; i < n; ++i) tmp3[i] = info[i].type;
    glUniform1iv(light_type, n, (GLint*)tmp3.data());
    CheckGLError();
}
void PBRShader::set_camera(glm::vec3 cam) {
    uniform_vec3(camera, cam);
    CheckGLError();
}
void PBRShader::set_depth(std::vector <GLuint> map) {
    if(map.empty()) {
        glUniform1i(has_depth_map, 0);
    } else {
        glUniform1i(has_depth_map, 1);
        int n = map.size();
        std::vector <int> tmp(n);
        for(int i = 0; i < n; ++i) tmp[i] = i + 2;
        glUniform1iv(depth_map, n, (GLint*)tmp.data());
        for(int i = 0; i < n; ++i) {
            glActiveTexture(GL_TEXTURE0 + 2 + i);
            CheckGLError();
            glBindTexture(GL_TEXTURE_2D, map[i]);
            CheckGLError();
        }
    }
}

namespace SSDO_text { 
static const char *vert = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 vp;

out vec2 o_uv;
out vec3 o_pos;
out vec3 o_norm;

void main() {
    vec4 p = model * vec4(position, 1);
    gl_Position = vp * p;
    o_pos = p.xyz / p.w;
    o_uv = uv;
    o_norm = normal;
}
)";

static const char *frag1 = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

in vec2 o_uv;
in vec3 o_pos;
in vec3 o_norm;
uniform mat4 vp;
uniform sampler2D tex;
uniform sampler2D tex_norm;
uniform vec3 tex_scale;
uniform vec3 tex_norm_scale;
uniform int has_tex;
uniform int has_tex_norm;
uniform vec3 camera;
uniform int light_cnt;
uniform vec3 light_position[10];
uniform vec3 light_intense[10];
uniform vec3 light_direction[10];
uniform mat4 light_vp[10];
uniform int light_type[10];
uniform sampler2D depth_map[10];
uniform int has_depth_map;
float F0; // constant for fresnel term
// material parameters
uniform vec3  m_albedo;
uniform float m_metallic;
uniform float m_roughness;
uniform float m_ao;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_depth;

float PI = 3.14159265;
vec2 scale_uv(vec2 uv, vec3 scale) {
    return vec2(uv.x / scale.x, uv.y / scale.y);
}
vec3 fresnel(vec3 v, vec3 h, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * vec3(pow(clamp(1.0 - dot(v, h), 0.0, 1.0), 5.0));
}
float D_GGX(vec3 n, vec3 h, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float G_SchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}
float G_Smith(vec3 n, vec3 v, vec3 i, float roughness) {
    return G_SchlickGGX(max(0., dot(n, v)), roughness) * G_SchlickGGX(max(0., dot(n,i)), roughness);
}

vec3 L(vec3 light_position, vec3 light_direction, vec3 light_intense, mat4 light_vp, int light_type, sampler2D depth_map, 
    vec3 n, vec3 pos, vec3 albedo, float metallic, float roughness) {
    
    vec3 i = light_position - pos;
    float r = dot(i, i);
    if(light_type == 2) i = -light_direction;
    i = normalize(i);
    
    vec3 v = normalize(camera - pos);
    vec3 h = normalize(i + v);

    float theta = dot(i, n);
    if(theta <= 0) return vec3(0);
    
    if(has_depth_map != 0) {
        vec4 lpos_w = light_vp * vec4(pos, 1);
        vec3 lpos = (lpos_w.xyz / lpos_w.w + vec3(1)) / 2;
        if(lpos.x >= 0 && lpos.x < 1 && lpos.y >= 0 && lpos.y < 1 && lpos.z >= 0 && lpos.z < 1) {
            float depth = texture(depth_map, lpos.xy).r;
            // return vec3(lpos.x, lpos.y, lpos.z);
            // depth / 10.0);
            float bias = max((1.0 - dot(n, i)) * sqrt(r), 1) * 1e-4; 
            if(lpos.z > depth + bias) return vec3(0);
        }
    }
 
    // cone light
    if(light_type == 1 && dot(-light_direction, i) < 0.7) {
        return vec3(0);
    }
    // point light
    vec3 radiance = light_intense / r;

    if(light_type == 2) {
        // directional light
        radiance = light_intense;
    }

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, metallic);

    vec3 F =  fresnel(v, h, F0);
    float D = D_GGX(n, h, roughness);
    float G = G_Smith(n, v, i, roughness);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI * radiance * theta;
    
    vec3 specular = (F * D * G) / (4.0 * max(dot(n, v), 0.0) * max(dot(n, i), 0.0) + 0.0001);
    specular = specular * radiance * theta;

    return specular + diffuse;
}

void main() {
    vec3 albedo = m_albedo;
    float metallic = m_metallic;
    float roughness = m_roughness;
    vec3 normal = o_norm;
    vec3 pos = o_pos;

    if(has_tex == 1) {
        vec3 color = texture(tex, scale_uv(o_uv, tex_scale)).rgb;
        albedo = pow(color, vec3(2.2));
    }
    if(has_tex_norm == 1) {
        normal = texture(tex_norm, scale_uv(o_uv, tex_norm_scale)).rgb;
        normal = normalize(normal * 2 - vec3(1,1,1));
        vec3 tmp;
        tmp.y = normal.z;
        tmp.x = normal.x;
        tmp.z = normal.y;
        normal = tmp;
    }
    
    frag_depth = (normal + vec3(1)) / 2;
    
    vec3 color = vec3(0);
    int i = 0;
    for(; i < light_cnt; ++i) {
        color += L(light_position[i], light_direction[i], light_intense[i], light_vp[i], light_type[i], depth_map[i],
            normal, pos, albedo, metallic, roughness);
    }

    frag_color = color;
}
)";

static const char *frag2 = R"(
#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

in vec2 o_uv;
in vec3 o_pos;
in vec3 o_norm;

uniform mat4 vp, vp_inv;

uniform sampler2D tex;
uniform sampler2D tex_norm;
uniform vec3 tex_scale;
uniform vec3 tex_norm_scale;
uniform int has_tex;
uniform int has_tex_norm;
uniform vec3 camera;

uniform int light_cnt;
uniform vec3 light_position[10];
uniform vec3 light_intense[10];
uniform vec3 light_direction[10];
uniform mat4 light_vp[10];
uniform int light_type[10];

uniform sampler2D depth_map[10];
uniform int has_depth_map;

uniform sampler2D geo_depth, geo_normal, geo_color;

float F0; // constant for fresnel term

// material parameters
uniform vec3  m_albedo;
uniform float m_metallic;
uniform float m_roughness;
uniform float m_ao;

// out vec4 frag_color[2];
out vec4 frag_color;

vec2 scale_uv(vec2 uv, vec3 scale) {
    return vec2(uv.x / scale.x, uv.y / scale.y);
}
vec3 decw(vec4 p) {
    return p.xyz / p.w;
}
vec3 world2screen(vec3 p) {
    return (decw(vp * vec4(p, 1)) + vec3(1)) / 2;
}
vec3 screen2world(vec3 p) {
    return decw(vp_inv * vec4(p * 2 - vec3(1), 1));
}

float PI = 3.14159265;

vec3 fresnel(vec3 v, vec3 h, vec3 F0) {
    return F0 + (vec3(1.0) - F0) * vec3(pow(clamp(1.0 - dot(v, h), 0.0, 1.0), 5.0));
}
float D_GGX(vec3 n, vec3 h, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float G_SchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}
float G_Smith(vec3 n, vec3 v, vec3 i, float roughness) {
    return G_SchlickGGX(max(0., dot(n, v)), roughness) * G_SchlickGGX(max(0., dot(n,i)), roughness);
}

vec3 L(vec3 light_position, vec3 light_normal, vec3 light_intense, vec3 n, vec3 pos, vec3 albedo, float metallic, float roughness) {
    // pixel rect light
    vec3 i = light_position - pos;
    float r = dot(i, i);
    i = normalize(i);
    
    vec3 v = normalize(camera - pos);
    vec3 h = normalize(i + v);

    float theta = dot(i, n);
    if(theta <= 0) return vec3(0);

    float theta2 = dot(light_normal, -i);
    if(theta2 <= 0) return vec3(0);
    
    vec3 radiance = light_intense / r * theta2;

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, metallic);

    vec3 F =  fresnel(v, h, F0);
    float D = D_GGX(n, h, roughness);
    float G = G_Smith(n, v, i, roughness);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI * radiance * theta;
    
    vec3 specular = (F * D * G) / (4.0 * max(dot(n, v), 0.0) * max(dot(n, i), 0.0) + 0.0001);
    specular = specular * radiance * theta;

    float As = 1000;
    return (specular + diffuse) * As;
}

float random (vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123);
}

int N = 50;
vec3 sampleHemisphereCosine(vec3 normal, vec2 seed1, vec2 seed2) {
    // Generate two random numbers
    float u1 = random(seed1);
    float u2 = random(seed2);

    // Convert to spherical coordinates
    float theta = acos(sqrt(u1));
    float phi = 2.0 * PI * u2;

    // Convert to Cartesian coordinates
    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    // Create a sample direction in local space
    vec3 sampleDir = vec3(x, y, z);

    // Transform the sample direction to world space using the normal
    vec3 up = vec3(0.0, 0.0, 1.0); // Or any other vector that is not parallel to the normal
    if (abs(normal.z) > 0.999) {
        up = vec3(1.0, 0.0, 0.0);
    }
    vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);
    vec3 worldSampleDir = tangent * sampleDir.x + bitangent * sampleDir.y + normal * sampleDir.z;

    return normalize(worldSampleDir);
}

void main() {
    vec3 albedo = m_albedo;
    float metallic = m_metallic;
    float roughness = m_roughness;
    vec3 normal = o_norm;
    vec3 pos = o_pos;
    vec3 pos_screen = world2screen(pos);
    
    // normal = texture(geo_normal, pos_screen.xy).rgb * 2 - 1;
    // frag_color = vec4((normal + 1) / 2, 1);
    // frag_color = vec4((pos_screen.z - 0.8) * 3);
    // pos = screen2world(pos_screen);
    // frag_color = vec4(pos / 3 + 0.5, 1);
    // return;
    

    if(texture(geo_depth, pos_screen.xy).r * (1 + 1e-3) < pos_screen.z) {
        frag_color = vec4(pos_screen.z / 4);
        return;
    }

    if(has_tex == 1) {
        vec3 color = texture(tex, scale_uv(o_uv, tex_scale)).rgb;
        albedo = pow(color, vec3(2.2));
    }
    if(has_tex_norm == 1) {
        normal = texture(tex_norm, scale_uv(o_uv, tex_norm_scale)).rgb;
        normal = normalize(normal * 2 - vec3(1,1,1));
        vec3 tmp;
        tmp.y = normal.z;
        tmp.x = normal.x;
        tmp.z = normal.y;
        normal = tmp;
    }
    
    vec3 di = texture(geo_color, pos_screen.xy).rgb; // Direct Illumination
    vec3 ind = vec3(0);
    int i = 0;
    int rmax = 4;
    for(i = 0; i < N; ++i) {
        vec3 dir = sampleHemisphereCosine(normal, pos_screen.xy + vec2(i + 1, 0), pos_screen.xy + vec2(0, i + 1));
        if(dot(dir, normal) < 1e-4) continue;
        float r = 0.01 + (rmax - 0.01) * max(0, random(pos_screen.xy + vec2(i + 1, i + 1)));
        vec3 p = pos + r * dir;
        vec3 p_screen = world2screen(p);
        if(p_screen.x < 0 || p_screen.x >= 1 || p_screen.y < 0 || p_screen.y >= 1) 
            continue;
        float z = texture(geo_depth, p_screen.xy).r;
        // float bias = max(length(p), 1) / 3000; 
        if(z + 1e-3 < p_screen.z) {
            p_screen.z = z;
            p = screen2world(p);
            vec3 p_normal = texture(geo_normal, p_screen.xy).rgb * 2 - 1;
            vec3 p_color = texture(geo_color, p_screen.xy).rgb;
            // ind = N * (p / 3 + 0.5);
            ind += L(p, p_normal, p_color, normal, pos, albedo, metallic, roughness);
            // = p_color * N;
        }
    }

    ind /= N;

    vec3 color =  ind * 4;

    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

    frag_color = vec4(color, 1);
}
)";
}

SSDO::SSDO(int render_pass): Shader(SSDO_text::vert, render_pass == 1 ? SSDO_text::frag1 : SSDO_text::frag2) {
    model = loc("model");
    vp = loc("vp");
    vp_inv = loc("vp_inv");
    has_tex = loc("has_tex");
    has_tex_norm = loc("has_tex_norm");
    scale = loc("tex_scale");
    norm_scale = loc("tex_norm_scale");
    camera = loc("camera");
    light_position = loc("light_position");
    light_intense = loc("light_intense");
    light_vp = loc("light_vp");
    light_direction = loc("light_direction");
    light_type = loc("light_type");
    light_cnt = loc("light_cnt");
    depth_map = loc("depth_map");
    tex = loc("tex");
    tex_norm = loc("tex_norm");
    has_depth_map = loc("has_depth_map");
    m_albedo = loc("m_albedo");
    m_metallic = loc("m_metallic");
    m_roughness = loc("m_roughness");
    m_ao = loc("m_ao");
    normal = loc("geo_normal");
    depth = loc("geo_depth");
    color = loc("geo_color");
}
void SSDO::set_mvp(glm::mat4 _model, glm::mat4 _vp) {
    glUniformMatrix4fv(model, 1, false, (GLfloat *)&_model);
    glUniformMatrix4fv(vp, 1, false, (GLfloat *)&_vp);
    auto inv = glm::inverse(_vp);
    glUniformMatrix4fv(vp_inv, 1, false, (GLfloat *)&inv);
}
void SSDO::set_material(Material *material) {
    glUniform1i(tex, 0);
    glUniform1i(tex_norm, 1);
    if(material == nullptr) {
        glUniform1i(has_tex, 0);
        CheckGLError();
        uniform_vec3(m_albedo, glm::vec3(0.f,0,0));
        glUniform1f(m_metallic, 0.5);
        glUniform1f(m_roughness, 0.5);
        glUniform1f(m_ao, 0.1);
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
        uniform_vec3(scale, material->texture_scale);
        CheckGLError();
        uniform_vec3(norm_scale, material->texture_normal_scale);
        CheckGLError();
        uniform_vec3(m_albedo, material -> Kd);
        glUniform1f(m_ao, material->ao);
        glUniform1f(m_metallic, material->metallic);
        glUniform1f(m_roughness, material->roughness);
    }
}
void SSDO::set_light(std::vector <LightInfo> info) {
    // glm::vec3 position, glm::vec3 intense, glm::vec3 direction) {
    int n = info.size();
    std::vector <glm::vec3> tmp(n);
    glUniform1i(light_cnt, n);
    CheckGLError();
    for(int i = 0; i < n; ++i) tmp[i] = info[i].camera.position;
    glUniform3fv(light_position, n, (GLfloat*)tmp.data());
    CheckGLError();
    for(int i = 0; i < n; ++i) tmp[i] = info[i].intense;
    glUniform3fv(light_intense, n, (GLfloat*)tmp.data());
    CheckGLError();
    for(int i = 0; i < n; ++i) tmp[i] = info[i].camera.dir();
    glUniform3fv(light_direction, n, (GLfloat*)tmp.data());
    CheckGLError();
    std::vector <glm::mat4> tmp2(n);
    for(int i = 0; i < n; ++i) tmp2[i] = info[i].vp();
    glUniformMatrix4fv(light_vp, n, false, (GLfloat*)tmp2.data());
    CheckGLError();
    std::vector <GLint> tmp3(n);
    for(int i = 0; i < n; ++i) tmp3[i] = info[i].type;
    glUniform1iv(light_type, n, (GLint*)tmp3.data());
    CheckGLError();
}
void SSDO::set_camera(glm::vec3 cam) {
    uniform_vec3(camera, cam);
    CheckGLError();
}
void SSDO::set_depth(std::vector <GLuint> map) {
    if(map.empty()) {
        glUniform1i(has_depth_map, 0);
    } else {
        glUniform1i(has_depth_map, 1);
        int n = map.size();
        std::vector <int> tmp(n);
        for(int i = 0; i < n; ++i) tmp[i] = i + 5;
        glUniform1iv(depth_map, n, (GLint*)tmp.data());
        for(int i = 0; i < n; ++i) {
            glActiveTexture(GL_TEXTURE0 + 5 + i);
            CheckGLError();
            glBindTexture(GL_TEXTURE_2D, map[i]);
            CheckGLError();
        }
    }
}
void SSDO::set_geo(GLuint d, GLuint n, GLuint c) {
    glUniform1i(depth, 2);
    glUniform1i(normal, 3);
    glUniform1i(color, 4);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, d);
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, n);
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, c);
}