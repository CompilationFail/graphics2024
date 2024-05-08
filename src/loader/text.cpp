#include <iostream>
#include "text.hpp"

static const char *vertex_shader = R"(
#version 330 core

layout(location = 0) in vec4 data;

out vec2 uv;

void main() {
  gl_Position = vec4(data.xy, 0., 1.);
  uv = data.zw;
}
)";

static const char *fragment_shader = R"(
#version 330 core

in vec2 uv;
uniform sampler2D tex;
uniform vec3 baseColor;
out vec3 color;

void main() {
  color = baseColor * texture(tex, uv);
}
)";


TextRenderer::TextRenderer(const Path &path) {

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    
    FT_Library lib;
    if (FT_Init_FreeType(&lib))
    {
        printf("[ERROR] FreeType: Cannot init library\n");
        return;
    }


    shader_program = prepare_shader(vertex_shader, fragment_shader);

    FT_Face face;
    
    if(FT_New_Face(lib, path.u8string().c_str(), 0, &face)) {
        std::cout << "[ERROR] FreeType: cannot load font " << path << std::endl;
    }
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph: ascii(" << c << ")" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        characters[c] = std::make_unique <Character> (
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x);
    }
}

void TextRenderer::render(const std::string &) {
    ;
}