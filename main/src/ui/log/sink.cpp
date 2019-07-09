//    Copyright The asap Project Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <array>
#include <fstream>
#include <sstream>  // for log record formatting

#include <date/date.h>  // for time formatting
// Disable warning generated by yaml-cpp
#include <asap/asap-features.h>
#if ASAP_COMPILER_IS_MSVC
__pragma(warning(push)) __pragma(warning(disable : 4127))
#endif
#include <cpptoml.h>
#if ASAP_COMPILER_IS_MSVC
    __pragma(warning(pop))
#endif

#include <ui/fonts/material_design_icons.h>
#include <ui/log/sink.h>
#include <ui/style/font.h>

#include <common/assert.h>
#include <common/logging.h>
#include <config.h>

namespace asap {
  namespace ui {

  const char *ImGuiLogSink::LOGGER_NAME = "main";

  const ImVec4 ImGuiLogSink::COLOR_WARN{0.9f, 0.7f, 0.0f, 1.0f};
  const ImVec4 ImGuiLogSink::COLOR_ERROR{1.0f, 0.0f, 0.0f, 1.0f};

  void ImGuiLogSink::Clear() {
    std::unique_lock<std::shared_timed_mutex> lock(records_mutex_);
    records_.clear();
  }

  void ImGuiLogSink::ShowLogLevelsPopup() {
    ImGui::MenuItem("Logging Levels", nullptr, false, false);

    std::vector<int> levels;
    for (auto &a_logger : asap::logging::Registry::Loggers()) {
      levels.push_back(a_logger.second.GetLevel());
      auto format = std::string("%u (")
                        .append(spdlog::level::to_string_view(a_logger.second.GetLevel()).data())
                        .append(")");
      if (ImGui::SliderInt(a_logger.second.Name().c_str(), &levels.back(), 0, 6,
                           format.c_str())) {
        a_logger.second.SetLevel(spdlog::level::level_enum(levels.back()));
      }
    }
  }

  void ImGuiLogSink::ShowLogFormatPopup() {
    ImGui::MenuItem("Logging Format", nullptr, false, false);
    ImGui::Checkbox("Time", &show_time_);
    ImGui::SameLine();
    ImGui::Checkbox("Thread", &show_thread_);
    ImGui::SameLine();
    ImGui::Checkbox("Level", &show_level_);
    ImGui::SameLine();
    ImGui::Checkbox("Logger", &show_logger_);
  }

  void ImGuiLogSink::Draw(const char *title, bool *open) {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

    // This is the case when the log viewer is supposed to open in its own ImGui
    // window (not docked).
    if (open) {
      ASAP_ASSERT(title != nullptr);
      ImGui::Begin(title, open);
    }

    /*
    if (ImGui::Button("Log something...")) {
      BXLOG_MISC(trace, "TRACE");
      BXLOG_MISC(debug, "DEBUG");
      BXLOG_MISC(info, "INFO");
      BXLOG_MISC(warn, "WARN");
      BXLOG_MISC(error, "ERROR");
      BXLOG_MISC(critical, "CRITICAL");
    }
    */

    // -------------------------------------------------------------------------
    // Toolbar
    // -------------------------------------------------------------------------

    {
      // Make all buttons transparent in the toolbar
      auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
      button_color.w = 0.0f;
      ImGui::PushStyleColor(ImGuiCol_Button, button_color);

      if (ImGui::Button(ICON_MDI_SETTINGS " Levels")) {
        ImGui::OpenPopup("LogLevelsPopup");
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Change the logging levels");
      }
      if (ImGui::BeginPopup("LogLevelsPopup")) {
        ShowLogLevelsPopup();
        ImGui::EndPopup();
      }

      ImGui::SameLine();
      if (ImGui::Button(ICON_MDI_VIEW_COLUMN " Format")) {
        ImGui::OpenPopup("LogFormatPopup");
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Chose what information to show");
      }
      if (ImGui::BeginPopup("LogFormatPopup")) {
        ShowLogFormatPopup();
        ImGui::EndPopup();
      }

      ImGui::SameLine();
      if (ImGui::Button(ICON_MDI_NOTIFICATION_CLEAR_ALL " Clear")) Clear();
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Discard all messages");
      }

      ImGui::SameLine();
      bool need_pop_style_var = false;
      if (wrap_) {
        // Highlight the button
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::PushStyleColor(
            ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
        need_pop_style_var = true;
      }
      if (ImGui::Button(ICON_MDI_WRAP)) ToggleWrap();
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle soft wraps");
      }
      if (need_pop_style_var) {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
      }

      ImGui::SameLine();
      need_pop_style_var = false;
      if (scroll_lock_) {
        // Highlight the button
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        ImGui::PushStyleColor(
            ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
        need_pop_style_var = true;
      }
      if (ImGui::Button(ICON_MDI_LOCK)) ToggleScrollLock();
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle automatic scrolling to the bottom");
      }
      if (need_pop_style_var) {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
      }

      ImGui::SameLine();
      display_filter_.Draw(ICON_MDI_FILTER " Filter", -100.0f);
    }
    // Restore the button color
    ImGui::PopStyleColor();

