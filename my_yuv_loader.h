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

enum class YUVFormat { kYUV420 = 0, kYUYV422, kUYVY422, kNV12, kYVYU };

class YUVFileLoader {
public:
  ~YUVFileLoader() { SDL_DestroyTexture(texture_); }
  bool loadFile(const std::string &file_path) {
    std::ifstream in(file_path, std::ios::in | std::ios::binary);
    if (in) {
      filepath_ = file_path;
      in.seekg(0, std::ios::end);
      file_contents_.resize(in.tellg());
      in.seekg(0, std::ios::beg);
      in.read(&file_contents_[0], file_contents_.size());
      in.close();
      return true;
    } else {
      filepath_ = "";
      file_contents_.clear();
    }
    return false;
  }

  const std::string &getLoadFilePath() const { return filepath_; }

  SDL_Texture *updateTexture(YUVFormat format, size_t width, size_t height,
                             SDL_Renderer *renderer) {

    createTextureIfNeed(format, width, height, renderer);

    switch (format) {
    case YUVFormat::kYUV420:
      updateTextureYUV420(width, height);
      break;
    case YUVFormat::kYUYV422:
    case YUVFormat::kYVYU:
    case YUVFormat::kUYVY422:
      updateTexturePacketYUV422(width);
      break;
    case YUVFormat::kNV12:
      updateTextureNV12(width, height);
      break;
    }

    return texture_;
  }

private:
  void createTexture(YUVFormat format, size_t width, size_t height,
                     SDL_Renderer *renderer) {
    if (texture_ != nullptr) {
      SDL_DestroyTexture(texture_);
      texture_ = nullptr;
    }

    auto sdl_pixel_format = SDL_PIXELFORMAT_IYUV;

    if (format == YUVFormat::kYUV420) {
      sdl_pixel_format = SDL_PIXELFORMAT_IYUV;
    } else if (format == YUVFormat::kNV12) {
      sdl_pixel_format = SDL_PIXELFORMAT_NV12;
    } else if (format == YUVFormat::kYUYV422) {
      sdl_pixel_format = SDL_PIXELFORMAT_YUY2;
    } else if (format == YUVFormat::kUYVY422) {
      sdl_pixel_format = SDL_PIXELFORMAT_UYVY;
    } else if(format == YUVFormat::kYVYU){
      sdl_pixel_format = SDL_PIXELFORMAT_YVYU;
    }

    texture_ = SDL_CreateTexture(renderer, sdl_pixel_format,
                                 SDL_TEXTUREACCESS_STREAMING, width, height);
  }

  void createTextureIfNeed(YUVFormat format, size_t width, size_t height,
                           SDL_Renderer *renderer) {
    if (texture_ == nullptr) {
      createTexture(format, width, height, renderer);
    } else {
      auto tex_width = 0;
      auto tex_height = 0;
      Uint32 tex_format = 0;
      SDL_QueryTexture(texture_, &tex_format, NULL, &tex_width, &tex_height);
      if (width != tex_width || height != tex_height ||
          tex_format != Uint32(format)) {
        createTexture(format, width, height, renderer);
      }
    }
  }

  void updateTextureNV12(size_t width, size_t height) {
    if (file_contents_.empty()) {
      return;
    }

    auto content_size = file_contents_.size();
    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto *y_plane = yuv_data;
    size_t y_stride = width;

    auto uv_offset = width * height;
    uv_offset = (uv_offset > content_size) ? (content_size) : (uv_offset);
    auto *uv_plane = yuv_data + uv_offset;
    size_t uv_stride = width;

    SDL_UpdateNVTexture(texture_, nullptr, y_plane, y_stride, uv_plane,
                        uv_stride);
  }

  void updateTexturePacketYUV422(size_t width)
  {
    if (file_contents_.empty()) {
      return;
    }

    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto pitch = 2 * width;
    SDL_UpdateTexture(texture_, nullptr, yuv_data, pitch);
  }

  void updateTextureYUV444(size_t width, size_t height) {
    if (file_contents_.empty()) {
      return;
    }

    auto content_size = file_contents_.size();
    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto *y_plane = yuv_data;
    size_t y_stride = width;

    auto u_offset = width * height;
    u_offset = (u_offset > content_size) ? (content_size) : (u_offset);
    auto *u_plane = yuv_data + u_offset;
    size_t u_stride = width;

    auto v_offset = u_offset + (width * height);
    v_offset = (v_offset > content_size) ? (content_size) : (v_offset);
    auto *v_plane = yuv_data + v_offset;
    size_t v_stride = width;

    SDL_UpdateYUVTexture(
        texture_, // the texture to update
        nullptr,  // a pointer to the rectangle of pixels to update, or NULL to
                  // update the entire texture
        y_plane,  // the raw pixel data for the Y plane
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

  void updateTextureYUV420(size_t width, size_t height) {
    if (file_contents_.empty()) {
      return;
    }

    auto content_size = file_contents_.size();
    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto *y_plane = yuv_data;
    size_t y_stride = width;

    auto u_offset = width * height;
    u_offset = (u_offset > content_size) ? (content_size) : (u_offset);
    auto *u_plane = yuv_data + u_offset;
    size_t u_stride = width / 2;

    auto v_offset = u_offset + (width * height) / 4;
    v_offset = (v_offset > content_size) ? (content_size) : (v_offset);
    auto *v_plane = yuv_data + v_offset;
    size_t v_stride = width / 2;

    SDL_UpdateYUVTexture(
        texture_, // the texture to update
        nullptr,  // a pointer to the rectangle of pixels to update, or NULL to
                  // update the entire texture
        y_plane,  // the raw pixel data for the Y plane
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
  std::string filepath_;
  SDL_Texture *texture_{nullptr};
};

#endif // SIMPLE_YUV_VIEWER_MY_YUV_LOADER_H
