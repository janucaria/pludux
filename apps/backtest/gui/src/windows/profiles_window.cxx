module;

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.profiles_window;

import pludux.backtest;
import :window_context;

export namespace pludux::apps {

class ProfilesWindow {
public:
  ProfilesWindow()
  : current_page_(ProfilePage::List)
  , selected_profile_handle_opt_{}
  , editing_profile_ptr_{nullptr}
  , last_position_sizing_values_{
     {backtest::PositionSizing::Mode::RiskDistance, 0.01},
     {backtest::PositionSizing::Mode::FixedQuantity, 1.0},
     {backtest::PositionSizing::Mode::FixedNotional, 1000.0},
     {backtest::PositionSizing::Mode::EquityPercent, 1.0}}
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Profiles", nullptr);
    switch(self.current_page_) {
    case ProfilePage::AddNewProfile:
      self.render_add_new_profile(context);
      break;
    case ProfilePage::EditProfile:
      self.render_edit_profile(context);
      break;
    case ProfilePage::List:
    default:
      self.render_profiles_list(context);
      break;
    }

    ImGui::End();
  }

private:
  enum class ProfilePage { List, AddNewProfile, EditProfile } current_page_;

  std::optional<backtest::ProfileStoreHandle> selected_profile_handle_opt_;
  std::shared_ptr<backtest::Profile> editing_profile_ptr_;

  std::unordered_map<backtest::PositionSizing::Mode, double>
   last_position_sizing_values_;

  void render_profiles_list(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& profile_handles = app_state.get_profile_handles();
    const auto backtest_ptr = app_state.selected_backtest_if_present();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    for(auto i = 0; i < profile_handles.size(); ++i) {
      const auto profile_handle = profile_handles[i];
      const auto& profile = app_state.get_profile(profile_handle);

      ImGui::PushID(i);

      auto is_selected =
       backtest_ptr && backtest_ptr->profile_handle() == profile_handle;
      const auto row_start = ImGui::GetCursorScreenPos();
      const auto row_width = ImGui::GetContentRegionAvail().x;
      const auto row_height = ImGui::GetFrameHeight();

      ImGui::SetNextItemAllowOverlap();
      ImGui::Selectable("##profile_row",
                        is_selected,
                        ImGuiSelectableFlags_AllowOverlap,
                        ImVec2(row_width, row_height));

      ImGui::AlignTextToFramePadding();
      ImGui::SetCursorScreenPos(row_start);
      ImGui::TextUnformatted(profile.name().c_str());

      const auto spacing = ImGui::GetStyle().ItemSpacing.x;
      const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
      const auto edit_width =
       ImGui::CalcTextSize("Edit").x + (2.0f * frame_padding_x);
      const auto delete_width =
       ImGui::CalcTextSize("Delete").x + (2.0f * frame_padding_x);
      const auto more_width =
       ImGui::CalcTextSize("More...").x + (2.0f * frame_padding_x);
      const auto buttons_width =
       edit_width + spacing + delete_width + spacing + more_width;
      const auto buttons_start_x = row_start.x + row_width - buttons_width;

      ImGui::SetCursorScreenPos(ImVec2(buttons_start_x, row_start.y));
      if(ImGui::Button("Edit")) {
        self.current_page_ = ProfilePage::EditProfile;
        self.selected_profile_handle_opt_ = profile_handle;
        self.editing_profile_ptr_ =
         std::make_shared<backtest::Profile>(profile);
      }

      ImGui::SameLine();

      if(ImGui::Button("Delete")) {
        context.push_action([profile_handle](ApplicationState& app_state) {
          app_state.remove_profile(profile_handle);
        });
      }

      ImGui::SameLine();

      if(ImGui::Button("More...")) {
        ImGui::OpenPopup("profile_menu_more");
      }

      if(ImGui::BeginPopup("profile_menu_more")) {
        if(ImGui::MenuItem("Duplicate")) {
          context.push_action([profile_handle](ApplicationState& app_state) {
            const auto& profile = app_state.get_profile(profile_handle);
            backtest::Profile duplicate_profile = profile;
            duplicate_profile.name(profile.name() + " Copy");
            app_state.add_profile(std::move(duplicate_profile));
          });
        }

        const auto move_up_disabled = i == 0;
        if(ImGui::MenuItem("Move Up", nullptr, false, !move_up_disabled)) {
          context.push_action(
           [from_index = i, to_index = i - 1](ApplicationState& app_state) {
             app_state.reorder_list_profile(from_index, to_index);
           });
        }

        const auto move_down_disabled = i == profile_handles.size() - 1;
        if(ImGui::MenuItem("Move Down", nullptr, false, !move_down_disabled)) {
          context.push_action(
           [from_index = i, to_index = i + 1](ApplicationState& app_state) {
             app_state.reorder_list_profile(from_index, to_index);
           });
        }

        ImGui::EndPopup();
      }

      ImGui::PopID();
    }

    ImGui::EndChild();
    if(ImGui::Button("Add New Profile")) {
      self.current_page_ = ProfilePage::AddNewProfile;

      self.selected_profile_handle_opt_ = std::nullopt;
      self.editing_profile_ptr_ = std::make_shared<backtest::Profile>();
      self.editing_profile_ptr_->position_sizing(backtest::PositionSizing{
       backtest::PositionSizing::Mode::RiskDistance, 0.01});
    }

    ImGui::EndGroup();
  }

