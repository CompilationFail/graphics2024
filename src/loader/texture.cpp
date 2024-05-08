#include "texture.hpp"
#include <sstream>
#include <stb_image.h>
#include <iostream>

Texture2D::Texture2D(const Path &path) {
  warn(0, "loading texture from image file: %s", path.u8string().c_str());
  // auto full_path = Data::resolve(name).string();
  stbi_set_flip_vertically_on_load(true);
  int width, height, channels;
  unsigned char *data =
      stbi_load(path.u8string().c_str(), &width, &height, &channels, 0);
  // std::cout << stbi_failure_reason() <<std::endl;
  if (!data) {
    std::stringstream ss;
    ss << "failed to load image " << path << " :" << stbi_failure_reason() << std::endl;
    throw ss.str();
  }
  init(data, GL_UNSIGNED_BYTE, width, height, channels);
  stbi_image_free(data);
}

void Texture2D::init(uint8_t *data,
                     GLenum data_type,
                     int width,
                     int height,
                     GLenum internal_format,
                     GLenum format) {
  _width = width;
  _height = height;

  glGenTextures(1, &_tex_id);
  if(_tex_id == 0) {
    warn(2, "[ERROR] Fail to generate texture2D");
  }
  glBindTexture(GL_TEXTURE_2D, _tex_id);
  CheckGLError();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  CheckGLError();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  CheckGLError();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  CheckGLError();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  CheckGLError();
  /*
    NOTE: default 4-byte alignment for each row of data.
      3-channel image may not be aligned themself.
  */
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,
               _width,
               _height,
               0,
               format,
               data_type,
               data);
  CheckGLError();
  glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture2D::init(uint8_t *data,
                     GLenum data_type,
                     int width,
                     int height,
                     int channels) {
  GLenum format = GL_RGBA;
  if (channels == 1) {
    format = GL_R;
  }
  if (channels == 2) {
    format = GL_RG;
  }
  if (channels == 3) {
    format = GL_RGB;
  }
  if (channels == 4) {
    format = GL_RGBA;
  }
  init(data, data_type, width, height, format, format);
}

/*
Texture2D::Texture2D(uint8_t *data,
                     GLenum data_type,
                     int width,
                     int height,
                     int channels,
                     TextureSettings *settings) {
  init(data, data_type, width, height, channels, settings);
}

Texture2D::Texture2D(uint8_t *data,
                     GLenum data_type,
                     int width,
                     int height,
                     GLenum internal_format,
                     GLenum format,
                     TextureSettings *settings) {
  init(data, data_type, width, height, internal_format, format, settings);
}*/

Texture2D::~Texture2D() {
  glDeleteTextures(1, &_tex_id);
}

GLuint Texture2D::get() const {
  return _tex_id;
}

int Texture2D::width() const {
  return _width;
}

int Texture2D::height() const {
  return _height;
}
