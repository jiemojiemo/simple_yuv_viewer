//
// Created by user on 1/6/23.
//

#ifndef SIMPLE_YUV_VIEWER_MY_YUV_LOADER_H
#define SIMPLE_YUV_VIEWER_MY_YUV_LOADER_H
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif
#include <SDL2/SDL.h>
#if defined(__cplusplus)
};
#endif

#include <fstream>
#include <string>

enum class YUVFormat { kYUV420 = 0, kYUV444, kYUV422 };

class YUVFileLoader {
public:
  ~YUVFileLoader() { SDL_DestroyTexture(texture_); }
  bool loadFile(const std::string &file_path) {
    std::ifstream in(file_path, std::ios::in | std::ios::binary);
    if (in) {
      in.seekg(0, std::ios::end);
      file_contents_.resize(in.tellg());
      in.seekg(0, std::ios::beg);
      in.read(&file_contents_[0], file_contents_.size());
      in.close();
      return true;
    }
    return false;
  }

  SDL_Texture *updateTexture(YUVFormat format, size_t width, size_t height,
                             SDL_Renderer *renderer) {

    createTextureIfNeed(width, height, renderer);
    updateTextureYUV420(width, height);

    return texture_;
  }

private:
  void createTexture(size_t width, size_t height, SDL_Renderer *renderer) {
    if (texture_ != nullptr) {
      SDL_DestroyTexture(texture_);
      texture_ = nullptr;
    }

    texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
                                 SDL_TEXTUREACCESS_STREAMING, width, height);
  }

  std::pair<int, int> getTextureSize(SDL_Texture *texture) {
    auto tex_width = 0;
    auto tex_height = 0;
    SDL_QueryTexture(texture, NULL, NULL, &tex_width, &tex_height);
    return {tex_width, tex_height};
  }

  void createTextureIfNeed(size_t width, size_t height,
                           SDL_Renderer *renderer)
  {
    if (texture_ == nullptr) {
      createTexture(width, height, renderer);
    } else {
      auto [tex_width, tex_height] = getTextureSize(texture_);
      if (width != tex_width || height != tex_height) {
        createTexture(width, height, renderer);
      }
    }
  }

  void updateTextureYUV420(size_t width, size_t height)
  {
    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto *y_plane = yuv_data;
    size_t y_stride = width;

    auto *u_plane = yuv_data + width * height;
    size_t u_stride = width / 2;

    auto *v_plane = u_plane + (width * height) / 4;
    size_t v_stride = width / 2;

    SDL_UpdateYUVTexture(
        texture_, // the texture to update
        nullptr, // a pointer to the rectangle of pixels to update, or NULL to
                  // update the entire texture
        y_plane, // the raw pixel data for the Y plane
        y_stride, // the number of bytes between rows of pixel data for the Y
                  // plane
        u_plane,  // the raw pixel data for the U plane
        u_stride, // the number of bytes between rows of pixel data for the U
                  // plane
        v_plane,  // the raw pixel data for the V plane
        v_stride  // the number of bytes between rows of pixel data for the V
                  // plane
    );
  }

  std::string file_contents_;
  SDL_Texture *texture_{nullptr};
};

#endif // SIMPLE_YUV_VIEWER_MY_YUV_LOADER_H
