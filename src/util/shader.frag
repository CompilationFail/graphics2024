#version 330 core
// #extension GL_ARB_explicit_uniform_location : enable

in vec2 o_uv;
in vec3 o_pos;
in vec3 o_norm;

uniform sampler2D tex;
uniform sampler2D tex_norm;
uniform vec3 tex_scale;
uniform vec3 tex_norm_scale;
uniform int has_tex;
uniform int has_tex_norm;
uniform vec3 camera;

uniform vec3 light_position;
uniform vec3 light_intense;
uniform mat4 light_transform;
uniform sampler2D depth_map;
uniform int has_depth_map;

float F0; // constant for fresnel term

// material parameters
uniform vec3  m_albedo;
uniform float m_metallic;
uniform float m_roughness;
uniform float m_ao;

layout(location = 0) out vec4 frag_color;

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

vec3 L(vec3 light_pos, vec3 light_intense, vec3 n, vec3 pos, vec3 albedo, float metallic, float roughness) {
    vec3 i = light_position - pos;
    float r = dot(i, i);
    i = normalize(i);
    vec3 v = normalize(camera - pos);
    vec3 h = normalize(i + v);

    float theta = dot(i, n);
    if(theta <= 0) return vec3(0);
    
    if(has_depth_map != 0) {
        vec4 lpos_w = light_transform * vec4(o_pos, 1);
        vec3 lpos = lpos_w.xyz / lpos_w.w;
        vec2 uv = (lpos.xy + vec2(1)) / 2;
        if(uv.x >= 0 && uv.x < 1 && uv.y >= 0 && uv.y < 1) {
            float depth = texture(depth_map, uv).r;
            if(lpos.z > depth + 1e-3) return vec3(0);
        }
    }
    
    vec3 radiance = light_intense / r;

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
        vec3 color = texture(tex, vec2(o_uv / tex_scale)).rgb;
        albedo = pow(color, 2.2);
    }
    if(has_tex_norm == 1) {
        normal = texture(tex_norm, vec2(o_uv / tex_norm_scale)).rgb;
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
    // frag_color = vec4(o_pos / 5 + vec3(0.5,0.5,0.5), 1);
    // return;
    
    vec3 ambient = light_intense * m_ao * albedo * 0.02;
    
    vec3 color = L(light_position, light_intense, normal, pos, albedo, metallic, roughness) + ambient;

    // Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

    frag_color = vec4(color, 1);
    
    // frag_color = vec4(diffuse, 0);
    // frag_color = vec4(color, 0);
    // frag_color = vec4(specular, 0);
}
