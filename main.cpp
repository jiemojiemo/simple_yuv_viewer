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

#include <string>
#include <fstream>

int main(int argc, char *argv[]) {
  bool quit = false;
  SDL_Event event;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
  {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  int window_width = 640;
  int window_height = 480;
  SDL_Window *window =
      SDL_CreateWindow("My SDL Empty window", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, window_width, window_height, 0);


  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (renderer == NULL)
  {
    SDL_Log("Error creating SDL_Renderer!");
    return 0;
  }

  // setup dear imgui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

  ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer_Init(renderer);


  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  bool show_demo_window = true;
  bool done = false;
  MyRender::GUIRender gui_render;
  std::string yuv_path = "/Users/user/Downloads/yuv-picture/juren_yuv_420.yuv";
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = window_width;
  rect.h = window_height;

  size_t width = 700, height = 700;
  uint8_t yuv_data[width*height*3/2];
  uint8_t* y_plane{nullptr};
  size_t y_stride = 0;
  uint8_t* u_plane{nullptr};
  size_t u_stride = 0;
  uint8_t* v_plane{nullptr};
  size_t v_stride = 0;
  {
    FILE *yuvfile = fopen("/Users/user/Downloads/rainbow-yuv420p.yuv", "rb");
    fread(yuv_data, sizeof(yuv_data), 1, yuvfile);
    fclose(yuvfile);

    y_plane = yuv_data;
    y_stride = width;

    u_plane = yuv_data + width*height;
    u_stride = width/2;

    v_plane = u_plane + (width*height)/4;
    v_stride = width/2;
  }

  SDL_Texture* texture = SDL_CreateTexture(renderer,
                                           SDL_PIXELFORMAT_YV12,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           width,
                                           height);
  SDL_UpdateYUVTexture(
      texture,           // the texture to update
      &rect,             // a pointer to the rectangle of pixels to update, or NULL to update the entire texture
      y_plane,     // the raw pixel data for the Y plane
      y_stride, // the number of bytes between rows of pixel data for the Y plane
      u_plane,     // the raw pixel data for the U plane
      u_stride, // the number of bytes between rows of pixel data for the U plane
      v_plane,     // the raw pixel data for the V plane
      v_stride  // the number of bytes between rows of pixel data for the V plane
  );

  while(!done)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    gui_render.Render();

    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(
        renderer, // the rendering context
        texture,  // the source texture
        NULL,     // the source SDL_Rect structure or NULL for the entire texture
        NULL      // the destination SDL_Rect structure or NULL for the entire rendering
                  // target; the texture will be stretched to fill the given rectangle
    );
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
  }

  // Cleanup
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}