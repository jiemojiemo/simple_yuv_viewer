//
// Created by user on 1/5/23.
//

#ifndef SIMPLE_YUV_VIEWER_MY_APPLICATION_H
#define SIMPLE_YUV_VIEWER_MY_APPLICATION_H
#pragma once

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "my_yuv_loader.h"

namespace MyApp {
class Application {
public:
  Application() = default;

  void RenderGUI(YUVFileLoader &loader, SDL_Renderer *renderer) {
    showDockSpace();
    showYUVSettings();
    if (loader.getLoadFilePath().empty()) {
      showDragToOpenWindow();
    } else {
      showYUVImage(loader, renderer);
    }
  }

  int format_item_index = 0;
  int yuv_width = 100;
  int yuv_height = 100;
  bool y_check = true;
  bool u_check = true;
  bool v_check = true;

private:
  void showDockSpace() {
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent
    // window not dockable into, because it would be confusing to have two
    // docking targets within each others.
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
      const ImGuiViewport *viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->WorkPos);
      ImGui::SetNextWindowSize(viewport->WorkSize);
      ImGui::SetNextWindowViewport(viewport->ID);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
      window_flags |= ImGuiWindowFlags_NoTitleBar |
                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                      ImGuiWindowFlags_NoMove;
      window_flags |=
          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    } else {
      dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will
    // render our background and handle the pass-thru hole, so we ask Begin() to
    // not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
      window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window
    // is collapsed). This is because we want to keep our DockSpace() active. If
    // a DockSpace() is inactive, all active windows docked into it will lose
    // their parent and become undocked. We cannot preserve the docking
    // relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in
    // limbo and never being visible.
    if (!opt_padding)
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    if (!opt_padding)
      ImGui::PopStyleVar();

    if (opt_fullscreen)
      ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Docking Space Options")) {
        // Disabling fullscreen would allow the window to be moved to the front
        // of other windows, which we can't undo at the moment without finer
        // window depth/z control.
        ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
        ImGui::MenuItem("Padding", NULL, &opt_padding);
        ImGui::Separator();

        if (ImGui::MenuItem("Flag: NoSplit", "",
                            (dockspace_flags & ImGuiDockNodeFlags_NoSplit) !=
                                0)) {
          dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
        }
        if (ImGui::MenuItem("Flag: NoResize", "",
                            (dockspace_flags & ImGuiDockNodeFlags_NoResize) !=
                                0)) {
          dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
        }
        if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "",
                            (dockspace_flags &
                             ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) {
          dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
        }
        if (ImGui::MenuItem(
                "Flag: AutoHideTabBar", "",
                (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) {
          dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
        }
        if (ImGui::MenuItem(
                "Flag: PassthruCentralNode", "",
                (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0,
                opt_fullscreen)) {
          dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
        }
        ImGui::Separator();

        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    ImGui::End();
  }

  void showYUVSettings() {
    ImGui::Begin("Settings");

    const char *items[] = {"YUV420",  "YUYV422", "UYVY422",
                           "YVYU422", "NV12",    "NV21"};
    ImGui::Combo("format", &format_item_index, items, IM_ARRAYSIZE(items));
    ImGui::InputInt("width", &yuv_width);
    ImGui::InputInt("height", &yuv_height);
    ImGui::Separator();

    auto format = YUVFormat(format_item_index);
    if (format != YUVFormat::kYUYV422 && format != YUVFormat::kUYVY422 &&
        format != YUVFormat::kYVYU422) {
      ImGui::Checkbox("Y", &y_check);
      ImGui::SameLine();
      ImGui::Checkbox("U", &u_check);
      ImGui::SameLine();
      ImGui::Checkbox("V", &v_check);
      ImGui::SameLine();
    }

    ImGui::End();
  }

  void showYUVImage(YUVFileLoader &loader, SDL_Renderer *renderer) {
    YUVSetting setting{};
    setting.format = YUVFormat(format_item_index);
    setting.width = yuv_width;
    setting.height = yuv_height;
    setting.show_y = y_check;
    setting.show_u = u_check;
    setting.show_v = v_check;

    SDL_Texture *image_texture = loader.updateTexture(setting, renderer);

    auto image_size = ImVec2{float(yuv_width), float(yuv_height)};
    ImGui::SetNextWindowSize(image_size,ImGuiCond_Always);
    ImGui::Begin("YUV Image", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("size = %d x %d", yuv_width, yuv_height);
    ImGui::Separator();
    ImGui::Image((void *)(intptr_t)image_texture,
                 image_size);
    ImGui::End();
  }

  void showDragToOpenWindow() {
    auto textCentered = [](std::string text) {
      auto windowWidth = ImGui::GetWindowSize().x;
      auto windowHeight = ImGui::GetWindowSize().y;

      auto textSize = ImGui::CalcTextSize(text.c_str());
      auto textWidth = textSize.x;
      auto textHeight = textSize.y;

      ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
      ImGui::SetCursorPosY((windowHeight - textHeight) * 0.5f);
      ImGui::Text("%s", text.c_str());
    };

    ImGui::SetNextWindowSize({300, 200}, ImGuiCond_Always);
    ImGui::Begin("Drag");
    textCentered("Drag YUV File Here To Open");
    ImGui::End();
  }
};

} // namespace MyApp

#endif // SIMPLE_YUV_VIEWER_MY_APPLICATION_H
