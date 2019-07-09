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

std::string const Font::FAMILY_MONOSPACE{"Inconsolata"};
std::string const Font::FAMILY_PROPORTIONAL{"Roboto"};

//namespace {

// !TODO: Implement this load functions diferently

std::string BuildFontName(std::string const &family, Font::Weight weight,
                          Font::Style style, Font::Size size) {
  std::string name(family);
  name.append(" ").append(Font::WeightString(weight));
  if (style == Font::Style::ITALIC)
    name.append(" ").append(Font::StyleString(style));
  name.append(" ").append(Font::SizeString(size));
  return name;
}

/// Merge in icons from Font Material Design Icons font.
ImFont *MergeIcons(float size) {
  ImGuiIO &io = ImGui::GetIO();
  // The ranges array is not copied by the AddFont* functions and is used lazily
  // so ensure it is available for duration of font usage
  static const ImWchar icons_ranges[] = {ICON_MIN_MDI, ICON_MAX_MDI, 0};

  ImFontConfig fontConfig;
  // Set Oversamping parameters to 1 on both axis, the texture will be 6 times
  // smaller. See https://github.com/ocornut/imgui/issues/1527
  fontConfig.OversampleH = 1;
  fontConfig.OversampleV = 1;
  fontConfig.MergeMode = true;
  fontConfig.PixelSnapH = true;
  auto font = io.Fonts->AddFontFromMemoryCompressedTTF(
      asap::debug::ui::Fonts::MATERIAL_DESIGN_ICONS_COMPRESSED_DATA,
      asap::debug::ui::Fonts::MATERIAL_DESIGN_ICONS_COMPRESSED_SIZE, size,
      &fontConfig, icons_ranges);
  // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

  return font;
}

ImFont *LoadRobotoFont(std::string const &name, Font::Weight weight,
                       Font::Style style, Font::Size size) {
  ImGuiIO &io = ImGui::GetIO();
  ImFontConfig fontConfig;
  // Set Oversamping parameters to 1 on both axis, the texture will be 6 times
  // smaller. See https://github.com/ocornut/imgui/issues/1527
  fontConfig.OversampleH = 1;
  fontConfig.OversampleV = 1;
  fontConfig.MergeMode = false;
  std::strncpy(fontConfig.Name, name.c_str(), sizeof(fontConfig.Name) - 1);
  fontConfig.Name[sizeof(fontConfig.Name) - 1] = 0;
  ImFont *font = nullptr;
  switch (weight) {
    case Font::Weight::LIGHT:
      switch (style) {
        case Font::Style::ITALIC:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::ROBOTO_LIGHTITALIC_COMPRESSED_DATA,
              asap::debug::ui::Fonts::ROBOTO_LIGHTITALIC_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig,
              io.Fonts->GetGlyphRangesDefault());
          font = MergeIcons(Font::SizeFloat(size));
          break;
        case Font::Style::NORMAL:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::ROBOTO_LIGHT_COMPRESSED_DATA,
              asap::debug::ui::Fonts::ROBOTO_LIGHT_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig,
              io.Fonts->GetGlyphRangesDefault());
          font = MergeIcons(Font::SizeFloat(size));
          break;
      }
      break;
    case Font::Weight::REGULAR:
      switch (style) {
        case Font::Style::ITALIC:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::ROBOTO_ITALIC_COMPRESSED_DATA,
              asap::debug::ui::Fonts::ROBOTO_ITALIC_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig,
              io.Fonts->GetGlyphRangesDefault());
          font = MergeIcons(Font::SizeFloat(size));
          break;
        case Font::Style::NORMAL:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::ROBOTO_REGULAR_COMPRESSED_DATA,
              asap::debug::ui::Fonts::ROBOTO_REGULAR_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig,
              io.Fonts->GetGlyphRangesDefault());
          font = MergeIcons(Font::SizeFloat(size));
          break;
      }
      break;
    case Font::Weight::BOLD:
      switch (style) {
        case Font::Style::ITALIC:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::ROBOTO_BOLDITALIC_COMPRESSED_DATA,
              asap::debug::ui::Fonts::ROBOTO_BOLDITALIC_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig,
              io.Fonts->GetGlyphRangesDefault());
          font = MergeIcons(Font::SizeFloat(size));
          break;
        case Font::Style::NORMAL:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::ROBOTO_BOLD_COMPRESSED_DATA,
              asap::debug::ui::Fonts::ROBOTO_BOLD_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig);
          font = MergeIcons(Font::SizeFloat(size));
          break;
      }
      break;
  }
  return font;
}

