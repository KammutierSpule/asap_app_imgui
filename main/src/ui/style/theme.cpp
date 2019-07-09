//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <array>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>  // for call_once()

#include <imgui/imgui.h>

#include <cpptoml.h>

#include <common/logging.h>
#include <config.h>
#include <ui/fonts/fonts.h>
#include <ui/fonts/material_design_icons.h>
#include <ui/style/theme.h>
#include <ui/style/font.h>

namespace asap {
namespace ui {


// !TODO: Implement it diferently
extern std::string BuildFontName(std::string const &family, Font::Weight weight,
                                 Font::Style style, Font::Size size);

extern ImFont *LoadRobotoFont(std::string const &name, Font::Weight weight,
                                 Font::Style style, Font::Size size);

extern ImFont *LoadInconsolataFont(std::string const &name, Font::Weight weight,
                                 Font::Style style, Font::Size size);

extern ImFont *LoadIconsFont(float size);


std::map<std::string, ImFont *> Theme::fonts_;
ImFont *Theme::icons_font_normal_{nullptr};

void Theme::Init() {

#if (THEME_USE_LOAD_SAVE)
  LoadStyle();
#else
  LoadDefaultStyle();
#endif

  //
  // Fonts
  //
  static std::once_flag init_flag;
  std::call_once(init_flag, []() {
    ImGui::GetIO().Fonts->AddFontDefault();
    LoadDefaultFonts();
    // TODO: Temporary default font - can be configured
    Font default_font(Font::FAMILY_PROPORTIONAL);
    default_font.Regular().MediumSize();
    ImGui::GetIO().FontDefault = default_font.ImGuiFont();
  });
}

void Theme::LoadDefaultFonts() {
  // There is currently an issue with too many fonts being loaded by ImGui that
  // cause a problems on certain platforms.
  // To decrease the chances of people running into this problem (usually a
  // blank screen for ImGui), we are only loading a minimal number of
  // variations. On systems with pretty decent 3D graphics, all variations can
  // be loaded with no issues.

  std::array<Font::Weight, 2> font_weights{
      {Font::Weight::REGULAR, Font::Weight::BOLD}};
  // std::array<Font::Weight, 3> font_weights{
  //     {Font::Weight::LIGHT, Font::Weight::REGULAR, Font::Weight::BOLD}};
  std::array<Font::Style, 2> font_styles{
      {Font::Style::NORMAL, Font::Style::ITALIC}};
  std::array<Font::Size, 2> font_sizes{{Font::Size::SMALL, Font::Size::MEDIUM}};
  // std::array<Font::Size, 4> font_sizes{{Font::Size::SMALL,
  // Font::Size::MEDIUM,
  //                                       Font::Size::LARGE,
  //                                       Font::Size::LARGER}};
  for (auto size : font_sizes) {
    for (auto weight : font_weights) {
      for (auto style : font_styles) {
        auto name =
            BuildFontName(Font::FAMILY_PROPORTIONAL, weight, style, size);
        auto font = LoadRobotoFont(name, weight, style, size);
        if (font) AddFont(name, font);
      }
    }

    // Monospaced
    auto name = BuildFontName(Font::FAMILY_MONOSPACE, Font::Weight::REGULAR,
                              Font::Style::NORMAL, size);
    auto font = LoadInconsolataFont(name, Font::Weight::REGULAR,
                                    Font::Style::NORMAL, size);
    if (font) {
      AddFont(name, font);
      // No Italic
      AddFont(BuildFontName(Font::FAMILY_MONOSPACE, Font::Weight::REGULAR,
                            Font::Style::ITALIC, size),
              font);
      // Treat LIGHT same as REGULAR
      AddFont(BuildFontName(Font::FAMILY_MONOSPACE, Font::Weight::LIGHT,
                            Font::Style::NORMAL, size),
              font);
      AddFont(BuildFontName(Font::FAMILY_MONOSPACE, Font::Weight::LIGHT,
                            Font::Style::ITALIC, size),
              font);
    }

    name = BuildFontName(Font::FAMILY_MONOSPACE, Font::Weight::BOLD,
                         Font::Style::NORMAL, size);
    font = LoadInconsolataFont(name, Font::Weight::BOLD, Font::Style::NORMAL,
                               size);
    if (font) {
      AddFont(name, font);
      // No Italic
      AddFont(BuildFontName(Font::FAMILY_MONOSPACE, Font::Weight::BOLD,
                            Font::Style::ITALIC, size),
              font);
    }
  }

  // The Icons font
  icons_font_normal_ = LoadIconsFont(32.0f);
}


ImFont *Theme::GetFont(std::string const &name) {
  auto font = fonts_.find(name);
  if (font == fonts_.end()) {
    return ImGui::GetFont();
  }
  return font->second;
}

void Theme::AddFont(std::string const &name, ImFont *font) {
  fonts_.insert({name, font});
}

// -------------------------------------------------------------------------
// Settings load/save
// -------------------------------------------------------------------------

#if (THEME_USE_LOAD_SAVE)

namespace {
void ConfigSanityChecks(std::shared_ptr<cpptoml::table> &config) {
  auto &logger = asap::logging::Registry::GetLogger("main");

  auto theme = config->get_table("theme");
  if (!theme) {
    ASLOG_TO_LOGGER(logger, warn, "missing 'theme' in config");
  }

  if (!theme->contains("style")) {
    ASLOG_TO_LOGGER(logger, warn, "missing 'theme/style' in config");
  }

  if (!theme->contains("colors")) {
    ASLOG_TO_LOGGER(logger, warn, "missing 'theme/colors' in config");
  }
}

}  // namespace

#define EMIT_TOML_STYLE_IMVEC2(FIELD)            \
  {                                              \
    auto float_array = cpptoml::make_array();    \
    float_array->push_back(style.FIELD.x);       \
    float_array->push_back(style.FIELD.y);       \
    style_settings->insert(#FIELD, float_array); \
  }

#define EMIT_TOML_COLOR(COLOR_ID)                    \
  {                                                  \
    auto float_array = cpptoml::make_array();        \
    auto &color = colors[COLOR_ID];                  \
    float_array->push_back(color.x);                 \
    float_array->push_back(color.y);                 \
    float_array->push_back(color.z);                 \
    float_array->push_back(color.w);                 \
    colors_settings->insert(#COLOR_ID, float_array); \
  }

void Theme::SaveStyle() {
  std::shared_ptr<cpptoml::table> root = cpptoml::make_table();

  // [theme]
  {
    auto theme_settings = cpptoml::make_table();
    auto &style = ImGui::GetStyle();

    // [style]
    {
      auto style_settings = cpptoml::make_table();
      style_settings->insert("Alpha", style.Alpha);
      EMIT_TOML_STYLE_IMVEC2(WindowPadding);
      style_settings->insert("WindowRounding", style.WindowRounding);
      style_settings->insert("WindowBorderSize", style.WindowBorderSize);
      EMIT_TOML_STYLE_IMVEC2(WindowMinSize);
      EMIT_TOML_STYLE_IMVEC2(WindowTitleAlign);
      style_settings->insert("ChildRounding", style.ChildRounding);
      style_settings->insert("ChildBorderSize", style.ChildBorderSize);
      style_settings->insert("PopupRounding", style.PopupRounding);
      style_settings->insert("PopupBorderSize", style.PopupBorderSize);
      EMIT_TOML_STYLE_IMVEC2(FramePadding);
      style_settings->insert("FrameRounding", style.FrameRounding);
      style_settings->insert("FrameBorderSize", style.FrameBorderSize);
      EMIT_TOML_STYLE_IMVEC2(ItemSpacing);
      EMIT_TOML_STYLE_IMVEC2(ItemInnerSpacing);
      EMIT_TOML_STYLE_IMVEC2(TouchExtraPadding);
      style_settings->insert("IndentSpacing", style.IndentSpacing);
      style_settings->insert("ColumnsMinSpacing", style.ColumnsMinSpacing);
      style_settings->insert("ScrollbarSize", style.ScrollbarSize);
      style_settings->insert("ScrollbarRounding", style.ScrollbarRounding);
      style_settings->insert("GrabMinSize", style.GrabMinSize);
      style_settings->insert("GrabRounding", style.GrabRounding);
      EMIT_TOML_STYLE_IMVEC2(ButtonTextAlign);
      EMIT_TOML_STYLE_IMVEC2(DisplayWindowPadding);
      EMIT_TOML_STYLE_IMVEC2(DisplaySafeAreaPadding);
      style_settings->insert("MouseCursorScale", style.MouseCursorScale);
      style_settings->insert("AntiAliasedLines", style.AntiAliasedLines);
      style_settings->insert("AntiAliasedFill", style.AntiAliasedFill);
      style_settings->insert("CurveTessellationTol",
                             style.CurveTessellationTol);

      theme_settings->insert("style", style_settings);
    }

    // [colors]
    {
      auto colors_settings = cpptoml::make_table();
      auto &colors = style.Colors;

      EMIT_TOML_COLOR(ImGuiCol_Text);
      EMIT_TOML_COLOR(ImGuiCol_TextDisabled);
      EMIT_TOML_COLOR(ImGuiCol_WindowBg);
      EMIT_TOML_COLOR(ImGuiCol_ChildBg);
      EMIT_TOML_COLOR(ImGuiCol_PopupBg);
      EMIT_TOML_COLOR(ImGuiCol_Border);
      EMIT_TOML_COLOR(ImGuiCol_BorderShadow);
      EMIT_TOML_COLOR(ImGuiCol_FrameBg);
      EMIT_TOML_COLOR(ImGuiCol_FrameBgHovered);
      EMIT_TOML_COLOR(ImGuiCol_FrameBgActive);
      EMIT_TOML_COLOR(ImGuiCol_TitleBg);
      EMIT_TOML_COLOR(ImGuiCol_TitleBgActive);
      EMIT_TOML_COLOR(ImGuiCol_TitleBgCollapsed);
      EMIT_TOML_COLOR(ImGuiCol_MenuBarBg);
      EMIT_TOML_COLOR(ImGuiCol_ScrollbarBg);
      EMIT_TOML_COLOR(ImGuiCol_ScrollbarGrab);
      EMIT_TOML_COLOR(ImGuiCol_ScrollbarGrabHovered);
      EMIT_TOML_COLOR(ImGuiCol_ScrollbarGrabActive);
      EMIT_TOML_COLOR(ImGuiCol_CheckMark);
      EMIT_TOML_COLOR(ImGuiCol_SliderGrab);
      EMIT_TOML_COLOR(ImGuiCol_SliderGrabActive);
      EMIT_TOML_COLOR(ImGuiCol_Button);
      EMIT_TOML_COLOR(ImGuiCol_ButtonHovered);
      EMIT_TOML_COLOR(ImGuiCol_ButtonActive);
      EMIT_TOML_COLOR(ImGuiCol_Header);
      EMIT_TOML_COLOR(ImGuiCol_HeaderHovered);
      EMIT_TOML_COLOR(ImGuiCol_HeaderActive);
      EMIT_TOML_COLOR(ImGuiCol_Separator);
      EMIT_TOML_COLOR(ImGuiCol_SeparatorHovered);
      EMIT_TOML_COLOR(ImGuiCol_SeparatorActive);
      EMIT_TOML_COLOR(ImGuiCol_ResizeGrip);
      EMIT_TOML_COLOR(ImGuiCol_ResizeGripHovered);
      EMIT_TOML_COLOR(ImGuiCol_ResizeGripActive);
      EMIT_TOML_COLOR(ImGuiCol_PlotLines);
      EMIT_TOML_COLOR(ImGuiCol_PlotLinesHovered);
      EMIT_TOML_COLOR(ImGuiCol_PlotHistogram);
      EMIT_TOML_COLOR(ImGuiCol_PlotHistogramHovered);
      EMIT_TOML_COLOR(ImGuiCol_TextSelectedBg);
      EMIT_TOML_COLOR(ImGuiCol_DragDropTarget);
      EMIT_TOML_COLOR(ImGuiCol_NavHighlight);
      EMIT_TOML_COLOR(ImGuiCol_NavWindowingHighlight);
      EMIT_TOML_COLOR(ImGuiCol_NavWindowingDimBg);
      EMIT_TOML_COLOR(ImGuiCol_ModalWindowDimBg);

      theme_settings->insert("colors", colors_settings);
    }
    root->insert("theme", theme_settings);
  }

  auto settings_path =
      asap::fs::GetPathFor(asap::fs::Location::F_THEME_SETTINGS);
  auto ofs = std::ofstream();
  ofs.open(settings_path.string());
  ofs << (*root) << std::endl;
  ofs.close();
}

#define SET_COLOR_FROM_TOML(id)                                              \
  if (colors_settings->contains(#id)) {                                      \
    auto color = colors_settings->get_array_of<double>(#id);                 \
    colors[id] = {                                                           \
        static_cast<float>(color->at(0)), static_cast<float>(color->at(1)),  \
        static_cast<float>(color->at(2)), static_cast<float>(color->at(3))}; \
  }

void Theme::LoadStyle() {
  auto &logger = asap::logging::Registry::GetLogger("main");

  std::shared_ptr<cpptoml::table> config;
  auto theme_settings =
      asap::fs::GetPathFor(asap::fs::Location::F_THEME_SETTINGS);
  auto has_config = false;
  if (asap::filesystem::exists(theme_settings)) {
    try {
      config = cpptoml::parse_file(theme_settings.string());
      ASLOG_TO_LOGGER(logger, info, "theme settings loaded from {}",
                      theme_settings);
      has_config = true;
    } catch (std::exception const &ex) {
      ASLOG_TO_LOGGER(logger, error,
                      "error () while loading theme settings from {}",
                      ex.what(), theme_settings);
    }
  } else {
    ASLOG_TO_LOGGER(logger, info, "file {} does not exist", theme_settings);
  }

  if (has_config) {
    ConfigSanityChecks(config);

    if (config->contains("theme")) {
      auto theme = config->get_table("theme");
      if (theme->contains("style")) {
        auto style = theme->get_table("style");

        if (style->contains("Alpha")) {
          ImGui::GetStyle().Alpha =
              static_cast<float>(*(style->get_as<double>("Alpha")));
        }
        if (style->contains("WindowPadding")) {
          auto vec2 = style->get_array_of<double>("WindowPadding");
          ImGui::GetStyle().WindowPadding = {static_cast<float>(vec2->at(0)),
                                             static_cast<float>(vec2->at(1))};
        }

        if (style->contains("WindowRounding")) {
          ImGui::GetStyle().WindowRounding =
              static_cast<float>(*(style->get_as<double>("WindowRounding")));
        }
        if (style->contains("WindowBorderSize")) {
          ImGui::GetStyle().WindowBorderSize =
              static_cast<float>(*(style->get_as<double>("WindowBorderSize")));
        }
        if (style->contains("WindowMinSize")) {
          auto vec2 = style->get_array_of<double>("WindowMinSize");
          ImGui::GetStyle().WindowMinSize = {static_cast<float>(vec2->at(0)),
                                             static_cast<float>(vec2->at(1))};
        }
        if (style->contains("WindowTitleAlign")) {
          auto vec2 = style->get_array_of<double>("WindowTitleAlign");
          ImGui::GetStyle().WindowTitleAlign = {
              static_cast<float>(vec2->at(0)), static_cast<float>(vec2->at(1))};
        }

        if (style->contains("ChildRounding")) {
          ImGui::GetStyle().ChildRounding =
              static_cast<float>(*(style->get_as<double>("ChildRounding")));
        }
        if (style->contains("ChildBorderSize")) {
          ImGui::GetStyle().ChildBorderSize =
              static_cast<float>(*(style->get_as<double>("ChildBorderSize")));
        }
        if (style->contains("PopupRounding")) {
          ImGui::GetStyle().PopupRounding =
              static_cast<float>(*(style->get_as<double>("PopupRounding")));
        }
        if (style->contains("PopupBorderSize")) {
          ImGui::GetStyle().PopupBorderSize =
              static_cast<float>(*(style->get_as<double>("PopupBorderSize")));
        }
        if (style->contains("FramePadding")) {
          auto vec2 = style->get_array_of<double>("FramePadding");
          ImGui::GetStyle().FramePadding = {static_cast<float>(vec2->at(0)),
                                            static_cast<float>(vec2->at(1))};
        }

        if (style->contains("FrameRounding")) {
          ImGui::GetStyle().FrameRounding =
              static_cast<float>(*(style->get_as<double>("FrameRounding")));
        }
        if (style->contains("FrameBorderSize")) {
          ImGui::GetStyle().FrameBorderSize =
              static_cast<float>(*(style->get_as<double>("FrameBorderSize")));
        }
        if (style->contains("ItemSpacing")) {
          auto vec2 = style->get_array_of<double>("ItemSpacing");
          ImGui::GetStyle().ItemSpacing = {static_cast<float>(vec2->at(0)),
                                           static_cast<float>(vec2->at(1))};
        }

        if (style->contains("ItemInnerSpacing")) {
          auto vec2 = style->get_array_of<double>("ItemInnerSpacing");
          ImGui::GetStyle().ItemInnerSpacing = {
              static_cast<float>(vec2->at(0)), static_cast<float>(vec2->at(1))};
        }

        if (style->contains("TouchExtraPadding")) {
          auto vec2 = style->get_array_of<double>("TouchExtraPadding");
          ImGui::GetStyle().TouchExtraPadding = {
              static_cast<float>(vec2->at(0)), static_cast<float>(vec2->at(1))};
        }

        if (style->contains("IndentSpacing")) {
          ImGui::GetStyle().IndentSpacing =
              static_cast<float>(*(style->get_as<double>("IndentSpacing")));
        }
        if (style->contains("ColumnsMinSpacing")) {
          ImGui::GetStyle().ColumnsMinSpacing =
              static_cast<float>(*(style->get_as<double>("ColumnsMinSpacing")));
        }
        if (style->contains("ScrollbarSize")) {
          ImGui::GetStyle().ScrollbarSize =
              static_cast<float>(*(style->get_as<double>("ScrollbarSize")));
        }
        if (style->contains("ScrollbarRounding")) {
          ImGui::GetStyle().ScrollbarRounding =
              static_cast<float>(*(style->get_as<double>("ScrollbarRounding")));
        }
        if (style->contains("GrabMinSize")) {
          ImGui::GetStyle().GrabMinSize =
              static_cast<float>(*(style->get_as<double>("GrabMinSize")));
        }
        if (style->contains("GrabRounding")) {
          ImGui::GetStyle().GrabRounding =
              static_cast<float>(*(style->get_as<double>("GrabRounding")));
        }
        if (style->contains("ButtonTextAlign")) {
          auto vec2 = style->get_array_of<double>("ButtonTextAlign");
          ImGui::GetStyle().ButtonTextAlign = {static_cast<float>(vec2->at(0)),
                                               static_cast<float>(vec2->at(1))};
        }

        if (style->contains("DisplayWindowPadding")) {
          auto vec2 = style->get_array_of<double>("DisplayWindowPadding");
          ImGui::GetStyle().DisplayWindowPadding = {
              static_cast<float>(vec2->at(0)), static_cast<float>(vec2->at(1))};
        }

        if (style->contains("DisplaySafeAreaPadding")) {
          auto vec2 = style->get_array_of<double>("DisplaySafeAreaPadding");
          ImGui::GetStyle().DisplaySafeAreaPadding = {
              static_cast<float>(vec2->at(0)), static_cast<float>(vec2->at(1))};
        }

        if (style->contains("MouseCursorScale")) {
          ImGui::GetStyle().MouseCursorScale =
              static_cast<float>(*(style->get_as<double>("MouseCursorScale")));
        }
        if (style->contains("AntiAliasedLines")) {
          ImGui::GetStyle().AntiAliasedLines =
              *(style->get_as<bool>("AntiAliasedLines"));
        }
        if (style->contains("AntiAliasedFill")) {
          ImGui::GetStyle().AntiAliasedFill =
              *(style->get_as<bool>("AntiAliasedFill"));
        }
        if (style->contains("CurveTessellationTol")) {
          ImGui::GetStyle().CurveTessellationTol = static_cast<float>(
              *(style->get_as<double>("CurveTessellationTol")));
        }
      }
      if (theme->contains("colors")) {
        auto &colors = ImGui::GetStyle().Colors;
        auto colors_settings = theme->get_table("colors");

        SET_COLOR_FROM_TOML(ImGuiCol_Text);
        SET_COLOR_FROM_TOML(ImGuiCol_TextDisabled);
        SET_COLOR_FROM_TOML(ImGuiCol_WindowBg);
        SET_COLOR_FROM_TOML(ImGuiCol_ChildBg);
        SET_COLOR_FROM_TOML(ImGuiCol_PopupBg);
        SET_COLOR_FROM_TOML(ImGuiCol_Border);
        SET_COLOR_FROM_TOML(ImGuiCol_BorderShadow);
        SET_COLOR_FROM_TOML(ImGuiCol_FrameBg);
        SET_COLOR_FROM_TOML(ImGuiCol_FrameBgHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_FrameBgActive);
        SET_COLOR_FROM_TOML(ImGuiCol_TitleBg);
        SET_COLOR_FROM_TOML(ImGuiCol_TitleBgActive);
        SET_COLOR_FROM_TOML(ImGuiCol_TitleBgCollapsed);
        SET_COLOR_FROM_TOML(ImGuiCol_MenuBarBg);
        SET_COLOR_FROM_TOML(ImGuiCol_ScrollbarBg);
        SET_COLOR_FROM_TOML(ImGuiCol_ScrollbarGrab);
        SET_COLOR_FROM_TOML(ImGuiCol_ScrollbarGrabHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_ScrollbarGrabActive);
        SET_COLOR_FROM_TOML(ImGuiCol_CheckMark);
        SET_COLOR_FROM_TOML(ImGuiCol_SliderGrab);
        SET_COLOR_FROM_TOML(ImGuiCol_SliderGrabActive);
        SET_COLOR_FROM_TOML(ImGuiCol_Button);
        SET_COLOR_FROM_TOML(ImGuiCol_ButtonHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_ButtonActive);
        SET_COLOR_FROM_TOML(ImGuiCol_Header);
        SET_COLOR_FROM_TOML(ImGuiCol_HeaderHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_HeaderActive);
        SET_COLOR_FROM_TOML(ImGuiCol_Separator);
        SET_COLOR_FROM_TOML(ImGuiCol_SeparatorHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_SeparatorActive);
        SET_COLOR_FROM_TOML(ImGuiCol_ResizeGrip);
        SET_COLOR_FROM_TOML(ImGuiCol_ResizeGripHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_ResizeGripActive);
        SET_COLOR_FROM_TOML(ImGuiCol_PlotLines);
        SET_COLOR_FROM_TOML(ImGuiCol_PlotLinesHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_PlotHistogram);
        SET_COLOR_FROM_TOML(ImGuiCol_PlotHistogramHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_PlotHistogramHovered);
        SET_COLOR_FROM_TOML(ImGuiCol_TextSelectedBg);
        SET_COLOR_FROM_TOML(ImGuiCol_DragDropTarget);
        SET_COLOR_FROM_TOML(ImGuiCol_NavHighlight);
        SET_COLOR_FROM_TOML(ImGuiCol_NavWindowingHighlight);
        SET_COLOR_FROM_TOML(ImGuiCol_NavWindowingHighlight);
        SET_COLOR_FROM_TOML(ImGuiCol_NavWindowingDimBg);
        SET_COLOR_FROM_TOML(ImGuiCol_ModalWindowDimBg);
      }
    }
  } else {
    LoadDefaultStyle();
  }
}

#endif

}  // namespace ui
}  // namespace asap
