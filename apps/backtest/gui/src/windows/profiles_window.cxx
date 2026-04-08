module;

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>

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
  , selected_profile_ptr_{nullptr}
  , editing_profile_ptr_{nullptr}
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

  std::shared_ptr<backtest::Profile> selected_profile_ptr_;
  std::shared_ptr<backtest::Profile> editing_profile_ptr_;

  void render_profiles_list(this auto& self, WindowContext& context)
  {
    const auto& backtest = context.app_state().selected_backtest();
    const auto& profiles = context.profiles();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    if(!profiles.empty()) {
      for(auto i = 0; i < profiles.size(); ++i) {
        const auto& profile = profiles[i];

        ImGui::PushID(i);

        ImGui::SetNextItemAllowOverlap();
        auto is_selected = backtest && backtest->profile_ptr() == profile;
        ImGui::Selectable(profile->name().c_str(), &is_selected);

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        if(ImGui::Button("Edit")) {
          self.current_page_ = ProfilePage::EditProfile;
          self.selected_profile_ptr_ = profile;
          self.editing_profile_ptr_ =
           std::make_shared<backtest::Profile>(*profile);
        }

        ImGui::SameLine();

        if(ImGui::Button("Delete")) {
          context.push_action([i](ApplicationState& app_state) {
            app_state.remove_profile_at_index(i);
          });
        }

        ImGui::PopID();
      }
    }
    ImGui::EndChild();
    if(ImGui::Button("Add New Profile")) {
      self.current_page_ = ProfilePage::AddNewProfile;

      self.selected_profile_ptr_ = nullptr;
      self.editing_profile_ptr_ = std::make_shared<backtest::Profile>();
      self.editing_profile_ptr_->capital_risk(0.01);
    }

    ImGui::EndGroup();
  }

  void render_add_new_profile(this auto& self, WindowContext& context)
  {
    const auto& profiles = context.profiles();

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

    const auto same_profile =
     *(self.selected_profile_ptr_) == *(self.editing_profile_ptr_);

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
      auto percentage = self.editing_profile_ptr_->capital_risk() * 100.0;
      ImGui::InputDouble("Capital Risk (%)", &percentage, 1.0, 10.0, "%.2f");
      self.editing_profile_ptr_->capital_risk(percentage / 100.0);
    }
    {
      ImGui::SeparatorText("R Distance");
      self.render_r_distance(context);
      ImGui::Text("");
    }
  }

  void render_r_distance(this auto& self, WindowContext& context)
  {
    auto r_distance_mode =
     static_cast<int>(self.editing_profile_ptr_->r_distance_mode());
    {
      if(ImGui::RadioButton(
          "ATR",
          &r_distance_mode,
          static_cast<int>(backtest::Profile::RDistance::Atr))) {
        self.editing_profile_ptr_->r_distance_mode(
         backtest::Profile::RDistance::Atr);
      }

      ImGui::Indent();
      ImGui::Text("Period:");
      ImGui::SameLine();
      auto r_mode_atr_period =
       static_cast<int>(self.editing_profile_ptr_->r_mode_atr().first);
      if(ImGui::InputInt("##r_mode_atr_period", &r_mode_atr_period)) {
        if(r_mode_atr_period < 1) {
          r_mode_atr_period = 1;
        }
      }
      ImGui::Text("Multiplier:");
      ImGui::SameLine();
      auto r_mode_atr_multiplier =
       self.editing_profile_ptr_->r_mode_atr().second;
      if(ImGui::InputDouble("##r_mode_atr_multiplier",
                            &r_mode_atr_multiplier,
                            0.1,
                            1.0,
                            "%.2f")) {
        if(r_mode_atr_multiplier < 0.1) {
          r_mode_atr_multiplier = 0.1;
        }
      }

      self.editing_profile_ptr_->r_mode_atr(
       {r_mode_atr_period, r_mode_atr_multiplier});
      ImGui::Unindent();
    }
    {
      if(ImGui::RadioButton(
          "Percentage",
          &r_distance_mode,
          static_cast<int>(backtest::Profile::RDistance::Percentage))) {
        self.editing_profile_ptr_->r_distance_mode(
         backtest::Profile::RDistance::Percentage);
      }

      ImGui::Indent();
      ImGui::Text("Percent:");
      ImGui::SameLine();
      auto r_mode_percentage = self.editing_profile_ptr_->r_mode_percentage();
      if(ImGui::InputDouble(
          "##percentage_risk", &r_mode_percentage, 0.1, 1.0, "%.2f")) {
        if(r_mode_percentage < 0.1) {
          r_mode_percentage = 0.1;
        }

        self.editing_profile_ptr_->r_mode_percentage(r_mode_percentage);
      }
      ImGui::Unindent();
    }
    {
      if(ImGui::RadioButton(
          "Price",
          &r_distance_mode,
          static_cast<int>(backtest::Profile::RDistance::Price))) {
        self.editing_profile_ptr_->r_distance_mode(
         backtest::Profile::RDistance::Price);
      }

      ImGui::Indent();
      ImGui::Text("Amount:");
      ImGui::SameLine();
      auto r_mode_price = self.editing_profile_ptr_->r_mode_price();
      if(ImGui::InputDouble(
          "##r_mode_price", &r_mode_price, 0.1, 1.0, "%.2f")) {
        if(r_mode_price < 0.1) {
          r_mode_price = 0.1;
        }

        self.editing_profile_ptr_->r_mode_price(r_mode_price);
      }
      ImGui::Unindent();
    }
  }

  void submit_profile_changes(this auto& self, WindowContext& context)
  {
    context.push_action([profile_ptr = self.selected_profile_ptr_,
                         edit_profile_ptr = self.editing_profile_ptr_](
                         ApplicationState& app_state) {
      if(edit_profile_ptr->name().empty()) {
        edit_profile_ptr->name("Unnamed");
      }

      if(profile_ptr == nullptr) {
        app_state.add_profile(
         std::make_shared<backtest::Profile>(*edit_profile_ptr));
        return;
      }

      const auto reset_backtests = !profile_ptr->equal_rules(*edit_profile_ptr);
      if(reset_backtests) {
        for(auto& backtest : app_state.backtests()) {
          if(backtest->profile_ptr() == profile_ptr) {
            backtest->reset();
          }
        }
      }

      *profile_ptr = *edit_profile_ptr;
    });
  }

  void reset(this ProfilesWindow& self) noexcept
  {
    self.current_page_ = ProfilePage::List;
    self.selected_profile_ptr_ = nullptr;
    self.editing_profile_ptr_ = nullptr;
  }
};

} // namespace pludux::apps