    // -------------------------------------------------------------------------
    // Log records
    // -------------------------------------------------------------------------

    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    {
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

      Font font("Inconsolata");
      font.MediumSize();

      std::shared_lock<std::shared_timed_mutex> lock(records_mutex_);
      for (auto const &record : records_) {
        if (!display_filter_.IsActive() ||
            display_filter_.PassFilter(
                record.properties_.c_str(),
                record.properties_.c_str() + record.properties_.size()) ||
            display_filter_.PassFilter(
                record.source_.c_str(),
                record.source_.c_str() + record.source_.size()) ||
            display_filter_.PassFilter(
                record.message_.c_str(),
                record.message_.c_str() + record.message_.size())) {
          ImGui::BeginGroup();
          if (record.emphasis_) {
            font.Bold();
          } else {
            font.Regular();
          }
          ImGui::PushFont(font.ImGuiFont());

          if (record.color_range_start_ > 0) {
            auto props_len = record.properties_.size();

            ASAP_ASSERT_VAL(record.color_range_start_ < props_len,
                            record.color_range_start_);
            ASAP_ASSERT(record.color_range_end_ > record.color_range_start_);
            ASAP_ASSERT_VAL(record.color_range_end_ < props_len,
                            record.color_range_end_);

            std::string part =
                record.properties_.substr(0, record.color_range_start_);
            ImGui::TextUnformatted(part.c_str());
            ImGui::SameLine();

            part = record.properties_.substr(
                record.color_range_start_,
                record.color_range_end_ - record.color_range_start_);
            ImGui::TextColored(record.color_, "%s", part.c_str());

            part = record.properties_.substr(
                record.color_range_end_, props_len - record.color_range_end_);
            ImGui::SameLine();
            ImGui::TextUnformatted(part.c_str());

          } else {
            if (record.color_range_end_ == 1) {
              ImGui::TextColored(record.color_, "%s",
                                 record.properties_.c_str());
            } else {
              ImGui::TextUnformatted(record.properties_.c_str());
            }
          }

          if (record.color_range_end_ == 1) {
            ImGui::SameLine();
            if (wrap_) ImGui::PushTextWrapPos(0.0f);
            ImGui::TextColored(record.color_, "%s", record.message_.c_str());
            if (wrap_) ImGui::PopTextWrapPos();
          } else {
            ImGui::SameLine();
            if (wrap_) ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(record.message_.c_str());
            if (wrap_) ImGui::PopTextWrapPos();
          }
          ImGui::EndGroup();
#ifndef NDEBUG
          // We only show the tooltip with the source location if in debug build. The source
          // location information is not produced in the logs in non-debug builds.
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", record.source_.c_str());
          }
#endif // NDEBUG

          ImGui::PopFont();
        }
      }
    }
    ImGui::PopStyleVar();

    if (!scroll_lock_ && scroll_to_bottom_) ImGui::SetScrollHere(1.0f);
    scroll_to_bottom_ = false;
    ImGui::EndChild();

