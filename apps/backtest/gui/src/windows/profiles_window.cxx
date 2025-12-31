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
import :app_state;

export namespace pludux::apps {

class ProfilesWindow {
public:
  ProfilesWindow()
  : current_page_(ProfilePage::List)
  , selected_profile_ptr_{nullptr}
  , editing_profile_ptr_{nullptr}
  {
  }

  void render(this auto& self, AppState& app_state)
  {
    auto& state = app_state.state();

    ImGui::Begin("Profiles", nullptr);
    switch(self.current_page_) {
    case ProfilePage::AddNewProfile:
      self.render_add_new_profile(app_state);
      break;
    case ProfilePage::EditProfile:
      self.render_edit_profile(app_state);
      break;
    case ProfilePage::List:
    default:
      self.render_profiles_list(app_state);
      break;
    }

    ImGui::End();
  }

private:
  enum class ProfilePage { List, AddNewProfile, EditProfile } current_page_;

  std::shared_ptr<backtest::Profile> selected_profile_ptr_;
  std::shared_ptr<backtest::Profile> editing_profile_ptr_;

  void render_profiles_list(this auto& self, AppState& app_state)
  {
    auto& state = app_state.state();
    auto& profiles = state.profiles;

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
        ImGui::Text("%s", profile->name().c_str());

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
          app_state.push_action([i](AppStateData& state) {
            auto& profiles = state.profiles;

            const auto it = std::next(profiles.begin(), i);
            auto& profile_ptr = *it;

            auto& backtests = state.backtests;
            for(auto j = 0; j < backtests.size(); ++j) {
              auto& backtest = backtests[j];
              if(backtest.profile_ptr() == profile_ptr) {
                backtests.erase(std::next(backtests.begin(), j));

                if(state.selected_backtest_index > i ||
                   state.selected_backtest_index >= backtests.size()) {
                  --state.selected_backtest_index;
                }

                // Adjust the index since we removed an element
                --j;
              }
            }

            // Remove the profile from the vector
            state.profiles.erase(it);
          });
        }

        ImGui::PopID();
      }
    }
    ImGui::EndChild();
    if(ImGui::Button("Add New Profile")) {
      self.current_page_ = ProfilePage::AddNewProfile;

      self.selected_profile_ptr_ = nullptr;
      self.editing_profile_ptr_ = std::make_shared<backtest::Profile>("");
      self.editing_profile_ptr_->initial_capital(100'000'000.0);
      self.editing_profile_ptr_->capital_risk(0.01);
    }

    ImGui::EndGroup();
  }

  void render_add_new_profile(this auto& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& profiles = state.profiles;

    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Profile");
    ImGui::Separator();

    self.edit_profile_form();

    ImGui::EndChild();
    if(ImGui::Button("Create Profile")) {
      self.submit_profile_changes(app_state);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_profile(this auto& self, AppState& app_state)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Profile");
    ImGui::Separator();

    self.edit_profile_form();

    ImGui::EndChild();
    if(ImGui::Button("Edit")) {
      self.submit_profile_changes(app_state);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Apply")) {
      self.submit_profile_changes(app_state);
    }

    ImGui::EndGroup();
  }

  void edit_profile_form(this auto& self)
  {
    {
      auto profile_name = self.editing_profile_ptr_->name();
      ImGui::InputText("Name", &profile_name);
      self.editing_profile_ptr_->name(profile_name);
    }
    {
      auto initial_capital = self.editing_profile_ptr_->initial_capital();
      ImGui::InputDouble(
       "Initial Capital", &initial_capital, 100.0, 1000.0, "%.0f");
      self.editing_profile_ptr_->initial_capital(initial_capital);
    }
    {
      auto percentage = self.editing_profile_ptr_->capital_risk() * 100.0;
      ImGui::InputDouble("Capital Risk (%)", &percentage, 1.0, 10.0, "%.2f");
      self.editing_profile_ptr_->capital_risk(percentage / 100.0);
    }
  }

  void submit_profile_changes(this auto& self, AppState& app_state)
  {
    if(self.editing_profile_ptr_->name().empty()) {
      self.editing_profile_ptr_->name("Unnamed");
    }

    app_state.push_action(
     [profile_ptr = self.selected_profile_ptr_,
      editing_profile = *self.editing_profile_ptr_](AppStateData& state) {
       if(profile_ptr == nullptr) {
         state.profiles.push_back(
          std::make_shared<backtest::Profile>(editing_profile));
         return;
       }

       *profile_ptr = editing_profile;

       for(auto& backtest : state.backtests) {
         if(backtest.profile_ptr() == profile_ptr) {
           backtest.reset();
         }
       }
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
