module;

#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "pludux_icons.hpp"

#include <imgui.h>

export module pludux.apps.backtest:ui.widgets;

namespace pludux::apps::ui {

// Renders `label` as a button right-aligned within the remaining width of
// the current line. Replaces the CalcTextSize/FramePadding/
// SetCursorScreenPos boilerplate that used to be duplicated at every delete
// button call site.
export auto right_aligned_button(const char* label) -> bool
{
  const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
  const auto button_width =
   ImGui::CalcTextSize(label).x + (2.0f * frame_padding_x);
  const auto line_start = ImGui::GetCursorScreenPos();
  const auto line_width = ImGui::GetContentRegionAvail().x;

  ImGui::SetCursorScreenPos(
   ImVec2(line_start.x + line_width - button_width, line_start.y));

  return ImGui::Button(label);
}

// Renders a compact icon action while keeping its text description available
// to mouse users and assistive UI tooling through a tooltip.
export auto icon_button(const char* label, const char* description) -> bool
{
  const auto pressed = ImGui::Button(label);
  if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
    ImGui::SetTooltip("%s", description);
  }
  return pressed;
}

export enum class DraftAction { None, Apply, Reset };

export auto apply_reset_button(bool changed, bool can_apply = true)
 -> DraftAction
{
  auto action = DraftAction::None;
  const auto* primary_label = PLUDUX_ICON_SAVE " Apply";
  const auto& style = ImGui::GetStyle();
  const auto primary_size = ImGui::CalcTextSize(primary_label);
  const auto arrow_size = ImGui::CalcTextSize(PLUDUX_ICON_CHEVRON_DOWN);
  const auto arrow_region_width = arrow_size.x + (2.0f * style.FramePadding.x);
  const auto button_size =
   ImVec2{primary_size.x + arrow_region_width + (2.0f * style.FramePadding.x),
          ImGui::GetFrameHeight()};
  const auto button_min = ImGui::GetCursorScreenPos();
  const auto pressed =
   ImGui::InvisibleButton("##apply_reset_button", button_size);
  const auto button_max =
   ImVec2{button_min.x + button_size.x, button_min.y + button_size.y};
  const auto arrow_region_x = button_max.x - arrow_region_width;

  const auto hovered = ImGui::IsItemHovered();
  const auto held = ImGui::IsItemActive();
  const auto background_color = ImGui::GetColorU32(
   held ? ImGuiCol_ButtonActive
        : (hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
  auto* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(
   button_min, button_max, background_color, style.FrameRounding);

  const auto primary_enabled = changed && can_apply;
  const auto text_color =
   ImGui::GetColorU32(primary_enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);
  const auto primary_position =
   ImVec2{button_min.x + style.FramePadding.x,
          button_min.y + ((button_size.y - primary_size.y) * 0.5f)};
  draw_list->AddText(primary_position, text_color, primary_label);

  const auto arrow_position =
   ImVec2{arrow_region_x + ((arrow_region_width - arrow_size.x) * 0.5f),
          button_min.y + ((button_size.y - arrow_size.y) * 0.5f)};
  draw_list->AddText(
   arrow_position, ImGui::GetColorU32(ImGuiCol_Text), PLUDUX_ICON_CHEVRON_DOWN);

  const auto arrow_hovered =
   hovered && ImGui::GetMousePos().x >= arrow_region_x;
  if(pressed && arrow_hovered &&
     ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
    ImGui::OpenPopup("apply_or_reset_draft");
  } else if(pressed && primary_enabled) {
    action = DraftAction::Apply;
  }

  if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
    if(arrow_hovered) {
      ImGui::SetTooltip("Choose the draft action");
    } else if(primary_enabled) {
      ImGui::SetTooltip("Apply the current draft");
    } else if(changed && !can_apply) {
      ImGui::SetTooltip("Complete the required fields before applying");
    }
  }

  if(ImGui::BeginPopup("apply_or_reset_draft")) {
    if(ImGui::MenuItem(PLUDUX_ICON_RESET " Reset", nullptr, false, changed)) {
      action = DraftAction::Reset;
    }
    ImGui::EndPopup();
  }
  return action;
}

export auto search_filter(ImGuiTextFilter& filter, const char* id) -> bool
{
  ImGui::SetNextItemWidth(-1.0f);
  const auto changed = ImGui::InputTextWithHint(id,
                                                PLUDUX_ICON_SEARCH " Search...",
                                                filter.InputBuf,
                                                sizeof(filter.InputBuf));
  if(changed) {
    filter.Build();
  }
  return changed;
}

namespace {

constexpr auto minimum_control_width = 220.0f;

auto uses_stacked_field_layout(float label_width) -> bool
{
  return ImGui::GetContentRegionAvail().x < label_width + minimum_control_width;
}

} // namespace

export void form_section(const char* title, const char* description)
{
  ImGui::Spacing();
  ImGui::SeparatorText(title);
  ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                         ImGui::GetContentRegionAvail().x);
  ImGui::TextDisabled("%s", description);
  ImGui::PopTextWrapPos();
  ImGui::Spacing();
}