ImFont *LoadInconsolataFont(std::string const &name, Font::Weight weight,
                            Font::Style style, Font::Size size) {
  ImGuiIO &io = ImGui::GetIO();
  ImFontConfig fontConfig;
  // Set Oversamping parameters to 1 on both axis, the texture will be 6 times
  // smaller. See https://github.com/ocornut/imgui/issues/1527
  fontConfig.OversampleH = 1;
  fontConfig.OversampleV = 1;
  fontConfig.MergeMode = false;
  std::strncpy(fontConfig.Name, name.c_str(), sizeof(fontConfig.Name) - 1);
  fontConfig.Name[sizeof(fontConfig.Name) - 1] = 0;
  ImFont *font = nullptr;
  switch (weight) {
    case Font::Weight::LIGHT:
      break;
    case Font::Weight::REGULAR:
      switch (style) {
        case Font::Style::ITALIC:
        case Font::Style::NORMAL:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::INCONSOLATA_REGULAR_COMPRESSED_DATA,
              asap::debug::ui::Fonts::INCONSOLATA_REGULAR_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig);
          font = MergeIcons(Font::SizeFloat(size));
          break;
      }
      break;
    case Font::Weight::BOLD:
      switch (style) {
        case Font::Style::ITALIC:
        case Font::Style::NORMAL:
          io.Fonts->AddFontFromMemoryCompressedTTF(
              asap::debug::ui::Fonts::INCONSOLATA_BOLD_COMPRESSED_DATA,
              asap::debug::ui::Fonts::INCONSOLATA_BOLD_COMPRESSED_SIZE,
              Font::SizeFloat(size), &fontConfig);
          font = MergeIcons(Font::SizeFloat(size));
          break;
      }
      break;
  }
  return font;
}

ImFont *LoadIconsFont(float size) {
  ImGuiIO &io = ImGui::GetIO();
  ImFontConfig fontConfig;
  // Set Oversamping parameters to 1 on both axis, the texture will be 6 times
  // smaller. See https://github.com/ocornut/imgui/issues/1527
  fontConfig.OversampleH = 1;
  fontConfig.OversampleV = 1;
  fontConfig.MergeMode = false;
  std::strncpy(fontConfig.Name, "Material Design Icons",
               sizeof(fontConfig.Name) - 1);
  fontConfig.Name[sizeof(fontConfig.Name) - 1] = 0;
  ImFont *font = nullptr;
  font = io.Fonts->AddFontFromMemoryCompressedTTF(
      asap::debug::ui::Fonts::MATERIAL_DESIGN_ICONS_COMPRESSED_DATA,
      asap::debug::ui::Fonts::MATERIAL_DESIGN_ICONS_COMPRESSED_SIZE, size,
      &fontConfig);
  return font;
}

//}  // namespace

Font::Font(std::string family) : family_(std::move(family)) { InitFont(); }

void Font::InitFont() {
  BuildName();
  font_ = Theme::GetFont(name_);
  name_.assign(font_->GetDebugName());
}

Font::Font(Font const &other)
    : font_(other.font_),
      size_(other.size_),
      style_(other.style_),
      weight_(other.weight_),
      name_(other.name_) {}

Font &Font::operator=(Font const &rhs) {
  font_ = rhs.font_;
  size_ = rhs.size_;
  style_ = rhs.style_;
  weight_ = rhs.weight_;
  name_ = rhs.name_;
  return *this;
}
Font::Font(Font &&moved) noexcept
    : font_(moved.font_),
      size_(moved.size_),
      style_(moved.style_),
      weight_(moved.weight_),
      name_(std::move(moved.name_)) {
  moved.font_ = nullptr;
}
Font &Font::operator=(Font &&moved) noexcept {
  auto tmp = std::move(moved);
  swap(tmp);
  return *this;
}
void Font::swap(Font &other) {
  std::swap(font_, other.font_);
  std::swap(size_, other.size_);
  std::swap(style_, other.style_);
  std::swap(weight_, other.weight_);
  name_.swap(other.name_);
}

void Font::BuildName() {
  name_ = BuildFontName(family_, weight_, style_, size_);
}

Font &Font::SmallSize() {
  size_ = Size::SMALL;
  InitFont();
  return *this;
}
Font &Font::MediumSize() {
  size_ = Size::MEDIUM;
  InitFont();
  return *this;
}
Font &Font::LargeSize() {
  size_ = Size::LARGE;
  InitFont();
  return *this;
}
Font &Font::LargerSize() {
  size_ = Size::LARGER;
  InitFont();
  return *this;
}

Font &Font::Normal() {
  style_ = Style::NORMAL;
  InitFont();
  return *this;
}

Font &Font::Italic() {
  style_ = Style::ITALIC;
  InitFont();
  return *this;
}

Font &Font::Light() {
  weight_ = Weight::LIGHT;
  InitFont();
  return *this;
}
Font &Font::Regular() {
  weight_ = Weight::REGULAR;
  InitFont();
  return *this;
}
Font &Font::Bold() {
  weight_ = Weight::BOLD;
  InitFont();
  return *this;
}

float Font::SizeFloat(Font::Size size) {
  auto val = static_cast<typename std::underlying_type<Font::Size>::type>(size);
  return static_cast<float>(val);
}
char const *Font::SizeString(Font::Size size) {
  switch (size) {
    case Size::SMALL:
      return "11px";
    case Size::MEDIUM:
      return "13px";
    case Size::LARGE:
      return "16px";
    case Size::LARGER:
      return "24px";
  }
  // Only needed for compilers that complain about not all control paths
  // return a value.
  return "__NEVER__";
}

char const *Font::StyleString(Font::Style style) {
  switch (style) {
    case Style::NORMAL:
      return "Normal";
    case Style::ITALIC:
      return "Italic";
  }
  // Only needed for compilers that complain about not all control paths
  // return a value.
  return "__NEVER__";
}

char const *Font::WeightString(Font::Weight weight) {
  switch (weight) {
    case Weight::LIGHT:
      return "Light";
    case Weight::REGULAR:
      return "Regular";
    case Weight::BOLD:
      return "Bold";
  }
  // Only needed for compilers that complain about not all control paths
  // return a value.
  return "__NEVER__";
}

}  // namespace ui
}  // namespace asap
