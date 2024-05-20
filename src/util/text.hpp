#include "common.hpp"
#include "shader.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H 

struct Character {
    bool available;
    glm::ivec2   size;       // Size of glyph
    glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
    GLuint advance;    // Offset to advance to next glyph
    glm::vec2 uv0, uv1;
};

class TextRenderer {
    Character characters[128];
    GLuint shader_program;
public:
    TextRenderer(const Path &);
    void render(const std::string &);
};