export void section_description(const char* description)
{
  ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                         ImGui::GetContentRegionAvail().x);
  ImGui::TextDisabled("%s", description);
  ImGui::PopTextWrapPos();
  ImGui::Spacing();
}

export void field_label(const char* label, float label_width = 180.0f)
{
  const auto row_start_x = ImGui::GetCursorPosX();
  const auto stacked = uses_stacked_field_layout(label_width);
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(label);
  if(!stacked) {
    ImGui::SameLine();
    ImGui::SetCursorPosX(
     std::max(ImGui::GetCursorPosX(), row_start_x + label_width));
  }
  ImGui::SetNextItemWidth(-1.0f);
}

export void field_label(const char* label,
                        const char* description,
                        float label_width = 180.0f)
{
  const auto row_start_x = ImGui::GetCursorPosX();
  const auto stacked = uses_stacked_field_layout(label_width);
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(label);
  ImGui::SameLine(0.0f, 4.0f);
  ImGui::TextDisabled(PLUDUX_ICON_HELP);
  if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f);
    ImGui::TextUnformatted(description);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
  if(!stacked) {
    ImGui::SameLine();
    ImGui::SetCursorPosX(
     std::max(ImGui::GetCursorPosX(), row_start_x + label_width));
  }
  ImGui::SetNextItemWidth(-1.0f);
}

export void validation_message(const char* message)
{
  const auto& style = ImGui::GetStyle();
  ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_PlotHistogram]);
  ImGui::TextWrapped("%s", message);
  ImGui::PopStyleColor();
}

export enum class ResourceRowAction {
  None,
  Select,
  Edit,
  Duplicate,
  Rerun,
  MoveUp,
  MoveDown,
  Delete,
};

export auto resource_row_actions(ImVec2 row_start,
                                 float row_width,
                                 std::size_t index,
                                 std::size_t count,
                                 bool show_rerun = false,
                                 bool can_rerun = false) -> ResourceRowAction
{
  auto action = ResourceRowAction::None;
  const auto& style = ImGui::GetStyle();
  const auto spacing = style.ItemSpacing.x;
  const auto edit_width =
   ImGui::CalcTextSize(PLUDUX_ICON_EDIT).x + (2.0f * style.FramePadding.x);
  const auto more_width =
   ImGui::CalcTextSize(PLUDUX_ICON_MORE).x + (2.0f * style.FramePadding.x);
  const auto buttons_width = edit_width + spacing + more_width;
  const auto buttons_start_x =
   std::max(row_start.x, row_start.x + row_width - buttons_width);
  ImGui::SetCursorScreenPos(ImVec2{buttons_start_x, row_start.y});

  if(icon_button(PLUDUX_ICON_EDIT "##edit", "Edit")) {
    action = ResourceRowAction::Edit;
  }
  ImGui::SameLine();
  if(icon_button(PLUDUX_ICON_MORE "##more", "More actions")) {
    ImGui::OpenPopup("resource_actions");
  }
  if(ImGui::BeginPopup("resource_actions")) {
    if(show_rerun) {
      if(ImGui::MenuItem(
          PLUDUX_ICON_RESET " Rerun Backtests", nullptr, false, can_rerun)) {
        action = ResourceRowAction::Rerun;
      }
      ImGui::Separator();
    }
    if(ImGui::MenuItem(PLUDUX_ICON_EDIT " Edit")) {
      action = ResourceRowAction::Edit;
    }
    if(ImGui::MenuItem(PLUDUX_ICON_COPY " Duplicate")) {
      action = ResourceRowAction::Duplicate;
    }
    ImGui::Separator();
    if(ImGui::MenuItem(
        PLUDUX_ICON_MOVE_UP " Move Up", nullptr, false, index > 0)) {
      action = ResourceRowAction::MoveUp;
    }
    if(ImGui::MenuItem(PLUDUX_ICON_MOVE_DOWN " Move Down",
                       nullptr,
                       false,
                       index + 1 < count)) {
      action = ResourceRowAction::MoveDown;
    }
    ImGui::Separator();
    if(ImGui::MenuItem(PLUDUX_ICON_DELETE " Delete")) {
      action = ResourceRowAction::Delete;
    }
    ImGui::EndPopup();
  }
  return action;
}

