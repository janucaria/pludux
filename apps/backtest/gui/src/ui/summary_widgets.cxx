module;

#include <algorithm>
#include <span>
#include <string>
#include <string_view>

#include "pludux_icons.hpp"

#include <imgui.h>

export module pludux.apps.backtest:ui.summary_widgets;

export namespace pludux::apps::ui {

enum class MetricTone { Neutral, Positive, Negative, Warning };

struct SummaryMetric {
  std::string label;
  std::string value;
  std::string detail;
  std::string description;
  MetricTone tone{MetricTone::Neutral};
};

} // namespace pludux::apps::ui

namespace pludux::apps::ui {

namespace {

auto tone_color(MetricTone tone) -> ImVec4
{
  switch(tone) {
  case MetricTone::Positive:
    return ImVec4{0.38f, 0.78f, 0.50f, 1.0f};
  case MetricTone::Negative:
    return ImVec4{0.91f, 0.40f, 0.40f, 1.0f};
  case MetricTone::Warning:
    return ImVec4{0.93f, 0.67f, 0.28f, 1.0f};
  case MetricTone::Neutral:
    return ImGui::GetStyleColorVec4(ImGuiCol_Text);
  }
  return ImGui::GetStyleColorVec4(ImGuiCol_Text);
}

void description_tooltip(std::string_view description)
{
  if(description.empty() ||
     !ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
    return;
  }

  ImGui::BeginTooltip();
  ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
  ImGui::TextUnformatted(description.data(),
                         description.data() + description.size());
  ImGui::PopTextWrapPos();
  ImGui::EndTooltip();
}

} // namespace

} // namespace pludux::apps::ui

export namespace pludux::apps::ui {

void summary_status(const char* icon,
                    std::string_view title,
                    std::string_view message)
{
  const auto available = ImGui::GetContentRegionAvail();
  const auto panel_height = std::max(150.0f, ImGui::GetTextLineHeight() * 8.0f);
  const auto top_padding =
   std::max(24.0f, (available.y - panel_height) * 0.35f);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + top_padding);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{20.0f, 20.0f});
  ImGui::PushStyleColor(ImGuiCol_ChildBg,
                        ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
  ImGui::BeginChild("##summary_status",
                    ImVec2{0.0f, panel_height},
                    ImGuiChildFlags_Borders |
                     ImGuiChildFlags_AlwaysUseWindowPadding);
  ImGui::PushStyleColor(ImGuiCol_Text,
                        ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
  ImGui::SetWindowFontScale(1.35f);
  ImGui::TextUnformatted(icon);
  ImGui::SetWindowFontScale(1.0f);
  ImGui::PopStyleColor();
  ImGui::Spacing();
  ImGui::TextUnformatted(title.data(), title.data() + title.size());
  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_Text,
                        ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
  ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                         ImGui::GetContentRegionAvail().x);
  ImGui::TextUnformatted(message.data(), message.data() + message.size());
  ImGui::PopTextWrapPos();
  ImGui::PopStyleColor();
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
}

void summary_kpi_card(const char* id,
                      std::string_view label,
                      std::string_view value,
                      std::string_view detail,
                      MetricTone tone,
                      float width)
{
  const auto accent = tone_color(tone);
  auto background = ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg);
  if(tone != MetricTone::Neutral) {
    constexpr auto accent_weight = 0.10f;
    background.x =
     (background.x * (1.0f - accent_weight)) + (accent.x * accent_weight);
    background.y =
     (background.y * (1.0f - accent_weight)) + (accent.y * accent_weight);
    background.z =
     (background.z * (1.0f - accent_weight)) + (accent.z * accent_weight);
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{14.0f, 12.0f});
  ImGui::PushStyleColor(ImGuiCol_ChildBg, background);
  ImGui::BeginChild(
   id,
   ImVec2{width, 94.0f},
   ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
   ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  ImGui::TextDisabled("%.*s", static_cast<int>(label.size()), label.data());
  ImGui::PushStyleColor(ImGuiCol_Text, accent);
  ImGui::SetWindowFontScale(1.18f);
  ImGui::TextWrapped("%.*s", static_cast<int>(value.size()), value.data());
  ImGui::SetWindowFontScale(1.0f);
  ImGui::PopStyleColor();
  if(!detail.empty()) {
    ImGui::TextDisabled("%.*s", static_cast<int>(detail.size()), detail.data());
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
}

void summary_metric_section(const char* id,
                            const char* icon,
                            std::string_view title,
                            std::string_view description,
                            std::span<const SummaryMetric> metrics)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{12.0f, 12.0f});
  ImGui::PushStyleColor(ImGuiCol_ChildBg,
                        ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
  ImGui::BeginChild(id,
                    ImVec2{0.0f, 0.0f},
                    ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY |
                     ImGuiChildFlags_AlwaysUseWindowPadding);

  ImGui::Text("%s  %.*s", icon, static_cast<int>(title.size()), title.data());
  if(!description.empty()) {
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                           ImGui::GetContentRegionAvail().x);
    ImGui::TextDisabled(
     "%.*s", static_cast<int>(description.size()), description.data());
    ImGui::PopTextWrapPos();
  }
  ImGui::Spacing();

  const auto table_flags =
   ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
   ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings;
  if(ImGui::BeginTable("##metrics", 2, table_flags)) {
    ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch, 1.0f);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.85f);

    for(const auto& metric : metrics) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      ImGui::TextUnformatted(metric.label.c_str());
      description_tooltip(metric.description);
      if(!metric.description.empty()) {
        ImGui::SameLine(0.0f, 4.0f);
        ImGui::TextDisabled(PLUDUX_ICON_HELP);
        description_tooltip(metric.description);
      }

      ImGui::TableNextColumn();
      ImGui::PushStyleColor(ImGuiCol_Text, tone_color(metric.tone));
      ImGui::TextWrapped("%s", metric.value.c_str());
      ImGui::PopStyleColor();
      if(!metric.detail.empty()) {
        ImGui::TextDisabled("%s", metric.detail.c_str());
      }
    }
    ImGui::EndTable();
  }

  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
}

} // namespace pludux::apps::ui
