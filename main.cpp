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
#include "my_application.h"
#include "my_yuv_loader.h"

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
      SDL_CreateWindow("Simple YUV viewer", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_RESIZABLE);

  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

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

  bool done = false;
  MyApp::Application gui_render;
  YUVFileLoader yuv_loader;

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

      if(event.type == SDL_DROPFILE)
      {
        yuv_loader.loadFile(event.drop.file);
      }

    }

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    gui_render.RenderGUI(yuv_loader, renderer);

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