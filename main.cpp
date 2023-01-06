//
// Created by user on 1/3/23.
//

#if defined(__cplusplus)
extern "C" {
#endif
#include <SDL2/SDL.h>
#if defined(__cplusplus)
};
#endif

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "my_render.h"

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
    if (texture_ == nullptr) {
      createTexture(width, height, renderer);
    } else {
      auto [tex_width, tex_height] = getTextureSize(texture_);
      if (width != tex_width || height != tex_height) {
        createTexture(width, height, renderer);
      }
    }

    {
      SDL_Rect rect;
      rect.x = 0;
      rect.y = 0;
      rect.w = 300;
      rect.h = 300;

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

  std::string file_contents_;
  SDL_Texture *texture_{nullptr};
};

int main(int argc, char *argv[]) {
  bool quit = false;
  SDL_Event event;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  int window_width = 640;
  int window_height = 480;
  SDL_Window *window =
      SDL_CreateWindow("My SDL Empty window", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, window_width, window_height, 0);

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    SDL_Log("Error creating SDL_Renderer!");
    return 0;
  }

  // setup dear imgui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad
  // Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport /
                                                      // Platform Windows

  ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer_Init(renderer);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  bool show_demo_window = true;
  bool done = false;
  MyRender::GUIRender gui_render;
  std::string yuv_path = "/Users/user/Downloads/rainbow-yuv420p.yuv";
  YUVFileLoader yuv_loader;
  yuv_loader.loadFile(yuv_path);

  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    gui_render.Render();

    {
      SDL_Texture* image_texture = yuv_loader.updateTexture(YUVFormat::kYUV420, gui_render.yuv_width, gui_render.yuv_height,
                                 renderer);

      ImGui::Begin("SDL Texture Text");
      ImGui::Text("size = %d x %d", gui_render.yuv_width, gui_render.yuv_height);
      ImGui::Image((void*)(intptr_t)image_texture, ImVec2(gui_render.yuv_width, gui_render.yuv_height));
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(
        renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255),
        (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);

    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
  }

  // Cleanup
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}