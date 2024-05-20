#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "common.hpp"

class Texture2D {
public:
  Texture2D(const Path &);
  Texture2D(uint8_t *data,
            GLenum data_type,
            int width,
            int height,
            int channels);

  Texture2D(uint8_t *data,
            GLenum data_type,
            int width,
            int height,
            GLenum internal_format,
            GLenum format);

  ~Texture2D();

  GLuint get() const;

  int width() const;
  int height() const;

private:
  GLuint _tex_id;
  int _width, _height;

  void init(uint8_t *data,
            GLenum data_type,
            int width,
            int height,
            int channels);

  void init(uint8_t *data,
            GLenum data_type,
            int width,
            int height,
            GLenum internal_format,
            GLenum format);
};