    // The case of the log viewer in its own ImGui window (not docked)
    if (open) {
      ImGui::End();
    }
  }

  void ImGuiLogSink::sink_it_(const spdlog::details::log_msg &msg) {
    auto ostr = std::ostringstream();
    std::size_t color_range_start = 0;
    std::size_t color_range_end = 0;
    ImVec4 const *color = nullptr;
    auto emphasis = false;

    if (show_time_) {
      ostr << "[" << date::format("%D %T %Z", msg.time) << "] ";
    }
    if (show_thread_) {
      ostr << "[" << msg.thread_id << "] ";
    }
    if (show_level_) {
      color_range_start = static_cast<std::size_t>(ostr.tellp());
      ostr << "[" << spdlog::level::to_short_c_str(msg.level) << "] ";
      color_range_end = static_cast<std::size_t>(ostr.tellp());
    }
    if (show_logger_) {
      ostr << "[" << *msg.logger_name << "] ";
    }
    auto properties = ostr.str();

    // Strip the filename:line from the message and put it in a separate string
    auto msg_str = std::string(msg.payload.data(), msg.payload.size());
    auto skip_to = msg_str.begin();
    if (*skip_to == '[') {
      // skip spaces
      while (skip_to != msg_str.end() && *skip_to != ']') ++skip_to;
      if (skip_to == msg_str.end()) {
        skip_to = msg_str.begin();
      } else {
        auto saved_skip_to = skip_to + 1;
        while (skip_to != msg_str.begin()) {
          --skip_to;
          if (*skip_to == ':' || !std::isdigit(*skip_to)) break;
        }
        if (*skip_to == ':') skip_to = ++saved_skip_to;  // Add one space
      }
    }
    // Source location will be found only in non-debug builds.
#ifndef NDEBUG
    // Ignore the '[' and ']'
    auto source = msg_str.substr(1, skip_to - msg_str.begin() - 3);
#else
    auto source = std::string();
#endif // NDEBUG
    // Select display color and colored text range based on level
    switch (msg.level) {
      case spdlog::level::trace:
        color = &ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
        // the entire message
        color_range_start = 0;
        color_range_end = 1;
        break;

      case spdlog::level::debug:
        color = &ImGui::GetStyleColorVec4(ImGuiCol_Text);
        // The level part if show, otherwise no coloring
        break;

      case spdlog::level::info:
        color = &ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
        // The level part if show, otherwise no coloring
        break;

      case spdlog::level::warn:
        color = &COLOR_WARN;
        // the entire message
        color_range_start = 0;
        color_range_end = 1;
        break;

      case spdlog::level::err:
        color = &COLOR_ERROR;
        // the entire message
        color_range_start = 0;
        color_range_end = 1;
        break;

      case spdlog::level::critical:
        color = &COLOR_ERROR;
        emphasis = true;
        // the entire message
        color_range_start = 0;
        color_range_end = 1;
        break;

      default:
          // Nothing
          ;
    }

    auto record = LogRecord{properties,
                            source,
                            msg_str.substr(skip_to - msg_str.begin()),
                            color_range_start,
                            color_range_end,
                            *color,
                            emphasis};
    {
      std::unique_lock<std::shared_timed_mutex> lock(records_mutex_);
      records_.push_back(std::move(record));
    }
    scroll_to_bottom_ = true;
  }

  void ImGuiLogSink::flush_() {
    // Your code here
  }

  namespace {
  void ConfigSanityChecks(std::shared_ptr<cpptoml::table> &config) {
    auto &logger = asap::logging::Registry::GetLogger("main");

    auto loggers = config->get_table_array("loggers");
    if (!loggers) {
      ASLOG_TO_LOGGER(logger, warn, "missing 'loggers' in config");
    }
    auto format = config->get_table("format");
    if (!format) {
      ASLOG_TO_LOGGER(logger, warn, "missing 'format' in config");
    } else {
      if (!format->contains("show-time")) {
        ASLOG_TO_LOGGER(logger, warn, "missing 'format/show-time' in config");
      }
      if (!format->contains("show-thread")) {
        ASLOG_TO_LOGGER(logger, warn, "missing 'format/show-thread' in config");
      }
      if (!format->contains("show-logger")) {
        ASLOG_TO_LOGGER(logger, warn, "missing 'format/show-logger' in config");
      }
      if (!format->contains("show-level")) {
        ASLOG_TO_LOGGER(logger, warn, "missing 'format/show-level' in config");
      }
    }
    if (!config->contains("scroll-lock")) {
      ASLOG_TO_LOGGER(logger, warn, "missing 'scroll-lock' in config");
    }
    if (!config->contains("soft-wrap")) {
      ASLOG_TO_LOGGER(logger, warn, "missing 'soft-wrap' in config");
    }
  }
  }  // namespace

  void ImGuiLogSink::LoadSettings() {
    std::shared_ptr<cpptoml::table> config;
    auto log_settings =
        asap::fs::GetPathFor(asap::fs::Location::F_LOG_SETTINGS);
    auto has_config = false;
    if (asap::filesystem::exists(log_settings)) {
      try {
        config = cpptoml::parse_file(log_settings.string());
        ASLOG(info, "settings loaded from {}", log_settings.string());
        has_config = true;
      } catch (std::exception const &ex) {
        ASLOG(error, "error () while loading settings from {}", ex.what(),
              log_settings.string());
      }
    } else {
      ASLOG(info, "file {} does not exist", log_settings);
    }

    if (has_config) {
      ConfigSanityChecks(config);

      auto loggers = config->get_table_array("loggers");
      if (loggers) {
        for (auto const &logger_settings : *loggers) {
          ASLOG(debug, "logger '{}' will have level '{}'",
                *(logger_settings->get_as<std::string>("name")),
                *(logger_settings->get_as<int>("level")));
          auto &logger =
              asap::logging::Registry::GetLogger(*(logger_settings->get_as<std::string>("name")));
          logger.set_level(static_cast<spdlog::level::level_enum>(
              *(logger_settings->get_as<int>("level"))));
        }
      }

      auto format = config->get_table("format");
      if (format) {
        if (format->contains("show-time")) {
          show_time_ = *(format->get_as<bool>("show-time"));
        }
        if (format->contains("show-thread")) {
          show_thread_ = *(format->get_as<bool>("show-thread"));
        }
        if (format->contains("show-logger")) {
          show_logger_ = *(format->get_as<bool>("show-logger"));
        }
        if (format->contains("show-level")) {
          show_level_ = *(format->get_as<bool>("show-level"));
        }
      }

      if (format->contains("scroll-lock")) {
        scroll_lock_ = *(format->get_as<bool>("scroll-lock"));
      }
      if (format->contains("soft-wrap")) {
        wrap_ = *(format->get_as<bool>("soft-wrap"));
      }
    }
  }

  void ImGuiLogSink::SaveSettings() {
    std::shared_ptr<cpptoml::table> root = cpptoml::make_table();

    auto loggers = cpptoml::make_table_array();

    for (auto &log : logging::Registry::Loggers()) {
      auto logcfg = cpptoml::make_table();
      logcfg->insert(
          "name",
          log.second.Name());
      logcfg->insert(
          "level",
          static_cast<
              typename std::underlying_type<spdlog::level::level_enum>::type>(
              log.second.GetLevel()));
      loggers->push_back(logcfg);
    }

    root->insert("loggers", loggers);

    auto format = cpptoml::make_table();
    format->insert("show-time", show_time_);
    format->insert("show-thread", show_thread_);
    format->insert("show-level", show_level_);
    format->insert("show-logger", show_logger_);
    root->insert("format", format);

    root->insert("scroll-lock", scroll_lock_);
    root->insert("soft-wrap", wrap_);

    auto settings_path =
        asap::fs::GetPathFor(asap::fs::Location::F_LOG_SETTINGS);
    auto ofs = std::ofstream();
    ofs.open(settings_path.string());
    ofs << "# Logging configuration (toml 0.5.1)" << std::endl;
    ofs << (*root) << std::endl;
    ofs.close();
  }

  }  // namespace ui
}  // namespace asap
