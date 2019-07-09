//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <ui/style/theme.h>
#include <imgui/imgui.h>

namespace asap {
namespace ui {

void Theme::LoadDefaultStyle() {
  auto &style = ImGui::GetStyle();

  style.WindowPadding = ImVec2(5, 5);
  style.WindowRounding = 0.0f;
  style.FramePadding = ImVec2(5, 5);
  style.FrameRounding = 3.0f;
  style.ItemSpacing = ImVec2(12, 5);
  style.ItemInnerSpacing = ImVec2(5, 5);
  style.IndentSpacing = 25.0f;
  style.ScrollbarSize = 15.0f;
  style.ScrollbarRounding = 9.0f;
  style.GrabMinSize = 5.0f;
  style.GrabRounding = 3.0f;

  // clang-format off
  style.Colors[ImGuiCol_Text] = ImVec4(0.91f, 0.91f, 0.91f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
  style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.75f, 0.42f, 0.02f, 0.40f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.75f, 0.42f, 0.02f, 0.67f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 0.80f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.75f, 0.42f, 0.02f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.42f, 0.02f, 0.78f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.75f, 0.42f, 0.02f, 1.00f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.75f, 0.42f, 0.02f, 0.40f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.42f, 0.02f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.94f, 0.47f, 0.02f, 1.00f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.75f, 0.42f, 0.02f, 0.31f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.42f, 0.02f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.75f, 0.42f, 0.02f, 1.00f);
  style.Colors[ImGuiCol_Separator] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.42f, 0.02f, 0.78f);
  style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.42f, 0.02f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.75f, 0.42f, 0.02f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.75f, 0.42f, 0.02f, 0.95f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 0.57f, 0.65f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.10f, 0.30f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.40f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.75f, 0.42f, 0.02f, 0.35f);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
  style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.06f, 0.06f, 0.06f, 0.35f);
  // clang-format on
}

}  // namespace ui
}  // namespace asap
