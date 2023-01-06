//
// Created by user on 1/5/23.
//

#ifndef SIMPLE_YUV_VIEWER_MY_RENDER_H
#define SIMPLE_YUV_VIEWER_MY_RENDER_H
#pragma once

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

namespace MyRender
{
class GUIRender
{
public:
  GUIRender() = default;

  void Render()
  {
    ImGui::Begin("Hello, world!");
    // Create a window called "Hello, world!" and append into it.

    const char* items[] = { "YUV420", "YUV444", "YUV422" };
    ImGui::Combo("format", &format_item_index, items, IM_ARRAYSIZE(items));

    ImGui::InputInt("width", &yuv_width);
    ImGui::InputInt("height", &yuv_height);

    ImGui::End();
  }

  int format_item_index = 0; // If the selection isn't within 0..count, Combo won't display a preview
  int yuv_width = 100;
  int yuv_height = 100;
};

}

#endif // SIMPLE_YUV_VIEWER_MY_RENDER_H