// A responsive resource row shared by the library windows. It keeps the
// primary edit action visible and moves secondary actions into a labeled menu.
export auto resource_row(const char* name,
                         bool selected,
                         std::size_t index,
                         std::size_t count) -> ResourceRowAction
{
  auto action = ResourceRowAction::None;
  const auto row_start = ImGui::GetCursorScreenPos();
  const auto row_width = ImGui::GetContentRegionAvail().x;
  const auto row_height = ImGui::GetFrameHeight();

  ImGui::SetNextItemAllowOverlap();
  const auto clicked = ImGui::Selectable("##resource_row",
                                         selected,
                                         ImGuiSelectableFlags_AllowDoubleClick |
                                          ImGuiSelectableFlags_AllowOverlap,
                                         ImVec2{row_width, row_height});
  if(clicked) {
    action = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
              ? ResourceRowAction::Edit
              : ResourceRowAction::Select;
  }

  ImGui::SetCursorScreenPos(row_start);
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(name);

  const auto control_action =
   resource_row_actions(row_start, row_width, index, count);
  if(control_action != ResourceRowAction::None) {
    action = control_action;
  }

  ImGui::Separator();
  return action;
}

// Thin wrapper over ImGui::CollapsingHeader so section open/closed styling
// stays consistent across the strategies window. Open/closed state persists
// automatically via ImGui's window settings (see
// WindowContext::update_imgui_ini_settings), keyed by `label`.
export auto collapsible_section(const char* label, bool default_open = true)
 -> bool
{
  const auto flags =
   default_open ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None;
  return ImGui::CollapsingHeader(label, flags);
}

// A single entry in a searchable_combo popup.
export struct ComboEntry {
  std::string id;
  std::string title;
  std::string category;
};

// Renders a searchable, category-grouped combo box. `entries` must already
// be ordered by category (entries sharing a category should be adjacent) so
// that category separators are only drawn once per group.
//
// The search filter is keyed internally by the combo's ImGui id, so every
// combo instance (including multiple recursive instances of the same
// combo function, e.g. nested series expressions) keeps its own
// independent search state. This fixes the previous bug where a single
// `static ImGuiTextFilter` local to a combo-drawing function was shared by
// every call site of that function.
//
// Returns the id of the newly selected entry, or std::nullopt if the
// selection did not change this frame.
export auto searchable_combo(const char* combo_label,
                             const std::string& current_id,
                             const std::string& current_title,
                             const std::vector<ComboEntry>& entries)
 -> std::optional<std::string>
{
  static auto filters = std::unordered_map<ImGuiID, ImGuiTextFilter>{};

  auto selected_id = std::optional<std::string>{};

  if(ImGui::BeginCombo(combo_label, current_title.c_str())) {
    auto& filter = filters[ImGui::GetID(combo_label)];

    if(ImGui::IsWindowAppearing()) {
      ImGui::SetKeyboardFocusHere();
      filter.Clear();
    }

    filter.Draw("##filter", -FLT_MIN);

    auto last_category = std::string{};
    for(const auto& entry : entries) {
      if(!filter.PassFilter(entry.title.c_str())) {
        continue;
      }

      if(entry.category != last_category) {
        ImGui::SeparatorText(entry.category.c_str());
        last_category = entry.category;
      }

      const auto is_selected = entry.id == current_id;
      if(ImGui::Selectable(entry.title.c_str(), is_selected)) {
        selected_id = entry.id;
      }

      if(is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }

    ImGui::EndCombo();
  }

  return selected_id;
}

} // namespace pludux::apps::ui
