#include "particle.hpp"

static const char *vertex_shader_text = R"(
#version 330 core
#extension GL_ARB_explicit_uniform_location : enable


layout(location = 0) uniform vec3 center;
layout(location = 1) uniform float v_angle;
layout(location = 2) uniform float particle_size;
layout(location = 3) uniform mat4 transform;

layout(location = 0) in vec3 direction;
layout(location = 1) in vec3 diffuse;
layout(location = 2) in float angle_bias;
layout(location = 3) in float r;
layout(location = 4) in float speed;
layout(location = 5) in float size;

out vec3 o_pos;
out vec3 o_norm;
out vec3 o_diffuse;

void main() {
    float angle = angle_bias + v_angle * speed;
    o_pos = center + vec3(r * cos(angle), 0, r * sin(angle) ) + direction * particle_size * size;
    gl_Position = transform * vec4(o_pos, 1);
    o_norm = direction;
    o_diffuse = diffuse;
}
)";

static const char *fragment_shader_text = R"(
#version 330 core
#extension GL_ARB_explicit_uniform_location : enable

in vec3 o_pos;
in vec3 o_norm;
in vec3 o_diffuse;

layout(location = 4) uniform vec3 camera;
struct LightSource {
    vec3 position;
    vec3 intense;
};
layout(location = 5) uniform LightSource light;

layout(location = 0) out vec4 frag_color;


void main() {
    vec3 i = light.position - o_pos;
    float r = dot(i, i);
    i = normalize(i);
    vec3 v = normalize(camera - o_pos);
    
    vec3 h = normalize((i + v) / 2);
    float theta = dot(i, o_norm);

    vec3 intense = light.intense / r;
    vec3 diffuse = o_diffuse * intense * max(0, theta);

    float alpha = dot(o_norm, h);
    
    vec3 specular = intense * pow(max(0, alpha), 100);
    vec3 ambient = vec3(0, 0, 0);

    frag_color = vec4(diffuse + specular + ambient, 0);
}
)";

ParticleShader::ParticleShader() : Shader(vertex_shader_text, fragment_shader_text) {
    ;
}
void ParticleShader::set_static(glm::vec3 center, float particle_size) {
    glUniform3f(_center, center.x, center.y, center.z);
    glUniform1f(_particle_size, particle_size);
}
void ParticleShader::set_dynamic(glm::mat4 transform, float v_angle, glm::vec3 camera, LightSource l) {
    glUniformMatrix4fv(_transform, 1, false, (GLfloat*)&transform);
    CheckGLError();
    glUniform1f(_v_angle, v_angle);
    CheckGLError();
    glUniform3f(_camera, camera.x, camera.y, camera.z);
    CheckGLError();
    glUniform3f(_light, l.position.x, l.position.y, l.position.z);
    CheckGLError();
    glUniform3f(_light + 1, l.intense.x, l.intense.y, l.intense.z);
    CheckGLError();
}

ParticleSystem::ParticleSystem(float particle_size,
                               glm::vec3 center,
                               float r_lb,
                               float r_rb) 
    : particle_size(particle_size),
      center(center),
      r_lb(r_lb), r_rb(r_rb),
      eng(618)
{
    try {
        shader = std::make_unique<ParticleShader>();
    } catch(std::string msg) {
        printf("Fail to load shader: %s\n", msg.c_str());
        printf("Abort.\n");
        exit(1);
    };
    vbo = vao = 0;
}

double ParticleSystem::_rand(double l, double r)
{
    return std::uniform_real_distribution <> (l, r)(eng);
}
void ParticleSystem::generate(size_t size) {
    vertices.resize(size * 4);
    indices.resize(size * 12);
    double c = cos(PI/6), s = sin(PI/6);
    glm::vec3 d[4] = {
        glm::vec3(0, 1, 0),
        glm::vec3(c, -s, 0),
        glm::vec3(-c * s, -s, c * c),
        glm::vec3(-c * s, -s, -c * c)
    };
    for(int i = 0; i < size; ++i) {
        // r between [r_lb, r_rb]
        float r = sqrt(_rand(r_lb*r_lb, r_rb*r_rb));
        float col_r = _rand(50, 200);
        float col_g = col_r *_rand(0.7, 0.8);
        glm::vec3 diffuse(col_r, col_g, _rand(0, col_r / 10));
        diffuse = diffuse * (1.f / 255);
        float angle_bias = _rand(0, 2 * PI);
        float speed = _rand(0.5, 3);
        float size = _rand(0.5, 2); 

        glm::mat4 transform =
            glm::rotate(glm::rotate(
                            glm::rotate(glm::mat4(1.f), (float)_rand(0, 2 * PI), glm::vec3(0, 0, 1)),
                            (float)_rand(0, 2 * PI), glm::vec3(0, 1, 0)),
                        (float)_rand(0, 2 * PI), glm::vec3(0, 0, 1));
        for(int j = 0; j < 4; ++j) {
            vertices[i << 2 | j] = {(transform * glm::vec4(d[j], 1)).xyz(), diffuse, angle_bias, r, speed, size};
        }
        for(int j = 0; j < 4; ++j) {
            for(int k = 0; k < 3; ++k) {
                indices[i * 12 + (j * 3 + k)] = (j + k) % 4 + i * 4;
            }
        }
    }
    if(vao) {
        glDeleteVertexArrays(1, &vao);
    }
    if(vbo) {
        glDeleteBuffers(1, &vbo);
    }
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(VertexInfo) * vertices.size(),
                 vertices.data(),
                 GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint element_buffer;
    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(GLuint) * indices.size(),
                 indices.data(),
                 GL_STATIC_DRAW);
    
    
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void *)offsetof(VertexInfo, direction));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void *)offsetof(VertexInfo, diffuse));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(
        2, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void *)offsetof(VertexInfo, angle_bias));
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(
        3, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void *)offsetof(VertexInfo, r));
    glEnableVertexAttribArray(3);
    
    glVertexAttribPointer(
        4, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void *)offsetof(VertexInfo, speed));
    glEnableVertexAttribArray(4);
    
    glVertexAttribPointer(
        5, 1, GL_FLOAT, GL_FALSE, sizeof(VertexInfo), (void *)offsetof(VertexInfo, size));
    glEnableVertexAttribArray(5);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
ParticleSystem::~ParticleSystem() {
    if(vao) glDeleteVertexArrays(1, &vao);
    if(vbo) glDeleteBuffers(1, &vbo);
}

void ParticleSystem::draw(size_t count, glm::mat4 transform, glm::vec3 camera, float v_angle, LightSource light) const {
    if(!vao) {
        throw "Particles are not generated";
    }
    if(vertices.size() < 4 * count) {
        throw "Vertices number too large";
    }
    glBindVertexArray(vao);
    shader->use();
    shader->set_static(center, particle_size);
    shader->set_dynamic(transform, v_angle, camera, light);
    glDrawElements(GL_TRIANGLES, (GLsizei)count * 12, GL_UNSIGNED_INT, nullptr);
}
void ParticleSystem::set_particle_size(float size) {
    particle_size = size;
}