  void render_add_new_profile(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Profile");
    ImGui::Separator();

    self.edit_profile_form(context);

    ImGui::EndChild();
    if(ImGui::Button("Create Profile")) {
      self.submit_profile_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_profile(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Profile");
    ImGui::Separator();

    self.edit_profile_form(context);

    ImGui::EndChild();

    const auto selected_profile_handle =
     self.selected_profile_handle_opt_.value();
    const auto& selected_profile =
     context.app_state().get_profile(selected_profile_handle);
    const auto same_profile = selected_profile == *(self.editing_profile_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_profile_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::BeginDisabled(same_profile);
    ImGui::SameLine();
    if(ImGui::Button("Apply")) {
      self.submit_profile_changes(context);
    }
    ImGui::EndDisabled();

    ImGui::EndGroup();
  }

  void edit_profile_form(this auto& self, WindowContext& context)
  {
    {
      auto profile_name = self.editing_profile_ptr_->name();
      ImGui::InputText("Name", &profile_name);
      self.editing_profile_ptr_->name(profile_name);
    }
    {
      auto position_sizing = self.editing_profile_ptr_->position_sizing();
      const auto mode_label = [](backtest::PositionSizing::Mode mode) {
        switch(mode) {
        case backtest::PositionSizing::Mode::RiskDistance:
          return "Risk Distance";
        case backtest::PositionSizing::Mode::FixedQuantity:
          return "Fixed Quantity";
        case backtest::PositionSizing::Mode::FixedNotional:
          return "Fixed Notional";
        case backtest::PositionSizing::Mode::EquityPercent:
          return "Equity Percent";
        }

        return "Risk Distance";
      };

      constexpr auto position_sizing_modes =
       std::array{backtest::PositionSizing::Mode::RiskDistance,
                  backtest::PositionSizing::Mode::FixedQuantity,
                  backtest::PositionSizing::Mode::FixedNotional,
                  backtest::PositionSizing::Mode::EquityPercent};

      auto mode = position_sizing.mode();
      if(ImGui::BeginCombo("Position Sizing", mode_label(mode))) {
        for(const auto& position_sizing_mode : position_sizing_modes) {
          const auto selected = mode == position_sizing_mode;
          if(ImGui::Selectable(mode_label(position_sizing_mode), selected)) {
            mode = position_sizing_mode;
            const auto value = self.last_position_sizing_values_[mode];
            position_sizing = backtest::PositionSizing{mode, value};
          }
          if(selected) {
            ImGui::SetItemDefaultFocus();
          }
        }

        ImGui::EndCombo();
      }

      auto value = position_sizing.value();
      switch(mode) {
      case backtest::PositionSizing::Mode::RiskDistance: {
        auto percentage = value * 100.0;
        ImGui::InputDouble("Capital Risk (%)", &percentage, 1.0, 10.0, "%.2f");
        value = percentage / 100.0;
        break;
      }
      case backtest::PositionSizing::Mode::FixedQuantity:
        ImGui::InputDouble("Quantity", &value, 1.0, 10.0, "%.8f");
        break;
      case backtest::PositionSizing::Mode::FixedNotional:
        ImGui::InputDouble("Notional", &value, 100.0, 1000.0, "%.2f");
        break;
      case backtest::PositionSizing::Mode::EquityPercent: {
        auto percentage = value * 100.0;
        ImGui::InputDouble("Equity (%)", &percentage, 1.0, 10.0, "%.2f");
        value = percentage / 100.0;
        break;
      }
      }

      position_sizing.mode(mode);
      position_sizing.value(value);
      self.last_position_sizing_values_[mode] = value;
      self.editing_profile_ptr_->position_sizing(position_sizing);
    }
    {
      auto drawdown_adjustment =
       self.editing_profile_ptr_->drawdown_adjustment();
      auto enabled = drawdown_adjustment.enabled();
      ImGui::Checkbox("Drawdown Adjustment", &enabled);
      drawdown_adjustment.enabled(enabled);

      auto drawdown_step = drawdown_adjustment.drawdown_step() * 100.0;
      ImGui::InputDouble(
       "Drawdown Step (%)", &drawdown_step, 1.0, 10.0, "%.2f");
      drawdown_adjustment.drawdown_step(drawdown_step / 100.0);

      auto size_reduction = drawdown_adjustment.size_reduction() * 100.0;
      ImGui::InputDouble(
       "Size Reduction (%)", &size_reduction, 1.0, 10.0, "%.2f");
      drawdown_adjustment.size_reduction(size_reduction / 100.0);

      self.editing_profile_ptr_->drawdown_adjustment(drawdown_adjustment);
    }
  }

  void submit_profile_changes(this auto& self, WindowContext& context)
  {
    context.push_action([profile_handle_opt = self.selected_profile_handle_opt_,
                         edit_profile_ptr = self.editing_profile_ptr_](
                         ApplicationState& app_state) {
      if(edit_profile_ptr->name().empty()) {
        edit_profile_ptr->name("Unnamed");
      }

      if(!profile_handle_opt.has_value()) {
        app_state.add_profile(*edit_profile_ptr);
        return;
      }

      app_state.update_profile(profile_handle_opt.value(), *edit_profile_ptr);
    });
  }

  void reset(this ProfilesWindow& self) noexcept
  {
    self.current_page_ = ProfilePage::List;
    self.selected_profile_handle_opt_ = std::nullopt;
    self.editing_profile_ptr_ = nullptr;
  }
};

} // namespace pludux::apps
