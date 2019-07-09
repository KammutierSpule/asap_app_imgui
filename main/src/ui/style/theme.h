//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#pragma once

#include <map>
#include <string>

#ifndef THEME_USE_LOAD_SAVE
#define THEME_USE_LOAD_SAVE 1
#endif

struct ImFont;

namespace asap {
namespace ui {

class Theme {
 public:
  static void Init();

  static ImFont *GetFont(std::string const &name);

  static ImFont *GetIconsFont() { return icons_font_normal_; }

#if (THEME_USE_LOAD_SAVE)
  static void SaveStyle();
  static void LoadStyle();
#endif

  static void LoadDefaultStyle();

 private:
  Theme() = default;

  static void LoadDefaultFonts();

  static void AddFont(std::string const &name, ImFont *font);

  static std::map<std::string, ImFont *> fonts_;
  static ImFont *icons_font_normal_;
};

}  // namespace ui
}  // namespace asap
