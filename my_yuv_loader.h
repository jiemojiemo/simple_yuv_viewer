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
#include <vector>

enum class YUVFormat {
  kYUV420 = 0,
  kYUYV422,
  kUYVY422,
  kYVYU422,
  kNV12,
  kNV21
};

struct YUVSetting
{
  YUVFormat format;
  int width;
  int height;
  bool show_y;
  bool show_u;
  bool show_v;
};


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

      fake_128_data_.resize(file_contents_.size(), 128);

      return true;
    } else {
      filepath_ = "";
      file_contents_.clear();
    }
    return false;
  }

  const std::string &getLoadFilePath() const { return filepath_; }

  SDL_Texture* updateTexture(const YUVSetting& setting,
                             SDL_Renderer *renderer
                             )
  {
    createTextureIfNeed(setting, renderer);
    switch (setting.format) {
    case YUVFormat::kYUV420:
      updateTextureYUV420(setting);
      break;
    case YUVFormat::kYUYV422:
    case YUVFormat::kYVYU422:
    case YUVFormat::kUYVY422:
      updateTexturePacketYUV422(setting);
      break;
    case YUVFormat::kNV12:
    case YUVFormat::kNV21:
      updateTextureNV(setting);
      break;
    }

    return texture_;
  }

private:
  void createTexture(const YUVSetting& setting,
                     SDL_Renderer *renderer) {
    if (texture_ != nullptr) {
      SDL_DestroyTexture(texture_);
      texture_ = nullptr;
    }

    auto sdl_pixel_format = SDL_PIXELFORMAT_IYUV;

    switch (setting.format) {
    case YUVFormat::kYUV420:
      sdl_pixel_format = SDL_PIXELFORMAT_IYUV;
      break;
    case YUVFormat::kYUYV422:
      sdl_pixel_format = SDL_PIXELFORMAT_YUY2;
      break;
    case YUVFormat::kUYVY422:
      sdl_pixel_format = SDL_PIXELFORMAT_UYVY;
      break;
    case YUVFormat::kNV12:
      sdl_pixel_format = SDL_PIXELFORMAT_NV12;
      break;
    case YUVFormat::kYVYU422:
      sdl_pixel_format = SDL_PIXELFORMAT_YVYU;
      break;
    case YUVFormat::kNV21:
      sdl_pixel_format = SDL_PIXELFORMAT_NV21;
      break;
    }

    texture_ = SDL_CreateTexture(renderer, sdl_pixel_format,
                                 SDL_TEXTUREACCESS_STREAMING, setting.width, setting.height);
  }

  void createTextureIfNeed(const YUVSetting& setting,
                           SDL_Renderer *renderer) {
    if (texture_ == nullptr) {
      createTexture(setting, renderer);
    } else {
      auto tex_width = 0;
      auto tex_height = 0;
      Uint32 tex_format = 0;
      SDL_QueryTexture(texture_, &tex_format, NULL, &tex_width, &tex_height);
      if (setting.width != tex_width || setting.height != tex_height ||
          tex_format != Uint32(setting.format)) {
        createTexture(setting, renderer);
      }
    }
  }

  void updateTextureNV(const YUVSetting& setting) {
    if (file_contents_.empty()) {
      return;
    }

    auto content_size = file_contents_.size();
    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto *y_plane = yuv_data;
    size_t y_stride = setting.width;

    auto uv_offset = setting.width * setting.height;
    uv_offset = (uv_offset > content_size) ? (content_size) : (uv_offset);
    auto *uv_plane = yuv_data + uv_offset;
    size_t uv_stride = setting.width;

    if(!setting.show_y){
      y_plane = fake_128_data_.data();
    }

    if(!setting.show_u || !setting.show_v){
      uv_plane = fake_128_data_.data();
    }


    SDL_UpdateNVTexture(texture_, nullptr, y_plane, y_stride, uv_plane,
                        uv_stride);
  }

  void updateTexturePacketYUV422(const YUVSetting& setting) {
    if (file_contents_.empty()) {
      return;
    }

    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto pitch = 2 * setting.width;
    SDL_UpdateTexture(texture_, nullptr, yuv_data, pitch);
  }

  void updateTextureYUV420(const YUVSetting& setting) {
    if (file_contents_.empty()) {
      return;
    }

    auto content_size = file_contents_.size();
    auto *yuv_data = reinterpret_cast<const uint8_t *>(file_contents_.data());
    auto *y_plane = yuv_data;
    size_t y_stride = setting.width;

    auto u_offset = setting.width * setting.height;
    u_offset = (u_offset > content_size) ? (content_size) : (u_offset);
    auto *u_plane = yuv_data + u_offset;
    size_t u_stride = setting.width / 2;

    auto v_offset = u_offset + (setting.width * setting.height) / 4;
    v_offset = (v_offset > content_size) ? (content_size) : (v_offset);
    auto *v_plane = yuv_data + v_offset;
    size_t v_stride = setting.width / 2;

    if(!setting.show_y){
      y_plane = fake_128_data_.data();
    }

    if(!setting.show_u){
      u_plane = fake_128_data_.data();
    }

    if(!setting.show_v){
      v_plane = fake_128_data_.data();
    }

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
  std::vector<uint8_t> fake_128_data_;
  std::string filepath_;
  SDL_Texture *texture_{nullptr};
};

#endif // SIMPLE_YUV_VIEWER_MY_YUV_LOADER_H
